//! The release pipeline file (e.g. `tools/local/release.yaml`).
//!
//! Its structure follows GitHub Actions: `targets` name lists of `jobs`, a
//! job runs its `steps` in an execution environment, `needs` orders jobs,
//! `strategy.matrix` repeats a job, `if` conditions skip jobs and steps.
//! Strings support expressions, rendered when the step runs (see
//! [`super::context`]).

use std::path::Path;

use anyhow::{bail, Context, Result};
use indexmap::IndexMap;
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Pipeline {
    #[serde(skip)]
    pub directory: std::path::PathBuf,
    #[serde(default)]
    pub env: IndexMap<String, String>,
    #[serde(default)]
    pub defaults: Defaults,
    /// Files containing environment definitions, relative to this pipeline.
    #[serde(rename = "environment-files", default)]
    pub environment_files: Vec<String>,
    /// Machine definitions, separate from the images selected by jobs.
    #[serde(rename = "runner-files", default)]
    pub runner_files: Vec<String>,
    #[serde(default)]
    pub runners: IndexMap<String, RunnerSpec>,
    /// What the engine itself needs; templates over `config` and `options`.
    #[serde(default)]
    pub settings: Settings,
    /// The pipeline's own command line options, available as
    /// `{{ options.<name> }}` (`--<name> <value>`, or `--<name>` for flags).
    #[serde(default)]
    pub options: IndexMap<String, OptionSpec>,
    /// Values computed once per job from the context, available as
    /// `{{ vars.<name> }}`; later entries can use earlier ones.
    #[serde(default)]
    pub vars: IndexMap<String, String>,
    #[serde(default)]
    pub environments: IndexMap<String, Environment>,
    #[serde(default)]
    pub targets: IndexMap<String, Vec<String>>,
    #[serde(default)]
    pub jobs: IndexMap<String, Job>,
}

#[derive(Debug, Clone, Default, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Defaults {
    #[serde(default)]
    pub run: RunDefaults,
}

#[derive(Debug, Clone, Default, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct RunDefaults {
    pub shell: Option<String>,
    #[serde(rename = "working-directory")]
    pub working_directory: Option<String>,
}

#[derive(Debug, Clone, Default, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct OptionSpec {
    /// One-letter alias, e.g. `t` for `-t`.
    pub short: Option<char>,
    /// Shown in `--help`.
    pub help: Option<String>,
    /// A boolean switch instead of a value.
    #[serde(default)]
    pub flag: bool,
    /// The value when the option is not given (empty, or false for a flag).
    /// Not used for options that set a configuration key.
    pub default: Option<serde_yaml::Value>,
    /// Instead of `{{ options.<name> }}`, the option overrides this
    /// configuration key for the run, e.g. `linux.ppa_increment`.
    pub config: Option<String>,
    /// For a flag: the value it sets (default `true`); `false` makes a
    /// `--no-something` switch.
    pub value: Option<serde_yaml::Value>,
}

#[derive(Debug, Clone, Default, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Settings {
    /// The signing service used by `secret()` and the `sign` action.
    pub signing_service_url: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Deserialize, serde::Serialize)]
#[serde(rename_all = "lowercase")]
pub enum EnvironmentKind {
    /// Commands run on this machine with `bash -c`.
    Host,
    /// Commands run in a build container with `<engine> run`.
    Container,
    /// Commands run in an MSYS2 login shell (Windows).
    Msys2,
    /// A persistent Windows Docker container, locally or through SSH.
    #[serde(rename = "windows-container")]
    WindowsContainer,
}

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Environment {
    #[serde(rename = "type")]
    pub kind: EnvironmentKind,
    /// container: the image to run.
    pub image: Option<String>,
    /// container: `podman` (default) or `docker`.
    pub engine: Option<String>,
    /// container: `auto` (default), `rsync` or `none` — whether to mirror the
    /// mounted directories to a remote podman host.
    pub sync: Option<String>,
    /// container: directory on the remote host (relative to its home) under
    /// which local directories are mirrored.
    #[serde(rename = "remote-dir")]
    pub remote_dir: Option<String>,
    /// container: default `--platform`.
    pub platform: Option<String>,
    /// Default working directory on the execution machine.
    #[serde(rename = "working-directory", alias = "cwd")]
    pub cwd: Option<String>,
    /// container: local directory -> path in the container. An entry whose
    /// key renders empty is dropped (optional directories).
    #[serde(default)]
    pub mounts: IndexMap<String, String>,
    #[serde(default)]
    pub env: IndexMap<String, String>,
    /// container: extra `run` flags, e.g. `--privileged`.
    #[serde(default)]
    pub options: Vec<String>,
    /// msys2: path to bash.exe.
    pub bash: Option<String>,
    /// msys2: MSYSTEM, e.g. CLANG64.
    pub msystem: Option<String>,
    /// Windows container: SSH destination. Empty means local Docker on Windows.
    pub ssh: Option<String>,
    /// Windows container: optional commands on the machine running Ship,
    /// before/after the job.
    pub start: Option<String>,
    pub stop: Option<String>,
    /// Windows container: powershell (default) or bash.
    pub shell: Option<String>,
    /// Windows container: optional build context, rebuilt when contents change.
    #[serde(rename = "image-context")]
    pub image_context: Option<String>,
    /// Windows container: local inputs -> paths in the container's local layer.
    #[serde(default)]
    pub copy: IndexMap<String, String>,
    /// Windows container: output directories -> local directories, even on failure.
    #[serde(default)]
    pub collect: IndexMap<String, String>,
    /// Windows container: shell initialization before each step.
    pub init: Option<String>,
}

