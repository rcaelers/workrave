//! Executes a [`Pipeline`]: selects the jobs of a target, orders them by
//! `needs`, expands matrices and `foreach`, and runs each step in its
//! environment — or, for `ship pipeline show`, prints what it would run.

use std::collections::HashMap;
use std::path::PathBuf;
use std::time::Instant;

use crate::system::windows_container::{self, Session as WindowsSession};
use anyhow::{anyhow, bail, Context, Result};
use indexmap::IndexMap;

use super::commands::{self, Commands, Files};
use super::context::{insert_dotted, HostVars, Renderer, Secrets, ShipVars, Vars};
use super::expressions;
use super::report::{JobReport, Report, Status, Timing};
use super::schema::{
    ContainerSpec, DryRunMode, Environment, EnvironmentKind, Job, JobContainer, Pipeline,
    RunnerKind, Step,
};
use crate::config::Config;
use crate::services::actions::{self, ActionEnv};
use crate::services::signing::SigningService;
use crate::system::container::{
    session::Session as LinuxSession, ContainerRun, ContainerSettings, Mounts,
};
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
    /// The pipeline's options as given (`${{ inputs.* }}`), including
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
    let started = Instant::now();
    let mut report = Report::default();
    let result = run_pipeline(config, pipeline, &opts, &mut report).await;
    if !opts.show {
        eprint!(
            "{}",
            report.render(
                started.elapsed(),
                Status::from_result(&result),
                opts.dry_run
            )
        );
    }
    result
}

