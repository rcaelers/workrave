//! Executes a [`Pipeline`]: selects the jobs of a target, orders them by
//! `needs`, expands matrices and `foreach`, and runs each step in its
//! environment — or, for `ship pipeline show`, prints what it would run.

use std::collections::HashMap;
use std::path::PathBuf;

use anyhow::{anyhow, bail, Context, Result};
use indexmap::IndexMap;

use super::actions::{self, ActionEnv};
use super::context::{insert_dotted, parse_outputs, HostVars, Renderer, Secrets, ShipVars, Vars};
use super::schema::{DryRunMode, Environment, EnvironmentKind, Job, Pipeline, Step};
use crate::config::Config;
use crate::services::signing::SigningService;
use crate::system::container::{ContainerRun, ContainerSettings, Mounts};
use crate::system::process::Cmd;

pub struct RunOptions {
    /// Target name; `None` picks by host OS.
    pub target: Option<String>,
    /// Restrict to these jobs (after target selection).
    pub jobs: Vec<String>,
    pub skip_jobs: Vec<String>,
    /// Builds happen, uploads and signing are only printed.
    pub dry_run: bool,
    /// Print the resolved plan instead of running it.
    pub show: bool,
    /// The pipeline's options as given (`{{ options.* }}`), including
    /// `--set` values; missing ones get their declared defaults.
    pub options: serde_json::Map<String, serde_json::Value>,
    /// `--set config.<key>=<value>` overrides of the configuration.
    pub config_overrides: Vec<(String, serde_json::Value)>,
}

fn host_os() -> &'static str {
    if cfg!(target_os = "macos") {
        "macos"
    } else if cfg!(target_os = "windows") {
        "windows"
    } else {
        "linux"
    }
}

/// The jobs to run, in order.
fn select_jobs(pipeline: &Pipeline, opts: &RunOptions) -> Result<Vec<String>> {
    let target_names: Vec<String> = match &opts.target {
        Some(t) => vec![t.clone()],
        None => match host_os() {
            "windows" => vec!["windows".to_string()],
            "macos" => vec!["linux".to_string(), "macos".to_string()],
            _ => vec!["linux".to_string()],
        },
    };
    let mut selected: Vec<String> = Vec::new();
    for name in &target_names {
        let jobs = pipeline.targets.get(name).ok_or_else(|| {
            anyhow!(
                "unknown target '{name}' (available: {})",
                pipeline
                    .targets
                    .keys()
                    .cloned()
                    .collect::<Vec<_>>()
                    .join(", ")
            )
        })?;
        for job in jobs {
            if !selected.contains(job) {
                selected.push(job.clone());
            }
        }
    }
    for job in opts.jobs.iter().chain(&opts.skip_jobs) {
        if !pipeline.jobs.contains_key(job) {
            bail!("unknown job '{job}'");
        }
    }
    if !opts.jobs.is_empty() {
        selected.retain(|j| opts.jobs.contains(j) || pipeline.jobs[j].always);
    }
    selected.retain(|j| !opts.skip_jobs.contains(j));
    pipeline.ordered_jobs(&selected)
}