/// Ship's machine binding for a runs-on label. It never selects a build image.
#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct RunnerSpec {
    #[serde(rename = "type")]
    pub kind: RunnerKind,
    pub engine: Option<String>,
    pub sync: Option<String>,
    #[serde(rename = "remote-dir")]
    pub remote_dir: Option<String>,
    pub ssh: Option<String>,
    pub start: Option<String>,
    pub stop: Option<String>,
}

#[derive(Debug, Clone, Copy, Deserialize, serde::Serialize)]
#[serde(rename_all = "kebab-case")]
pub enum RunnerKind {
    Host,
    LinuxContainerHost,
    WindowsContainerHost,
}

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(untagged)]
pub enum JobContainer {
    Image(String),
    Definition(ContainerSpec),
}

#[derive(Debug, Clone, Default, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct ContainerSpec {
    pub image: String,
    #[serde(default)]
    pub env: IndexMap<String, String>,
    #[serde(default)]
    pub volumes: Vec<String>,
    pub options: Option<String>,
    // Ship extensions: synchronization, native Windows image preparation and
    // copying files between the controller and the container's local layer.
    pub platform: Option<String>,
    #[serde(default)]
    pub mounts: IndexMap<String, String>,
    #[serde(rename = "image-context")]
    pub image_context: Option<String>,
    #[serde(default)]
    pub copy: IndexMap<String, String>,
    #[serde(default)]
    pub collect: IndexMap<String, String>,
    pub init: Option<String>,
}

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Job {
    /// Environment name; `host` by default.
    #[serde(rename = "runs-in")]
    pub runs_in: Option<String>,
    #[serde(rename = "runs-on")]
    pub runs_on: Option<String>,
    pub container: Option<JobContainer>,
    #[serde(default, deserialize_with = "one_or_many")]
    pub needs: Vec<String>,
    #[serde(default)]
    pub outputs: IndexMap<String, String>,
    #[serde(default)]
    pub defaults: Defaults,
    #[serde(rename = "if", default, deserialize_with = "condition")]
    pub condition: Option<String>,
    /// Runs even when `--job` selects other jobs (unless `--skip-job`ed):
    /// for cheap prerequisites such as the workspace job that sets the
    /// version.
    #[serde(default)]
    pub always: bool,
    pub strategy: Option<Strategy>,
    #[serde(default)]
    pub env: IndexMap<String, String>,
    /// host/msys2: working directory of the steps.
    pub cwd: Option<String>,
    #[serde(default)]
    pub steps: Vec<Step>,
}

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Strategy {
    pub matrix: serde_yaml::Value,
    #[serde(rename = "fail-fast", default = "default_true")]
    pub fail_fast: bool,
    #[serde(rename = "max-parallel")]
    pub max_parallel: Option<usize>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Deserialize, serde::Serialize)]
#[serde(rename_all = "lowercase")]
pub enum DryRunMode {
    /// The step runs in a dry run too (builds, local file handling).
    #[default]
    Run,
    /// The command is only printed in a dry run (uploads, pushes).
    Echo,
}