async fn run_pipeline(
    config: Config,
    pipeline: Pipeline,
    opts: &RunOptions,
    report: &mut Report,
) -> Result<()> {
    let ordered = select_jobs(&pipeline, opts)?;
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
        status: Default::default(),
        options,
        dry_run: opts.dry_run,
        host: HostVars {
            os: host_os().to_string(),
        },
        ship: ShipVars {
            exe: std::env::current_exe().unwrap_or_else(|_| PathBuf::from("ship")),
            workflow_dir: pipeline.directory.clone(),
            targets: opts
                .target
                .clone()
                .map(|t| vec![t])
                .unwrap_or_else(|| match host_os() {
                    "windows" => vec!["windows".into()],
                    "macos" => vec!["linux".into(), "macos".into()],
                    _ => vec!["linux".into()],
                }),
        },
        env: std::env::vars().collect(),
        vars: HashMap::new(),
        vars_errors: HashMap::new(),
        matrix: HashMap::new(),
        steps: Default::default(),
        needs: Default::default(),
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
        opts,
        signing_service_url,
    };

    let mut completed = serde_json::Map::new();
    let mut failed_ancestors = std::collections::HashSet::<String>::new();
    let mut first_error = None;
    for name in &ordered {
        if crate::system::process::interrupted() {
            bail!("release interrupted");
        }
        let job = &pipeline.jobs[name];
        vars.needs = job
            .needs
            .iter()
            .filter_map(|name| {
                completed
                    .get(name)
                    .map(|value: &serde_json::Value| (name.clone(), value.clone()))
            })
            .collect();
        let upstream_failure = job.needs.iter().any(|name| failed_ancestors.contains(name));
        vars.status = expressions::Status {
            success: vars.needs.values().all(|need| need["result"] == "success"),
            failure: upstream_failure,
            cancelled: false,
        };
        if upstream_failure {
            failed_ancestors.insert(name.clone());
        }
        runner.refresh_vars(&mut vars);
        // Job conditions run before expanding the matrix, as on GitHub.
        let ready = if opts.skip_jobs.contains(name) {
            Ok(false)
        } else {
            runner
                .renderer
                .condition(job.condition.as_deref().unwrap_or("success()"), &vars)
        };
        let matrix = ready.and_then(|ready| {
            if ready {
                expand_matrix(job, &runner.renderer, &vars)
            } else {
                Ok(Vec::new())
            }
        });
        let matrix = match matrix {
            Ok(matrix) => matrix,
            Err(error) => {
                completed.insert(
                    name.clone(),
                    serde_json::json!({"outputs": {}, "result": "failure"}),
                );
                failed_ancestors.insert(name.clone());
                report.jobs.push(JobReport {
                    timing: Timing {
                        label: name.clone(),
                        status: Status::Failed,
                        elapsed: Some(std::time::Duration::ZERO),
                    },
                    steps: Vec::new(),
                });
                let error = error.context(format!("preparing job {name}"));
                tracing::error!("{error:#}");
                first_error.get_or_insert(error);
                continue;
            }
        };
        if matrix.is_empty() {
            runner.say(&format!("== {name}: skipped"));
            completed.insert(
                name.clone(),
                serde_json::json!({"outputs": {}, "result": "skipped"}),
            );
            report.jobs.push(JobReport {
                timing: Timing::skipped(name.clone()),
                steps: Vec::new(),
            });
            continue;
        }
        let mut job_failed = false;
        let mut named_outputs = serde_json::Map::new();
        for matrix in matrix {
            let label = if matrix.is_empty() {
                name.clone()
            } else {
                let mut values: Vec<_> = matrix
                    .iter()
                    .map(|(k, v)| format!("{k}={}", value_text(v)))
                    .collect();
                values.sort();
                format!("{name} [{}]", values.join(", "))
            };
            if job_failed && job.strategy.as_ref().is_some_and(|s| s.fail_fast) {
                report.jobs.push(JobReport {
                    timing: Timing::skipped(label),
                    steps: Vec::new(),
                });
                continue;
            }
            let mut job_vars = vars.with_matrix(matrix);
            job_vars.status = Default::default();
            let started = Instant::now();
            let mut steps = Vec::new();
            let result = runner
                .run_job(name, job, &label, job_vars, &mut steps)
                .await
                .context(format!("in job {label}"));
            report.jobs.push(JobReport {
                timing: Timing {
                    label,
                    status: if result.as_ref().is_ok_and(|outputs| outputs.error.is_none()) {
                        Status::Ok
                    } else {
                        Status::Failed
                    },
                    elapsed: Some(started.elapsed()),
                },
                steps,
            });
            match result {
                Ok(outputs) => {
                    if let Some(error) = outputs.error {
                        job_failed = true;
                        failed_ancestors.insert(name.clone());
                        first_error.get_or_insert(error.context(format!("in job {name}")));
                    }
                    named_outputs.extend(outputs.named);
                    for (key, value) in outputs.legacy {
                        vars.set_output(&key, value)?;
                    }
                }
                Err(error) => {
                    tracing::error!("{error:#}");
                    job_failed = true;
                    failed_ancestors.insert(name.clone());
                    first_error.get_or_insert(error);
                }
            }
        }
        completed.insert(name.clone(), serde_json::json!({"outputs": named_outputs, "result": if job_failed {"failure"} else {"success"}}));
    }
    if let Some(error) = first_error {
        return Err(error);
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

/// Matrix axes and include/exclude follow GitHub's merge rules. The executor
/// remains serial, which also satisfies every positive max-parallel bound.
fn expand_matrix(
    job: &Job,
    renderer: &Renderer,
    vars: &Vars,
) -> Result<Vec<HashMap<String, serde_json::Value>>> {
    let Some(strategy) = &job.strategy else {
        return Ok(vec![HashMap::new()]);
    };
    let rendered = renderer.render_yaml(&strategy.matrix, vars)?;
    let matrix: IndexMap<String, serde_json::Value> =
        serde_yaml::from_value(rendered).context("strategy.matrix must be an object")?;
    let mut original = vec![HashMap::new()];
    let mut axes = 0;
    for (key, values) in &matrix {
        if matches!(key.as_str(), "include" | "exclude") {
            continue;
        }
        axes += 1;
        let values = values
            .as_array()
            .with_context(|| format!("matrix axis '{key}' must be an array"))?;
        let mut next = Vec::new();
        for combination in original {
            for value in values {
                let mut combination = combination.clone();
                combination.insert(key.clone(), value.clone());
                next.push(combination);
            }
        }
        original = next;
    }
    let entries = |name: &str| -> Result<Vec<serde_json::Map<String, serde_json::Value>>> {
        match matrix.get(name) {
            None => Ok(Vec::new()),
            Some(value) => value
                .as_array()
                .with_context(|| format!("matrix {name} must be an array"))?
                .iter()
                .map(|entry| {
                    entry
                        .as_object()
                        .cloned()
                        .with_context(|| format!("matrix {name} entries must be objects"))
                })
                .collect(),
        }
    };
    for excluded in entries("exclude")? {
        if excluded
            .keys()
            .any(|key| !matrix.contains_key(key) || matches!(key.as_str(), "include" | "exclude"))
        {
            bail!("matrix exclude contains an unknown axis");
        }
        original.retain(|c| {
            !excluded
                .iter()
                .all(|(key, value)| c.get(key) == Some(value))
        });
    }
    if axes == 0 {
        original.clear();
    }
    let mut combinations = original.clone();
    for included in entries("include")? {
        let mut merged = false;
        for (base, combination) in original.iter().zip(&mut combinations) {
            if included
                .iter()
                .all(|(key, value)| base.get(key).is_none_or(|base| base == value))
            {
                combination.extend(included.clone());
                merged = true;
            }
        }
        if !merged {
            combinations.push(included.into_iter().collect());
        }
    }
    Ok(combinations)
}

/// Values set by steps, in order.
type Outputs = Vec<(String, serde_json::Value)>;

#[derive(Default)]
struct JobOutputs {
    legacy: Outputs,
    named: serde_json::Map<String, serde_json::Value>,
    error: Option<anyhow::Error>,
}

#[derive(Default)]
struct StepResult {
    error: Option<anyhow::Error>,
    legacy: Outputs,
    commands: Commands,
}

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
    shell: Option<String>,
    mounts: Vec<(PathBuf, String)>,
    env: Vec<(String, String)>,
    options: Vec<String>,
    bash: PathBuf,
    msystem: String,
    windows: Option<windows_container::Settings>,
}