pub async fn run(config: Config, pipeline: Pipeline, opts: RunOptions) -> Result<()> {
    let ordered = select_jobs(&pipeline, &opts)?;
    if ordered.is_empty() {
        bail!("no jobs selected");
    }

    let mut config_value = super::context::config_value(config.value())?;
    for (key, value) in &opts.config_overrides {
        insert_dotted(&mut config_value, key, value.clone());
    }

    // The pipeline's options: declared defaults, then what was given.
    let mut options = serde_json::Value::Object(Default::default());
    for (name, spec) in &pipeline.options {
        let default = match &spec.default {
            Some(v) => serde_json::to_value(v)?,
            None if spec.flag => serde_json::Value::Bool(false),
            None => serde_json::Value::String(String::new()),
        };
        insert_dotted(&mut options, name, default);
    }
    for (key, value) in &opts.options {
        insert_dotted(&mut options, key, value.clone());
    }

    let mut vars = Vars {
        config: config_value,
        options,
        dry_run: opts.dry_run,
        host: HostVars {
            os: host_os().to_string(),
        },
        ship: ShipVars {
            exe: std::env::current_exe().unwrap_or_else(|_| PathBuf::from("ship")),
        },
        env: std::env::vars().collect(),
        vars: HashMap::new(),
        vars_errors: HashMap::new(),
        matrix: HashMap::new(),
        item: None,
        outputs: Default::default(),
    };

    // The engine's own settings, rendered once over config and options.
    let settings_renderer = Renderer::new(Secrets::Placeholder);
    let signing_service_url = match &pipeline.settings.signing_service_url {
        Some(url) => Some(settings_renderer.render(url, &vars)?).filter(|u| !u.is_empty()),
        None => None,
    };

    let secrets = if opts.show {
        Secrets::Placeholder
    } else if opts.dry_run {
        Secrets::DryRun
    } else {
        match &signing_service_url {
            Some(url) => Secrets::Fetch(SigningService::new(url)?),
            None => Secrets::Unavailable,
        }
    };
    let renderer = Renderer::new(secrets);

    if opts.show {
        println!("Jobs: {}", ordered.join(", "));
    } else {
        tracing::info!("Jobs: {}", ordered.join(", "));
    }

    let runner = Runner {
        pipeline: &pipeline,
        renderer,
        opts: &opts,
        signing_service_url,
    };

    for name in &ordered {
        let job = &pipeline.jobs[name];
        runner.refresh_vars(&mut vars);
        if let Some(condition) = &job.condition {
            if !runner.renderer.condition(condition, &vars)? {
                runner.say(&format!("== {name}: skipped ({condition})"));
                continue;
            }
        }
        for matrix in expand_matrix(job, &runner.renderer, &vars)? {
            let label = if matrix.is_empty() {
                name.clone()
            } else {
                format!(
                    "{name} [{}]",
                    matrix
                        .iter()
                        .map(|(k, v)| format!("{k}={}", value_text(v)))
                        .collect::<Vec<_>>()
                        .join(", ")
                )
            };
            let job_vars = vars.with_matrix(matrix);
            let outputs = runner
                .run_job(name, job, &label, job_vars)
                .await
                .with_context(|| format!("in job {label}"))?;
            for (key, value) in outputs {
                vars.set_output(&key, value)?;
            }
        }
    }
    runner.say("Done");
    Ok(())
}

fn value_text(value: &serde_json::Value) -> String {
    match value {
        serde_json::Value::String(s) => s.clone(),
        other => other.to_string(),
    }
}

/// All combinations of a job's matrix, in declaration order (empty when the
/// job has no matrix: a single run).
fn expand_matrix(
    job: &Job,
    renderer: &Renderer,
    vars: &Vars,
) -> Result<Vec<HashMap<String, serde_json::Value>>> {
    let Some(strategy) = &job.strategy else {
        return Ok(vec![HashMap::new()]);
    };
    let mut combos: Vec<HashMap<String, serde_json::Value>> = vec![HashMap::new()];
    for (key, values) in &strategy.matrix {
        let mut next = Vec::new();
        for combo in &combos {
            for value in values {
                let rendered = renderer.render_yaml(value, vars)?;
                let json: serde_json::Value = serde_json::to_value(&rendered)?;
                let mut c = combo.clone();
                c.insert(key.clone(), json);
                next.push(c);
            }
        }
        combos = next;
    }
    Ok(combos)
}

/// Values set by steps, in order.
type Outputs = Vec<(String, serde_json::Value)>;

struct Runner<'a> {
    pipeline: &'a Pipeline,
    renderer: Renderer,
    opts: &'a RunOptions,
    signing_service_url: Option<String>,
}