#[derive(Debug, Clone, Deserialize, serde::Serialize)]
#[serde(deny_unknown_fields)]
pub struct Step {
    pub id: Option<String>,
    pub name: Option<String>,
    #[serde(rename = "if", default, deserialize_with = "condition")]
    pub condition: Option<String>,
    /// A shell command to run in the job's environment.
    pub run: Option<String>,
    /// A builtin action (see `actions.rs`).
    pub uses: Option<String>,
    /// Parameters of the action.
    #[serde(default)]
    pub with: serde_yaml::Value,
    /// Renders to a list; the step runs once per element as `{{ item }}`.
    pub foreach: Option<String>,
    #[serde(default)]
    pub env: IndexMap<String, String>,
    #[serde(rename = "working-directory", alias = "cwd")]
    pub cwd: Option<String>,
    pub shell: Option<String>,
    /// container: `--platform` for this step.
    pub platform: Option<String>,
    /// container: extra mounts for this step; one with the same destination
    /// as an environment mount replaces it.
    #[serde(default)]
    pub mounts: IndexMap<String, String>,
    /// container: extra `run` flags for this step.
    #[serde(default)]
    pub options: Vec<String>,
    #[serde(rename = "dry-run", default)]
    pub dry_run: DryRunMode,
    /// Legacy global outputs. New workflows use `id` and scoped outputs.
    /// Dotted names nest:
    /// `version.tag=v1` is `{{ version.tag }}`. `true`/`false` become
    /// booleans. Supported in every execution environment.
    #[serde(default)]
    pub outputs: Vec<String>,
}

fn default_true() -> bool {
    true
}

fn condition<'de, D: serde::Deserializer<'de>>(
    deserializer: D,
) -> Result<Option<String>, D::Error> {
    let value = Option::<serde_yaml::Value>::deserialize(deserializer)?;
    match value {
        None => Ok(None),
        Some(serde_yaml::Value::String(value)) => Ok(Some(value)),
        Some(serde_yaml::Value::Bool(value)) => Ok(Some(value.to_string())),
        _ => Err(serde::de::Error::custom(
            "if must be a boolean or expression string",
        )),
    }
}