impl Runner<'_> {
    /// Renders the pipeline's `vars:` in order; each can refer to the
    /// previous ones. A var that cannot be rendered yet (no version before
    /// the workspace job sets it) is recorded with its reason and errors when used.
    fn refresh_vars(&self, vars: &mut Vars) {
        vars.vars.clear();
        vars.vars_errors.clear();
        for (name, template) in &self.pipeline.vars {
            match self.renderer.render_native(template, vars) {
                Ok(value) => {
                    let value = if template.contains("${{") {
                        value
                    } else {
                        super::context::parse_output_value(value.as_str().unwrap_or_default())
                    };
                    vars.vars.insert(name.clone(), value);
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
            ssh: None,
            start: None,
            stop: None,
            shell: None,
            image_context: None,
            copy: IndexMap::new(),
            collect: IndexMap::new(),
            init: None,
        };
        let (name, mut environment) = match &job.runs_in {
            Some(name) => {
                let name = self.renderer.render(name, vars)?;
                let environment = self
                    .pipeline
                    .environments
                    .get(&name)
                    .ok_or_else(|| anyhow!("unknown environment '{name}'"))?;
                (name, environment.clone())
            }
            None => (
                self.renderer
                    .render(job.runs_on.as_deref().unwrap_or("host"), vars)?,
                host,
            ),
        };
        if job.runs_in.is_none() {
            if name != "host" {
                let runner = self
                    .pipeline
                    .runners
                    .get(&name)
                    .ok_or_else(|| anyhow!("unknown runner '{name}'"))?;
                environment.kind = match runner.kind {
                    RunnerKind::Host => EnvironmentKind::Host,
                    RunnerKind::LinuxContainerHost => EnvironmentKind::Container,
                    RunnerKind::WindowsContainerHost => EnvironmentKind::WindowsContainer,
                };
                environment.engine = runner.engine.clone();
                environment.sync = runner.sync.clone();
                environment.remote_dir = runner.remote_dir.clone();
                environment.ssh = runner.ssh.clone();
                environment.start = runner.start.clone();
                environment.stop = runner.stop.clone();
            }
            if let Some(container) = &job.container {
                let spec = match container {
                    JobContainer::Image(image) => ContainerSpec {
                        image: image.clone(),
                        ..Default::default()
                    },
                    JobContainer::Definition(spec) => spec.clone(),
                };
                if environment.kind == EnvironmentKind::Host {
                    environment.kind = if cfg!(windows) {
                        EnvironmentKind::WindowsContainer
                    } else {
                        EnvironmentKind::Container
                    };
                }
                if environment.kind != EnvironmentKind::WindowsContainer
                    && (spec.image_context.is_some()
                        || !spec.copy.is_empty()
                        || !spec.collect.is_empty()
                        || spec.init.is_some())
                {
                    bail!("container image-context/copy/collect/init currently require a Windows container runner");
                }
                if environment.kind == EnvironmentKind::WindowsContainer
                    && (spec.platform.is_some() || !spec.mounts.is_empty())
                {
                    bail!("Windows job containers use volumes or copy/collect; platform and synchronized mounts are only supported on Linux");
                }
                environment.image = Some(spec.image);
                environment.env = spec.env;
                environment.platform = spec.platform;
                environment.mounts = spec.mounts;
                environment.image_context = spec.image_context;
                environment.copy = spec.copy;
                environment.collect = spec.collect;
                environment.init = spec.init;
                if let Some(options) = spec.options {
                    // Tokenize after interpolation, preserving quoted option values.
                    environment.options = shlex::split(&self.renderer.render(&options, vars)?)
                        .context("invalid quoting in container.options")?;
                    if environment.options.iter().any(|o| {
                        matches!(
                            o.split('=').next().unwrap_or_default(),
                            "--entrypoint" | "--name" | "--rm" | "--detach" | "-d"
                        )
                    }) {
                        bail!("container.options cannot override the job container lifecycle or entrypoint");
                    }
                }
                for volume in spec.volumes {
                    environment
                        .options
                        .push(format!("--volume={}", self.renderer.render(&volume, vars)?));
                }
            } else if matches!(
                environment.kind,
                EnvironmentKind::Container | EnvironmentKind::WindowsContainer
            ) {
                bail!("runner '{name}' requires a job container");
            }
            if matches!(
                environment.kind,
                EnvironmentKind::Container | EnvironmentKind::WindowsContainer
            ) && environment
                .image
                .as_ref()
                .is_none_or(|image| image.trim().is_empty())
            {
                bail!("job container requires a nonempty image");
            }
        }
        let mut env_context = vars.clone();
        let mut env = self.render_map(&self.pipeline.env, &env_context)?;
        env_context.env.extend(env.iter().cloned());
        let environment_env = self.render_map(&environment.env, &env_context)?;
        env_context.env.extend(environment_env.iter().cloned());
        env.extend(environment_env);
        env.extend(self.render_map(&job.env, &env_context)?);
        let render_opt = |value: &Option<String>| -> Result<Option<String>> {
            match value {
                Some(v) => Ok(Some(self.renderer.render(v, vars)?)),
                None => Ok(None),
            }
        };
        let defaults = ContainerSettings::default();
        let container = if environment.kind == EnvironmentKind::WindowsContainer {
            defaults.clone()
        } else {
            ContainerSettings {
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
            }
        };
        // The legacy job cwd remains an alias during workflow migration.
        let cwd = match render_opt(&job.defaults.run.working_directory)?
            .or(render_opt(&job.cwd)?)
            .or(render_opt(&self.pipeline.defaults.run.working_directory)?)
            .or(render_opt(&environment.cwd)?)
        {
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
            shell: render_opt(&job.defaults.run.shell)?
                .or(render_opt(&self.pipeline.defaults.run.shell)?)
                .or(render_opt(&environment.shell)?),
            mounts: self.render_mounts(&environment.mounts, vars)?,
            env,
            options: environment
                .options
                .iter()
                .map(|o| {
                    if job.container.is_some() {
                        Ok(o.clone())
                    } else {
                        self.renderer.render(o, vars)
                    }
                })
                .collect::<Result<_>>()?,
            bash: PathBuf::from(match &environment.bash {
                Some(b) => self.renderer.render(b, vars)?,
                None => "bash".to_string(),
            }),
            windows: if environment.kind == EnvironmentKind::WindowsContainer {
                Some(windows_container::Settings {
                    ssh: render_opt(&environment.ssh)?.unwrap_or_default(),
                    start: render_opt(&environment.start)?.unwrap_or_default(),
                    stop: render_opt(&environment.stop)?.unwrap_or_default(),
                    docker: render_opt(&environment.engine)?.unwrap_or_else(|| "docker.exe".into()),
                    image: render_opt(&environment.image)?.unwrap_or_default(),
                    context: render_opt(&environment.image_context)?.map(PathBuf::from),
                    init: render_opt(&environment.init)?.unwrap_or_default(),
                    options: environment
                        .options
                        .iter()
                        .map(|o| {
                            if job.container.is_some() {
                                Ok(o.clone())
                            } else {
                                self.renderer.render(o, vars)
                            }
                        })
                        .collect::<Result<_>>()?,
                    copy: self.render_mounts(&environment.copy, vars)?,
                    collect: environment
                        .collect
                        .iter()
                        .map(|(guest, local)| {
                            Ok((
                                self.renderer.render(guest, vars)?,
                                PathBuf::from(self.renderer.render(local, vars)?),
                            ))
                        })
                        .collect::<Result<_>>()?,
                })
            } else {
                None
            },
            msystem: match &environment.msystem {
                Some(m) => self.renderer.render(m, vars)?,
                None => "CLANG64".to_string(),
            },
        })
    }

    async fn run_job(
        &self,
        name: &str,
        job: &Job,
        label: &str,
        mut vars: Vars,
        timings: &mut Vec<Timing>,
    ) -> Result<JobOutputs> {
        self.refresh_vars(&mut vars);
        let environment = self.resolve_environment(job, &vars)?;
        if environment.kind == EnvironmentKind::WindowsContainer
            && job.steps.iter().any(|step| {
                step.platform.is_some() || !step.mounts.is_empty() || !step.options.is_empty()
            })
        {
            bail!("Windows job '{name}' shares one container; put mounts and options in its environment");
        }
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

        let linux = if environment.kind == EnvironmentKind::Container && job.container.is_some() {
            let mut run = ContainerRun::new(&environment.image)
                .flags(environment.options.clone())
                .envs(environment.env.clone());
            for (local, guest) in &environment.mounts {
                run = run.mount(local, guest);
            }
            if let Some(platform) = &environment.platform {
                run = run.platform(platform);
            }
            Some(LinuxSession::new(environment.container.engine, run))
        } else {
            None
        };
        let mut windows = match &environment.windows {
            Some(settings) if !self.opts.show => Some(WindowsSession::new(settings.clone())?),
            Some(settings) => {
                windows_container::show(settings);
                None
            }
            None => None,
        };
        let prepare = match (&mut windows, &linux) {
            (Some(session), _) => session.prepare(),
            (_, Some(session)) => {
                session.prepare(mounts.as_ref().expect("container mounts"), self.opts.show)
            }
            _ => Ok(()),
        };
        let result = match prepare {
            Ok(()) => {
                self.run_steps(
                    name,
                    job,
                    &environment,
                    mounts.as_ref(),
                    windows.as_ref(),
                    linux.as_ref(),
                    &mut vars,
                    timings,
                )
                .await
            }
            Err(error) => Err(error),
        };
        let windows_finish = match &windows {
            Some(session) => session.finish(),
            None => Ok(()),
        };
        let linux_finish = match &linux {
            Some(session) if !self.opts.show => session.finish(),
            _ => Ok(()),
        };
        // Bring results back, also after a failure (logs, partial output).
        let finish = match &mounts {
            Some(m) if !self.opts.show => m.finish(),
            _ => Ok(()),
        };
        if result.is_err() || result.as_ref().is_ok_and(|outputs| outputs.error.is_some()) {
            if let Err(error) = &finish {
                tracing::warn!("Container mount collection: {error:#}");
            }
            if let Err(error) = &linux_finish {
                tracing::warn!("Linux job cleanup: {error:#}");
            }
            if let Err(error) = &windows_finish {
                tracing::warn!("Windows job cleanup: {error:#}");
            }
        }
        let mut outputs = result?;
        for cleanup in [finish, windows_finish, linux_finish] {
            if let Err(error) = cleanup {
                outputs.error.get_or_insert(error);
            }
        }
        let named = job
            .outputs
            .iter()
            .map(|(name, template)| {
                Ok((
                    name.clone(),
                    serde_json::Value::String(self.renderer.render(template, &vars)?),
                ))
            })
            .collect::<Result<_>>()?;
        Ok(JobOutputs { named, ..outputs })
    }

    async fn run_steps(
        &self,
        job_name: &str,
        job: &Job,
        environment: &ResolvedEnvironment,
        mounts: Option<&Mounts>,
        windows: Option<&WindowsSession>,
        linux: Option<&LinuxSession>,
        vars: &mut Vars,
        timings: &mut Vec<Timing>,
    ) -> Result<JobOutputs> {
        let mut all_outputs: Outputs = Vec::new();
        let mut job_env = IndexMap::<String, String>::new();
        let mut paths = Vec::new();
        let mut first_error = None;
        vars.env.extend(environment.env.iter().cloned());
        for (index, step) in job.steps.iter().enumerate() {
            if crate::system::process::interrupted() {
                bail!("release interrupted");
            }
            let step_label = step
                .name
                .clone()
                .unwrap_or_else(|| format!("step {}", index + 1));
            let report_label = match &step.name {
                Some(name) => format!("{}. {name}", index + 1),
                None => step
                    .uses
                    .as_ref()
                    .map(|action| format!("{step_label}: {action}"))
                    .unwrap_or_else(|| step_label.clone()),
            };
            let started = Instant::now();
            // Foreach is a Ship extension. Evaluate its condition per item so
            // conditions can actually refer to item (e.g. symbol directories).
            let items = match self.step_items(step, &step_label, vars) {
                Ok(items) => items,
                Err(error) => {
                    timings.push(Timing {
                        label: report_label,
                        status: Status::Failed,
                        elapsed: Some(started.elapsed()),
                    });
                    vars.status.success = false;
                    vars.status.failure = true;
                    if let Some(id) = &step.id {
                        record_step(vars, id, "failure", Default::default());
                    }
                    first_error
                        .get_or_insert(error.context(format!("in {step_label} of job {job_name}")));
                    continue;
                }
            };
            if items.is_empty() {
                timings.push(Timing::skipped(report_label.clone()));
                if let Some(id) = &step.id {
                    record_step(vars, id, "skipped", Default::default());
                }
            }
            for (iteration, item) in items.into_iter().enumerate() {
                let label = item
                    .as_ref()
                    .map(|item| {
                        format!(
                            "{report_label} [item {}: {}]",
                            iteration + 1,
                            value_text(item)
                        )
                    })
                    .unwrap_or_else(|| report_label.clone());
                let step_vars = item
                    .map(|item| vars.with_item(item))
                    .unwrap_or_else(|| vars.clone());
                let started = Instant::now();
                let result = async {
                    if !self
                        .renderer
                        .condition(step.condition.as_deref().unwrap_or("success()"), &step_vars)?
                    {
                        return Ok(None);
                    }
                    self.run_step(
                        step,
                        &step_label,
                        environment,
                        mounts,
                        windows,
                        linux,
                        &step_vars,
                        &job_env,
                        &paths,
                    )
                    .await
                    .map(Some)
                }
                .await
                .context(format!("in {step_label} of job {job_name}"));
                match result {
                    Ok(None) => {
                        timings.push(Timing::skipped(label));
                        if let Some(id) = &step.id {
                            record_step(vars, id, "skipped", Default::default());
                        }
                    }
                    Ok(Some(output)) => {
                        let failed = output.error.is_some();
                        if let Some(id) = &step.id {
                            record_step(
                                vars,
                                id,
                                if failed { "failure" } else { "success" },
                                output.commands.outputs,
                            );
                        }
                        if let Some(error) = output.error {
                            vars.status.success = false;
                            vars.status.failure = true;
                            first_error.get_or_insert(
                                error.context(format!("in {step_label} of job {job_name}")),
                            );
                        }
                        for (key, value) in output.commands.env {
                            vars.env.insert(key.clone(), value.clone());
                            job_env.insert(key, value);
                        }
                        for path in output.commands.paths {
                            paths.retain(|p| p != &path);
                            paths.push(path);
                        }
                        for (key, value) in &output.legacy {
                            vars.set_output(key, value.clone())?;
                        }
                        all_outputs.extend(output.legacy);
                        self.refresh_vars(vars);
                        timings.push(Timing {
                            label,
                            status: if failed {
                                Status::Failed
                            } else if self.opts.dry_run
                                && step.run.is_some()
                                && step.dry_run == DryRunMode::Echo
                            {
                                Status::Echoed
                            } else {
                                Status::Ok
                            },
                            elapsed: Some(started.elapsed()),
                        });
                    }
                    Err(error) => {
                        vars.status.success = false;
                        vars.status.failure = true;
                        if let Some(id) = &step.id {
                            record_step(vars, id, "failure", Default::default());
                        }
                        timings.push(Timing {
                            label,
                            status: Status::Failed,
                            elapsed: Some(started.elapsed()),
                        });
                        tracing::error!("{error:#}");
                        first_error.get_or_insert(error);
                    }
                }
            }
        }
        Ok(JobOutputs {
            legacy: all_outputs,
            error: first_error,
            ..Default::default()
        })
    }

    fn step_items(
        &self,
        step: &Step,
        _label: &str,
        vars: &Vars,
    ) -> Result<Vec<Option<serde_json::Value>>> {
        match &step.foreach {
            Some(list) => {
                // Do not expand a loop's unavailable inputs after an earlier
                // failure unless its condition explicitly requests recovery.
                if !vars.status.success {
                    let condition = step.condition.as_deref().unwrap_or("success()");
                    let per_item_recovery =
                        if condition.contains("{{") && !condition.contains("${{") {
                            false
                        } else {
                            let expression =
                                expressions::parse(super::context::condition_source(condition)?)?;
                            expression.has_status_check() && expression.references("item")
                        };
                    if !per_item_recovery && !self.renderer.condition(condition, vars)? {
                        return Ok(Vec::new());
                    }
                }
                Ok(self
                    .renderer
                    .render_list(list, vars)?
                    .into_iter()
                    .map(Some)
                    .collect())
            }
            None => Ok(vec![None]),
        }
    }

    async fn run_step(
        &self,
        step: &Step,
        label: &str,
        environment: &ResolvedEnvironment,
        mounts: Option<&Mounts>,
        windows: Option<&WindowsSession>,
        linux: Option<&LinuxSession>,
        vars: &Vars,
        job_env: &IndexMap<String, String>,
        paths: &[String],
    ) -> Result<StepResult> {
        let step_env = self.render_map(&step.env, vars)?;
        let mut step_context = vars.clone();
        step_context.env.extend(step_env.iter().cloned());
        let vars = &step_context;
        if let Some(action) = &step.uses {
            let with = self.renderer.render_yaml(&step.with, vars)?;
            let env = ActionEnv {
                dry_run: self.opts.dry_run,
                signing_service_url: self.signing_service_url.as_deref(),
                config: &vars.config,
            };
            if self.opts.show {
                println!("   uses {action}: {}", yaml_inline(&with));
            } else {
                tracing::info!("-- {label}: {action}");
            }
            let outputs = if !self.opts.show || actions::find(action)?.runs_in_show() {
                actions::run(action, &with, &env).await?.outputs
            } else {
                Vec::new()
            };
            return if step.id.is_some() {
                Ok(StepResult {
                    commands: Commands {
                        outputs: outputs
                            .into_iter()
                            .map(|(key, value)| (key, value_text(&value)))
                            .collect(),
                        ..Default::default()
                    },
                    ..Default::default()
                })
            } else {
                Ok(StepResult {
                    legacy: outputs,
                    ..Default::default()
                })
            };
        }

        let shell = step
            .shell
            .as_ref()
            .map(|s| self.renderer.render(s, vars))
            .transpose()?
            .or(environment.shell.clone());
        let default_shell = match environment.kind {
            EnvironmentKind::WindowsContainer => "powershell",
            EnvironmentKind::Container => "sh",
            _ => "bash",
        };
        let shell_name = shell.as_deref().unwrap_or(default_shell);
        if !matches!(shell_name, "bash" | "sh" | "powershell" | "pwsh") {
            bail!("unsupported shell '{shell_name}' (supported: bash, sh, powershell, pwsh)");
        }
        let script = self
            .renderer
            .render(step.run.as_deref().unwrap_or_default(), vars)?;
        let script = commands::prepend_path(
            script.trim(),
            shell_name,
            paths,
            matches!(
                environment.kind,
                EnvironmentKind::WindowsContainer | EnvironmentKind::Msys2
            ) || (environment.kind == EnvironmentKind::Host && cfg!(windows)),
        );
        let mut env = environment.env.clone();
        env.extend(job_env.iter().map(|(k, v)| (k.clone(), v.clone())));
        env.extend(step_env);
        let cwd = step
            .cwd
            .as_ref()
            .map(|cwd| self.renderer.render(cwd, vars).map(PathBuf::from))
            .transpose()?
            .or(environment.cwd.clone());
        let root = match environment.kind {
            EnvironmentKind::Container => Some("/tmp"),
            EnvironmentKind::WindowsContainer => Some("C:/"),
            _ => None,
        };
        let files = Files::new(root)?;
        let msys = environment.kind == EnvironmentKind::Msys2
            || (environment.kind == EnvironmentKind::WindowsContainer
                && matches!(shell_name, "bash" | "sh"));
        env.extend(files.env(msys));
        let echo_only = self.opts.dry_run && step.dry_run == DryRunMode::Echo;

        if environment.kind == EnvironmentKind::WindowsContainer {
            if self.opts.show {
                windows_container::show_step(&script, &env, shell_name);
                return Ok(preview_outputs(step));
            }
            tracing::info!("-- {label}");
            let session = windows.expect("Windows job session");
            if !echo_only {
                session.prepare_command_files(&files.guest)?;
            }
            let result = session.run(&script, &env, cwd.as_deref(), shell_name, echo_only);
            if echo_only {
                return Ok(StepResult::default());
            }
            let commands = session
                .command_files(&files.guest)
                .and_then(|texts| Commands::parse(&texts["output"], &texts["env"], &texts["path"]));
            return completed_step(step, label, result, commands);
        }

        static CONTAINERS: std::sync::atomic::AtomicUsize = std::sync::atomic::AtomicUsize::new(0);
        let container_name = format!(
            "ship-{}-{}",
            std::process::id(),
            CONTAINERS.fetch_add(1, std::sync::atomic::Ordering::SeqCst) + 1
        );
        let script = if environment.kind == EnvironmentKind::Container {
            if !matches!(shell_name, "bash" | "sh") {
                bail!("Linux container steps currently support bash and sh");
            }
            format!("{}{script}", files.initialize_sh())
        } else {
            script
        };
        let mut command = shell_command(shell_name, &script, shell.is_some());
        if environment.kind == EnvironmentKind::Msys2 && shell_name == "bash" {
            command[0] = environment.bash.to_string_lossy().into_owned();
            if shell.is_none() {
                command.insert(1, "-l".into());
            }
        }
        let cmd = match environment.kind {
            EnvironmentKind::WindowsContainer => unreachable!("handled above"),
            EnvironmentKind::Host | EnvironmentKind::Msys2 => {
                let mut cmd = Cmd::new(&command[0]).args(&command[1..]).envs(env.clone());
                if environment.kind == EnvironmentKind::Msys2 {
                    cmd = cmd
                        .env("MSYSTEM", &environment.msystem)
                        .env("CHERE_INVOKING", "1");
                }
                if let Some(cwd) = &cwd {
                    cmd = cmd.cwd(cwd);
                }
                cmd
            }
            EnvironmentKind::Container if linux.is_some() => {
                linux.unwrap().command(&command, &env, cwd.as_deref())
            }
            EnvironmentKind::Container => {
                let mounts = mounts.expect("container job has mounts");
                let mut run = ContainerRun::new(&environment.image)
                    .remove_on_exit(false)
                    .name(&container_name)
                    .flags(environment.options.clone())
                    .flags(
                        step.options
                            .iter()
                            .map(|o| self.renderer.render(o, vars))
                            .collect::<Result<Vec<_>>>()?,
                    )
                    .envs(env.clone());
                if let Some(cwd) = &cwd {
                    run = run.flags(["--workdir".to_owned(), cwd.to_string_lossy().into_owned()]);
                }
                for (local, guest) in merge_mounts(
                    &environment.mounts,
                    &self.render_mounts(&step.mounts, vars)?,
                ) {
                    run = run.mount(local, guest);
                }
                if let Some(platform) = step
                    .platform
                    .as_ref()
                    .map(|p| self.renderer.render(p, vars))
                    .transpose()?
                    .or(environment.platform.clone())
                {
                    run = run.platform(platform);
                }
                run.command(command)
                    .to_cmd(environment.container.engine, mounts)
            }
        };
        if self.opts.show {
            let prefix = if step.dry_run == DryRunMode::Echo {
                "   $ (echo in dry run) "
            } else {
                "   $ "
            };
            println!("{prefix}{}", cmd.display());
            if environment.kind != EnvironmentKind::Container {
                for (key, value) in &env {
                    if !key.starts_with("SHIP_") && !key.starts_with("GITHUB_") {
                        println!("       {key}={}", crate::system::process::redacted(value));
                    }
                }
            }
            return Ok(preview_outputs(step));
        }
        tracing::info!("-- {label}");
        let mut result = cmd.run_or_echo(echo_only);
        if environment.kind == EnvironmentKind::Container && !echo_only {
            if !crate::system::process::interrupted() {
                // Copying command files also works with remote engines, without
                // binding a controller temporary directory into the container.
                let copied = match linux {
                    Some(session) => session.collect_commands(&files.guest, &files.local),
                    None => Cmd::new(environment.container.engine.program())
                        .args(["cp", &format!("{container_name}:{}/.", files.guest)])
                        .arg(&files.local)
                        .run(),
                };
                if result.is_ok() {
                    result = copied;
                }
            }
            if linux.is_none() {
                crate::system::container::remove_container(
                    environment.container.engine,
                    &container_name,
                );
            }
        }
        if echo_only {
            result?;
            return Ok(StepResult::default());
        }
        completed_step(step, label, result, files.read())
    }
}