/// A job's environment with its templates rendered.
struct ResolvedEnvironment {
    kind: EnvironmentKind,
    name: String,
    image: String,
    container: ContainerSettings,
    platform: Option<String>,
    cwd: Option<PathBuf>,
    mounts: Vec<(PathBuf, String)>,
    env: Vec<(String, String)>,
    options: Vec<String>,
    bash: PathBuf,
    msystem: String,
}

impl Runner<'_> {
    /// Renders the pipeline's `vars:` in order; each can refer to the
    /// previous ones. A var that cannot be rendered yet (no version before
    /// the workspace job sets it) is recorded with its reason and errors when used.
    fn refresh_vars(&self, vars: &mut Vars) {
        vars.vars.clear();
        vars.vars_errors.clear();
        for (name, template) in &self.pipeline.vars {
            match self.renderer.render(template, vars) {
                Ok(value) => {
                    vars.vars
                        .insert(name.clone(), super::context::parse_output_value(&value));
                }
                Err(e) => {
                    vars.vars_errors.insert(name.clone(), format!("{e:#}"));
                }
            }
        }
    }

    fn say(&self, message: &str) {
        if self.opts.show {
            println!("{message}");
        } else {
            tracing::info!("{message}");
        }
    }

    fn render_map(
        &self,
        map: &IndexMap<String, String>,
        vars: &Vars,
    ) -> Result<Vec<(String, String)>> {
        map.iter()
            .map(|(k, v)| {
                Ok((
                    self.renderer.render(k, vars)?,
                    self.renderer.render(v, vars)?,
                ))
            })
            .collect()
    }

    /// Renders mounts, dropping entries whose local path renders empty.
    fn render_mounts(
        &self,
        map: &IndexMap<String, String>,
        vars: &Vars,
    ) -> Result<Vec<(PathBuf, String)>> {
        let mut mounts = Vec::new();
        for (local, guest) in map {
            let local = self.renderer.render(local, vars)?;
            if local.trim().is_empty() {
                continue;
            }
            mounts.push((
                PathBuf::from(local.trim()),
                self.renderer.render(guest, vars)?,
            ));
        }
        Ok(mounts)
    }

    fn resolve_environment(&self, job: &Job, vars: &Vars) -> Result<ResolvedEnvironment> {
        let host = Environment {
            kind: EnvironmentKind::Host,
            image: None,
            engine: None,
            sync: None,
            remote_dir: None,
            platform: None,
            cwd: None,
            mounts: IndexMap::new(),
            env: IndexMap::new(),
            options: Vec::new(),
            bash: None,
            msystem: None,
        };
        let (name, environment) = match &job.runs_in {
            Some(name) => (name.clone(), &self.pipeline.environments[name]),
            None => ("host".to_string(), &host),
        };
        let mut env = self.render_map(&environment.env, vars)?;
        env.extend(self.render_map(&job.env, vars)?);
        let render_opt = |value: &Option<String>| -> Result<Option<String>> {
            match value {
                Some(v) => Ok(Some(self.renderer.render(v, vars)?)),
                None => Ok(None),
            }
        };
        let defaults = ContainerSettings::default();
        let container = ContainerSettings {
            engine: render_opt(&environment.engine)?
                .unwrap_or_default()
                .parse()
                .with_context(|| format!("in environment '{name}'"))?,
            sync: render_opt(&environment.sync)?
                .unwrap_or_default()
                .parse()
                .with_context(|| format!("in environment '{name}'"))?,
            remote_dir: render_opt(&environment.remote_dir)?
                .filter(|d| !d.is_empty())
                .unwrap_or(defaults.remote_dir),
        };
        // Working directory: step > job > environment > current directory.
        let cwd = match render_opt(&job.cwd)?.or(render_opt(&environment.cwd)?) {
            Some(dir) if !dir.is_empty() => Some(PathBuf::from(dir)),
            _ => None,
        };
        Ok(ResolvedEnvironment {
            kind: environment.kind,
            name,
            image: render_opt(&environment.image)?.unwrap_or_default(),
            container,
            platform: render_opt(&environment.platform)?,
            cwd,
            mounts: self.render_mounts(&environment.mounts, vars)?,
            env,
            options: environment
                .options
                .iter()
                .map(|o| self.renderer.render(o, vars))
                .collect::<Result<_>>()?,
            bash: PathBuf::from(match &environment.bash {
                Some(b) => self.renderer.render(b, vars)?,
                None => "bash".to_string(),
            }),
            msystem: match &environment.msystem {
                Some(m) => self.renderer.render(m, vars)?,
                None => "CLANG64".to_string(),
            },
        })
    }

    async fn run_job(&self, name: &str, job: &Job, label: &str, mut vars: Vars) -> Result<Outputs> {
        let environment = self.resolve_environment(job, &vars)?;
        self.say(&format!("== {label} ({})", environment.name));

        // Container mounts are prepared once per job: the environment's plus
        // every step's, so a remote podman gets everything mirrored up front.
        let mounts = if environment.kind == EnvironmentKind::Container {
            let mut dirs: Vec<PathBuf> =
                environment.mounts.iter().map(|(l, _)| l.clone()).collect();
            for step in &job.steps {
                for (local, _) in self.render_mounts(&step.mounts, &vars)? {
                    if !dirs.contains(&local) {
                        dirs.push(local);
                    }
                }
            }
            // A directory inside another mounted directory comes along with it.
            let all = dirs.clone();
            dirs.retain(|d| !all.iter().any(|other| other != d && d.starts_with(other)));
            if self.opts.show {
                Some(Mounts::direct(dirs))
            } else {
                for dir in &dirs {
                    std::fs::create_dir_all(dir)
                        .with_context(|| format!("creating {}", dir.display()))?;
                }
                let engine = environment.container.engine;
                if let Some(platform) = &environment.platform {
                    crate::system::container::check_platforms(
                        engine,
                        &environment.image,
                        &[platform],
                    )?;
                }
                let platforms: Vec<String> = job
                    .steps
                    .iter()
                    .filter_map(|s| s.platform.as_ref())
                    .map(|p| self.renderer.render(p, &vars))
                    .collect::<Result<_>>()?;
                let platform_refs: Vec<&str> = platforms.iter().map(String::as_str).collect();
                if !platform_refs.is_empty() {
                    crate::system::container::check_platforms(
                        engine,
                        &environment.image,
                        &platform_refs,
                    )?;
                }
                Some(Mounts::prepare(
                    engine,
                    &environment.container,
                    &environment.image,
                    environment.platform.as_deref(),
                    dirs,
                )?)
            }
        } else {
            None
        };

        let result = self
            .run_steps(name, job, &environment, mounts.as_ref(), &mut vars)
            .await;
        // Bring results back, also after a failure (logs, partial output).
        let finish = match &mounts {
            Some(m) if !self.opts.show => m.finish(),
            _ => Ok(()),
        };
        let outputs = result?;
        finish?;
        Ok(outputs)
    }

    async fn run_steps(
        &self,
        job_name: &str,
        job: &Job,
        environment: &ResolvedEnvironment,
        mounts: Option<&Mounts>,
        vars: &mut Vars,
    ) -> Result<Outputs> {
        let mut all_outputs: Outputs = Vec::new();
        for (index, step) in job.steps.iter().enumerate() {
            let step_label = step
                .name
                .clone()
                .unwrap_or_else(|| format!("step {}", index + 1));
            if let Some(condition) = &step.condition {
                if !self.renderer.condition(condition, vars)? {
                    self.say(&format!("-- {step_label}: skipped ({condition})"));
                    continue;
                }
            }
            let items: Vec<Option<serde_json::Value>> = match &step.foreach {
                Some(list) => self
                    .renderer
                    .render_list(list, vars)?
                    .into_iter()
                    .map(Some)
                    .collect(),
                None => vec![None],
            };
            if items.is_empty() {
                self.say(&format!(
                    "-- {step_label}: nothing to do ({})",
                    step.foreach.as_deref().unwrap_or_default()
                ));
            }
            for item in items {
                let step_vars = match item {
                    Some(item) => vars.with_item(item),
                    None => vars.clone(),
                };
                let outputs = self
                    .run_step(step, &step_label, environment, mounts, &step_vars)
                    .await
                    .with_context(|| format!("in {step_label} of job {job_name}"))?;
                if !outputs.is_empty() {
                    for (key, value) in &outputs {
                        vars.set_output(key, value.clone())?;
                    }
                    self.refresh_vars(vars);
                    all_outputs.extend(outputs);
                }
            }
        }
        Ok(all_outputs)
    }

    async fn run_step(
        &self,
        step: &Step,
        label: &str,
        environment: &ResolvedEnvironment,
        mounts: Option<&Mounts>,
        vars: &Vars,
    ) -> Result<Outputs> {
        if let Some(action) = &step.uses {
            let with = self.renderer.render_yaml(&step.with, vars)?;
            let env = ActionEnv {
                dry_run: self.opts.dry_run,
                signing_service_url: self.signing_service_url.as_deref(),
                config: &vars.config,
            };
            if self.opts.show {
                println!("   uses {action}: {}", yaml_inline(&with));
                if actions::find(action)?.runs_in_show() {
                    actions::run(action, &with, &env).await?;
                }
                return Ok(Vec::new());
            }
            tracing::info!("-- {label}: {action}");
            return Ok(actions::run(action, &with, &env).await?.outputs);
        }

        let script = self
            .renderer
            .render(step.run.as_deref().unwrap_or_default(), vars)?;
        let script = script.trim().to_string();
        let mut env = environment.env.clone();
        env.extend(self.render_map(&step.env, vars)?);
        let cwd = match &step.cwd {
            Some(cwd) => PathBuf::from(self.renderer.render(cwd, vars)?),
            None => environment
                .cwd
                .clone()
                .unwrap_or_else(|| std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."))),
        };

        // Where a step writes its `outputs` (name=value lines).
        let output_file = (!step.outputs.is_empty()).then(|| {
            std::env::temp_dir().join(format!(
                "ship-output-{}-{}",
                std::process::id(),
                std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .map(|d| d.as_nanos())
                    .unwrap_or(0)
            ))
        });
        if let Some(file) = &output_file {
            let path = match environment.kind {
                EnvironmentKind::Msys2 => super::context::msys_path(&file.to_string_lossy()),
                _ => file.to_string_lossy().into_owned(),
            };
            env.push(("SHIP_OUTPUT".to_string(), path));
        }

        // Containers are named so an interrupted one can be removed.
        static CONTAINERS: std::sync::atomic::AtomicUsize = std::sync::atomic::AtomicUsize::new(0);
        let container_name = format!(
            "ship-{}-{}",
            std::process::id(),
            CONTAINERS.fetch_add(1, std::sync::atomic::Ordering::SeqCst) + 1
        );
        let cmd = match environment.kind {
            EnvironmentKind::Host => Cmd::new("bash")
                .args(["-c", &script])
                .envs(env.clone())
                .cwd(&cwd),
            EnvironmentKind::Msys2 => Cmd::new(&environment.bash)
                .args(["-l", "-c", &script])
                .env("MSYSTEM", &environment.msystem)
                .env("CHERE_INVOKING", "1")
                .envs(env.clone())
                .cwd(&cwd),
            EnvironmentKind::Container => {
                let mounts = mounts.expect("container job has mounts");
                let mut run = ContainerRun::new(&environment.image)
                    .name(&container_name)
                    .flags(environment.options.clone())
                    .flags(
                        step.options
                            .iter()
                            .map(|o| self.renderer.render(o, vars))
                            .collect::<Result<Vec<_>>>()?,
                    )
                    .envs(env.clone());
                for (local, guest) in merge_mounts(
                    &environment.mounts,
                    &self.render_mounts(&step.mounts, vars)?,
                ) {
                    run = run.mount(local, guest);
                }
                let platform = match &step.platform {
                    Some(p) => Some(self.renderer.render(p, vars)?),
                    None => environment.platform.clone(),
                };
                if let Some(platform) = platform {
                    run = run.platform(platform);
                }
                run.command(["sh", "-c", &script])
                    .to_cmd(environment.container.engine, mounts)
            }
        };

        let echo_only = self.opts.dry_run && step.dry_run == DryRunMode::Echo;
        if self.opts.show {
            let prefix = if step.dry_run == DryRunMode::Echo {
                "   $ (echo in dry run) "
            } else {
                "   $ "
            };
            println!("{prefix}{}", cmd.display());
            // Container commands carry their -e flags; show the others' env.
            if environment.kind != EnvironmentKind::Container && !env.is_empty() {
                for (key, value) in &env {
                    if key != "SHIP_OUTPUT" {
                        println!("       {key}={}", crate::system::process::redacted(value));
                    }
                }
            }
            if !step.outputs.is_empty() {
                println!("       sets {}", step.outputs.join(", "));
            }
            // Placeholders, so later templates render.
            return Ok(step
                .outputs
                .iter()
                .map(|k| (k.clone(), serde_json::Value::String(format!("<{k}>"))))
                .collect());
        }
        tracing::info!("-- {label}");
        let result = cmd.run_or_echo(echo_only);
        if result.is_err()
            && crate::system::process::interrupted()
            && environment.kind == EnvironmentKind::Container
        {
            crate::system::container::remove_container(
                environment.container.engine,
                &container_name,
            );
        }
        let outputs = match &output_file {
            Some(file) => {
                let text = std::fs::read_to_string(file).unwrap_or_default();
                let _ = std::fs::remove_file(file);
                result?;
                let outputs = parse_outputs(&text)?;
                for declared in &step.outputs {
                    if !outputs.iter().any(|(k, _)| k == declared) {
                        bail!("{label} declares output '{declared}' but did not write it to $SHIP_OUTPUT");
                    }
                }
                for (key, _) in &outputs {
                    if !step.outputs.contains(key) {
                        bail!("{label} wrote output '{key}' without declaring it in `outputs`");
                    }
                }
                outputs
            }
            None => {
                result?;
                Vec::new()
            }
        };
        Ok(outputs)
    }
}