fn valid_id(id: &str) -> bool {
    let mut chars = id.chars();
    matches!(chars.next(), Some(c) if c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_' || c == '-')
}

fn one_or_many<'de, D: serde::Deserializer<'de>>(deserializer: D) -> Result<Vec<String>, D::Error> {
    #[derive(Deserialize, serde::Serialize)]
    #[serde(untagged)]
    enum Names {
        One(String),
        Many(Vec<String>),
    }
    Ok(match Names::deserialize(deserializer)? {
        Names::One(name) => vec![name],
        Names::Many(names) => names,
    })
}

fn validate_expressions(value: &serde_yaml::Value, path: &str) -> Result<()> {
    match value {
        serde_yaml::Value::String(value) => {
            let mut rest = value.as_str();
            while let Some(start) = rest.find("${{") {
                rest = &rest[start + 3..];
                let end = super::expressions::end(rest).with_context(|| format!("in {path}"))?;
                super::expressions::parse(&rest[..end]).with_context(|| format!("in {path}"))?;
                rest = &rest[end + 2..];
            }
        }
        serde_yaml::Value::Mapping(values) => {
            for (key, value) in values {
                validate_expressions(key, path)?;
                validate_expressions(value, &format!("{path}.{}", key.as_str().unwrap_or("?")))?;
            }
        }
        serde_yaml::Value::Sequence(values) => {
            for (index, value) in values.iter().enumerate() {
                validate_expressions(value, &format!("{path}[{index}]"))?;
            }
        }
        _ => (),
    }
    Ok(())
}

impl Pipeline {
    pub fn load(path: &Path) -> Result<Pipeline> {
        let raw = std::fs::read_to_string(path)
            .with_context(|| format!("reading pipeline file {}", path.display()))?;
        let mut pipeline: Pipeline = serde_yaml::from_str(&raw).context("parsing YAML")?;
        pipeline.directory = path
            .canonicalize()?
            .parent()
            .unwrap_or(Path::new("."))
            .to_owned();
        for file in &pipeline.environment_files {
            #[derive(Deserialize, serde::Serialize)]
            #[serde(deny_unknown_fields)]
            struct Environments {
                environments: IndexMap<String, Environment>,
            }
            let file = path.parent().unwrap_or(Path::new(".")).join(file);
            let definitions: Environments = serde_yaml::from_str(
                &std::fs::read_to_string(&file)
                    .with_context(|| format!("reading environments {}", file.display()))?,
            )?;
            for (name, environment) in definitions.environments {
                if pipeline
                    .environments
                    .insert(name.clone(), environment)
                    .is_some()
                {
                    bail!("duplicate environment '{name}' in {}", file.display());
                }
            }
        }
        for file in &pipeline.runner_files {
            #[derive(Deserialize)]
            #[serde(deny_unknown_fields)]
            struct Runners {
                runners: IndexMap<String, RunnerSpec>,
            }
            let file = pipeline.directory.join(file);
            let definitions: Runners = serde_yaml::from_str(
                &std::fs::read_to_string(&file)
                    .with_context(|| format!("reading runners {}", file.display()))?,
            )?;
            for (name, runner) in definitions.runners {
                if pipeline.runners.insert(name.clone(), runner).is_some() {
                    bail!("duplicate runner '{name}' in {}", file.display());
                }
            }
        }
        pipeline
            .validate()
            .with_context(|| format!("in pipeline file {}", path.display()))?;
        Ok(pipeline)
    }

    #[cfg(test)]
    pub fn parse(raw: &str) -> Result<Pipeline> {
        let pipeline: Pipeline = serde_yaml::from_str(raw).context("parsing YAML")?;
        pipeline.validate()?;
        Ok(pipeline)
    }

    fn validate(&self) -> Result<()> {
        validate_expressions(&serde_yaml::to_value(self)?, "workflow")?;
        const ENGINE_OPTIONS: [&str; 8] = [
            "target", "job", "skip-job", "dry-run", "set", "config", "profile", "pipeline",
        ];
        const ENGINE_SHORTS: [char; 5] = ['T', 'j', 'd', 'f', 'p'];
        for (name, runner) in &self.runners {
            if name == "host" {
                bail!("runner name 'host' is reserved for the machine running Ship");
            }
            if !matches!(runner.kind, RunnerKind::WindowsContainerHost)
                && (runner.ssh.is_some() || runner.start.is_some() || runner.stop.is_some())
            {
                bail!("runner '{name}': ssh/start/stop require windows-container-host; Linux transport uses the configured Podman connection");
            }
            if matches!(runner.kind, RunnerKind::WindowsContainerHost)
                && (runner.sync.is_some() || runner.remote_dir.is_some())
            {
                bail!("runner '{name}': sync/remote-dir require linux-container-host; Windows uses container copy/collect");
            }
        }
        for (name, spec) in &self.options {
            if ENGINE_OPTIONS.contains(&name.as_str()) {
                bail!("option '{name}' is reserved for ship itself");
            }
            if !name
                .chars()
                .all(|c| c.is_ascii_alphanumeric() || c == '-' || c == '_')
                || name.is_empty()
            {
                bail!("option '{name}': names are letters, digits, - and _");
            }
            if let Some(short) = spec.short {
                if ENGINE_SHORTS.contains(&short) || short == 'h' || short == 'V' {
                    bail!("option '{name}': -{short} is reserved for ship itself");
                }
            }
            if spec.value.is_some() && !spec.flag {
                bail!("option '{name}': `value` only applies to flags");
            }
        }
        for (target, jobs) in &self.targets {
            for job in jobs {
                if !self.jobs.contains_key(job) {
                    bail!("target '{target}' lists unknown job '{job}'");
                }
            }
        }
        for (name, job) in &self.jobs {
            if job.runs_in.is_some() && (job.runs_on.is_some() || job.container.is_some()) {
                bail!("job '{name}': runs-in cannot be combined with runs-on or container");
            }
            if let Some(runner) = job.runs_on.as_ref().filter(|r| !r.contains("{{")) {
                if runner != "host" && !self.runners.contains_key(runner) {
                    bail!("job '{name}' selects unknown runner '{runner}'");
                }
            }
            if job.container.is_some()
                && job
                    .steps
                    .iter()
                    .any(|s| s.platform.is_some() || !s.mounts.is_empty() || !s.options.is_empty())
            {
                bail!("job '{name}': a job container is shared by every step; put platform, mounts and options in container");
            }
            if job.container.is_some() && job.steps.iter().any(|s| s.uses.is_some()) {
                bail!("job '{name}': legacy built-in uses steps run on the controller; use run: ship action inside a job container");
            }
            if job
                .strategy
                .as_ref()
                .is_some_and(|s| s.max_parallel == Some(0))
            {
                bail!("job '{name}': max-parallel must be positive");
            }
            if let Some(condition) = &job.condition {
                if condition.contains("{{")
                    && !condition.contains("${{")
                    && condition.contains("matrix.")
                {
                    bail!("job '{name}': job if is evaluated before matrix expansion");
                }
                if !condition.contains("{{") || condition.contains("${{") {
                    let expression =
                        super::expressions::parse(super::context::condition_source(condition)?)?;
                    if expression.references("matrix") {
                        bail!("job '{name}': job if is evaluated before matrix expansion; move matrix filtering into strategy.matrix");
                    }
                }
            }
            if let Some(env) = job.runs_in.as_ref().filter(|name| !name.contains("{{")) {
                let environment = self.environments.get(env).ok_or_else(|| {
                    anyhow::anyhow!("job '{name}' runs in unknown environment '{env}'")
                })?;
                if matches!(
                    environment.kind,
                    EnvironmentKind::Container | EnvironmentKind::WindowsContainer
                ) && environment.image.is_none()
                {
                    bail!("environment '{env}' is a container but has no image");
                }
                if environment.kind == EnvironmentKind::Msys2 && environment.bash.is_none() {
                    bail!("environment '{env}' is msys2 but has no bash");
                }
            }
            for need in &job.needs {
                if !self.jobs.contains_key(need) {
                    bail!("job '{name}' needs unknown job '{need}'");
                }
            }
            let mut ids = std::collections::HashSet::new();
            for (i, step) in job.steps.iter().enumerate() {
                let what = format!("step {} of job '{name}'", i + 1);
                if let Some(condition) = &step.condition {
                    if !condition.contains("{{") || condition.contains("${{") {
                        super::expressions::parse(super::context::condition_source(condition)?)
                            .with_context(|| format!("in if of {what}"))?;
                    }
                }
                if let Some(id) = &step.id {
                    if !valid_id(id) {
                        bail!("{what} has invalid id '{id}' (start with a letter or _, then letters, digits, - or _)");
                    }
                    if !ids.insert(id) {
                        bail!("duplicate step id '{id}' in job '{name}'");
                    }
                    if step.foreach.is_some() || !step.outputs.is_empty() {
                        bail!("{what}: `id` cannot be combined with legacy `outputs` or `foreach`");
                    }
                }
                if !step.outputs.is_empty() {
                    if step.run.is_none() {
                        bail!("{what} has `outputs` but is not a `run` step");
                    }
                }
                match (&step.run, &step.uses) {
                    (Some(_), Some(_)) => bail!("{what} has both `run` and `uses`"),
                    (None, None) => bail!("{what} has neither `run` nor `uses`"),
                    (Some(_), None) => {
                        if !step.with.is_null() {
                            bail!("{what} has `with` but is a `run` step");
                        }
                    }
                    (None, Some(action)) => {
                        crate::services::actions::validate(action, &step.with)
                            .with_context(|| format!("in {what}"))?;
                        if step.platform.is_some()
                            || !step.mounts.is_empty()
                            || !step.options.is_empty()
                            || step.shell.is_some()
                            || step.cwd.is_some()
                        {
                            bail!("{what} is a `uses` step; platform/mounts/options/shell/working-directory only apply to `run` steps");
                        }
                    }
                }
            }
        }
        Ok(())
    }

    /// The selected jobs in execution order: every job after the jobs it
    /// needs, including dependencies outside the initial selection. Independent
    /// jobs retain selection order; the executor currently runs serially.
    pub fn ordered_jobs(&self, selected: &[String]) -> Result<Vec<String>> {
        let mut ordered: Vec<String> = Vec::new();
        let mut visiting: Vec<String> = Vec::new();

        fn visit(
            pipeline: &Pipeline,
            selected: &[String],
            name: &str,
            ordered: &mut Vec<String>,
            visiting: &mut Vec<String>,
        ) -> Result<()> {
            if ordered.iter().any(|j| j == name) {
                return Ok(());
            }
            if visiting.iter().any(|j| j == name) {
                bail!("job dependency cycle: {} -> {name}", visiting.join(" -> "));
            }
            visiting.push(name.to_string());
            for need in &pipeline.jobs[name].needs {
                visit(pipeline, selected, need, ordered, visiting)?;
            }
            visiting.pop();
            ordered.push(name.to_string());
            Ok(())
        }

        for name in selected {
            visit(self, selected, name, &mut ordered, &mut visiting)?;
        }
        Ok(ordered)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const SAMPLE: &str = r#"
environments:
  ubuntu:
    type: container
    image: img
targets:
  linux: [workspace, appimage, ppa, github]
jobs:
  workspace:
    steps:
      - run: git clone r
        outputs: [version.tag]
  changelogs:
    needs: [workspace]
    steps:
      - uses: newsgen
        with: { input: i, template: t, output: o }
  appimage:
    needs: [workspace]
    runs-in: ubuntu
    strategy: { matrix: { platform: [a, b] } }
    steps:
      - run: build.sh
        platform: "{{ matrix.platform }}"
  ppa:
    needs: [changelogs]
    runs-in: ubuntu
    steps:
      - run: ppa.sh
  github:
    needs: [appimage, ppa]
    steps:
      - uses: github-release
        with: { tag: t, title: t, notes: n, assets: [x], repo: r, token: k }
"#;

    #[test]
    fn parses_and_orders() {
        let p = Pipeline::parse(SAMPLE).unwrap();
        let selected: Vec<String> = p.targets["linux"].clone();
        // Dependencies are included even when omitted from the selected roots.
        assert_eq!(
            p.ordered_jobs(&selected).unwrap(),
            vec!["workspace", "appimage", "changelogs", "ppa", "github"]
        );
        let all: Vec<String> = p.jobs.keys().cloned().collect();
        assert_eq!(
            p.ordered_jobs(&all).unwrap(),
            vec!["workspace", "changelogs", "appimage", "ppa", "github"]
        );
        // Selection order is kept where needs allow it.
        let reordered: Vec<String> = ["github", "ppa", "changelogs", "workspace", "appimage"]
            .iter()
            .map(|s| s.to_string())
            .collect();
        assert_eq!(
            p.ordered_jobs(&reordered).unwrap(),
            vec!["workspace", "appimage", "changelogs", "ppa", "github"]
        );
    }

    #[test]
    fn needs_can_reorder_file_order() {
        let p = Pipeline::parse(
            "jobs:\n  b:\n    needs: [a]\n    steps: [{run: x}]\n  a:\n    steps: [{run: y}]\n",
        )
        .unwrap();
        let all: Vec<String> = p.jobs.keys().cloned().collect();
        assert_eq!(p.ordered_jobs(&all).unwrap(), vec!["a", "b"]);
    }

    #[test]
    fn detects_cycles() {
        let p = Pipeline::parse(
            "jobs:\n  a:\n    needs: [b]\n    steps: [{run: x}]\n  b:\n    needs: [a]\n    steps: [{run: y}]\n",
        )
        .unwrap();
        let all: Vec<String> = p.jobs.keys().cloned().collect();
        let err = p.ordered_jobs(&all).unwrap_err().to_string();
        assert!(err.contains("cycle"), "{err}");
    }

    #[test]
    fn rejects_invalid_pipelines() {
        for (yaml, expected) in [
            ("targets: {x: [nope]}\n", "unknown job 'nope'"),
            ("options:\n  target: {}\n", "reserved for ship"),
            ("options:\n  x: {short: d}\n", "-d is reserved"),
            ("jobs:\n  a:\n    runs-in: nope\n    steps: [{run: x}]\n", "unknown environment"),
            ("jobs:\n  a:\n    steps: [{run: x, uses: y}]\n", "both `run` and `uses`"),
            ("jobs:\n  a:\n    steps: [{name: x}]\n", "neither `run` nor `uses`"),
            ("jobs:\n  a:\n    steps: [{uses: nope}]\n", "unknown action"),
            ("jobs:\n  a:\n    steps: [{uses: newsgen, with: {input: i, template: t}}]\n", "missing field `output`"),
            ("jobs:\n  a:\n    steps: [{run: x, bogus: 1}]\n", "unknown field `bogus`"),
            ("environments:\n  c:\n    type: container\njobs:\n  a:\n    runs-in: c\n    steps: [{run: x}]\n", "no image"),
            ("jobs:\n  a:\n    steps: [{uses: newsgen, with: {input: i, template: t, output: o}, outputs: [x]}]\n", "not a `run` step"),
        ] {
            let err = format!("{:#}", Pipeline::parse(yaml).unwrap_err());
            assert!(err.contains(expected), "{yaml}: {err}");
        }
    }
}