fn record_step(
    vars: &mut Vars,
    id: &str,
    status: &str,
    outputs: std::collections::BTreeMap<String, String>,
) {
    vars.steps.insert(
        id.into(),
        serde_json::json!({"outputs": outputs, "outcome": status, "conclusion": status}),
    );
}

fn shell_command(shell: &str, script: &str, explicit: bool) -> Vec<String> {
    match shell {
        "bash" if explicit => vec![
            "bash".into(),
            "--noprofile".into(),
            "--norc".into(),
            "-e".into(),
            "-o".into(),
            "pipefail".into(),
            "-c".into(),
            script.into(),
        ],
        "bash" | "sh" => vec![shell.into(), "-e".into(), "-c".into(), script.into()],
        "powershell" | "pwsh" => {
            let script = format!("$ErrorActionPreference='Stop'\n$LASTEXITCODE=0\n{script}\nif ($LASTEXITCODE) {{ exit $LASTEXITCODE }}");
            vec![
                shell.into(),
                "-NoProfile".into(),
                "-NonInteractive".into(),
                "-EncodedCommand".into(),
                windows_container::encode(&script),
            ]
        }
        _ => unreachable!("validated shell"),
    }
}

fn preview_outputs(step: &Step) -> StepResult {
    StepResult {
        legacy: step
            .outputs
            .iter()
            .map(|key| (key.clone(), serde_json::Value::String(format!("<{key}>"))))
            .collect(),
        ..Default::default()
    }
}