/// The environment's mounts plus the step's; a step mount replaces an
/// environment mount with the same destination (e.g. mounting only
/// `deploy/<tag>` at `/workspace/deploy`).
fn merge_mounts(
    environment: &[(PathBuf, String)],
    step: &[(PathBuf, String)],
) -> Vec<(PathBuf, String)> {
    let same = |a: &str, b: &str| a.trim_end_matches('/') == b.trim_end_matches('/');
    let mut merged: Vec<(PathBuf, String)> = environment
        .iter()
        .filter(|(_, guest)| !step.iter().any(|(_, g)| same(g, guest)))
        .cloned()
        .collect();
    merged.extend(step.iter().cloned());
    merged
}

/// `with` parameters on one line, for `pipeline show`.
fn yaml_inline(value: &serde_yaml::Value) -> String {
    match value {
        serde_yaml::Value::Mapping(map) => map
            .iter()
            .map(|(k, v)| format!("{}={}", yaml_inline(k), yaml_inline(v)))
            .collect::<Vec<_>>()
            .join(" "),
        serde_yaml::Value::Sequence(items) => {
            format!(
                "[{}]",
                items.iter().map(yaml_inline).collect::<Vec<_>>().join(", ")
            )
        }
        serde_yaml::Value::String(s) => s.clone(),
        other => serde_yaml::to_string(other)
            .unwrap_or_default()
            .trim()
            .to_string(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::cmd::release::context::tests::sample_vars;

    #[test]
    fn matrix_expansion_keeps_order() {
        let job: Job = serde_yaml::from_str(
            "strategy:\n  matrix:\n    platform: [linux/amd64, linux/aarch64]\n    config: [Release]\nsteps: [{run: x}]\n",
        )
        .unwrap();
        let renderer = Renderer::new(Secrets::Placeholder);
        let combos = expand_matrix(&job, &renderer, &sample_vars()).unwrap();
        assert_eq!(combos.len(), 2);
        assert_eq!(combos[0]["platform"], serde_json::json!("linux/amd64"));
        assert_eq!(combos[1]["platform"], serde_json::json!("linux/aarch64"));
        assert_eq!(combos[1]["config"], serde_json::json!("Release"));

        let plain: Job = serde_yaml::from_str("steps: [{run: x}]\n").unwrap();
        assert_eq!(
            expand_matrix(&plain, &renderer, &sample_vars())
                .unwrap()
                .len(),
            1
        );
    }

    #[test]
    fn selects_target_jobs_with_filters() {
        let pipeline = Pipeline::parse(
            "targets:\n  linux: [a, b, c]\njobs:\n  a: {always: true, steps: [{run: x}]}\n  b: {needs: [a], steps: [{run: x}]}\n  c: {steps: [{run: x}]}\n",
        )
        .unwrap();
        let base = RunOptions {
            target: Some("linux".to_string()),
            jobs: vec![],
            skip_jobs: vec![],
            dry_run: false,
            show: true,
            options: Default::default(),
            config_overrides: vec![],
        };
        assert_eq!(select_jobs(&pipeline, &base).unwrap(), vec!["a", "b", "c"]);
        let only = RunOptions {
            jobs: vec!["b".to_string()],
            ..base
        };
        // `a` is always run, unless skipped explicitly.
        assert_eq!(select_jobs(&pipeline, &only).unwrap(), vec!["a", "b"]);
        let only_skip = RunOptions {
            jobs: vec!["b".to_string()],
            skip_jobs: vec!["a".to_string()],
            target: Some("linux".to_string()),
            ..only
        };
        assert_eq!(select_jobs(&pipeline, &only_skip).unwrap(), vec!["b"]);
        let only = RunOptions {
            jobs: vec!["b".to_string()],
            skip_jobs: vec![],
            target: Some("linux".to_string()),
            ..only_skip
        };
        let skip = RunOptions {
            jobs: vec![],
            skip_jobs: vec!["a".to_string()],
            target: Some("linux".to_string()),
            ..only
        };
        assert_eq!(select_jobs(&pipeline, &skip).unwrap(), vec!["b", "c"]);
        let bad = RunOptions {
            target: Some("nope".to_string()),
            ..skip
        };
        assert!(select_jobs(&pipeline, &bad).is_err());
    }

    #[test]
    fn step_mounts_override_environment_mounts() {
        let environment = vec![
            (PathBuf::from("/w/source"), "/workspace/source".to_string()),
            (PathBuf::from("/w/deploy"), "/workspace/deploy".to_string()),
        ];
        let step = vec![(
            PathBuf::from("/w/deploy/v1"),
            "/workspace/deploy/".to_string(),
        )];
        let merged = merge_mounts(&environment, &step);
        assert_eq!(
            merged,
            vec![
                (PathBuf::from("/w/source"), "/workspace/source".to_string()),
                (
                    PathBuf::from("/w/deploy/v1"),
                    "/workspace/deploy/".to_string()
                ),
            ]
        );
    }

    #[test]
    fn yaml_inline_formats_parameters() {
        let v: serde_yaml::Value =
            serde_yaml::from_str("{ tag: v1, assets: [a, b], n: 2 }").unwrap();
        assert_eq!(yaml_inline(&v), "tag=v1 assets=[a, b] n=2");
    }
}