fn completed_step(
    step: &Step,
    label: &str,
    result: Result<()>,
    commands: Result<Commands>,
) -> Result<StepResult> {
    match result {
        Ok(()) => step_result(step, label, commands?),
        Err(error) => Ok(StepResult {
            commands: commands.unwrap_or_default(),
            error: Some(error),
            ..Default::default()
        }),
    }
}

fn step_result(step: &Step, label: &str, commands: Commands) -> Result<StepResult> {
    let mut legacy = Vec::new();
    if !step.outputs.is_empty() {
        for declared in &step.outputs {
            let value = commands.outputs.get(declared).ok_or_else(|| {
                anyhow!("{label} did not write declared output '{declared}' to SHIP_OUTPUT")
            })?;
            legacy.push((declared.clone(), super::context::parse_output_value(value)));
        }
        for name in commands.outputs.keys() {
            if !step.outputs.contains(name) {
                bail!("{label} wrote undeclared output '{name}'");
            }
        }
    }
    Ok(StepResult {
        legacy,
        commands,
        error: None,
    })
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
        assert_eq!(select_jobs(&pipeline, &only_skip).unwrap(), vec!["a", "b"]);
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
        assert_eq!(select_jobs(&pipeline, &skip).unwrap(), vec!["a", "b", "c"]);
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
