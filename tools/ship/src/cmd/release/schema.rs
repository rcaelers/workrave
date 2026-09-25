//! The release pipeline file (e.g. `tools/local/release.yaml`).
//!
//! Its structure follows GitHub Actions: `targets` name lists of `jobs`, a
//! job runs its `steps` in an execution environment, `needs` orders jobs,
//! `strategy.matrix` repeats a job, `if` conditions skip jobs and steps.
//! Every string value is a Jinja template, rendered when the step runs (see
//! [`super::context`]).

use std::path::Path;

use anyhow::{bail, Context, Result};
use indexmap::IndexMap;
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Pipeline {
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

#[derive(Debug, Clone, Default, Deserialize)]
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

#[derive(Debug, Clone, Default, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Settings {
    /// The signing service used by `secret()` and the `sign` action.
    pub signing_service_url: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum EnvironmentKind {
    /// Commands run on this machine with `bash -c`.
    Host,
    /// Commands run in a build container with `<engine> run`.
    Container,
    /// Commands run in an MSYS2 login shell (Windows).
    Msys2,
}

#[derive(Debug, Clone, Deserialize)]
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
    /// host/msys2: working directory of the commands.
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
}

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Job {
    /// Environment name; `host` by default.
    #[serde(rename = "runs-in")]
    pub runs_in: Option<String>,
    #[serde(default)]
    pub needs: Vec<String>,
    #[serde(rename = "if")]
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

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Strategy {
    pub matrix: IndexMap<String, Vec<serde_yaml::Value>>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum DryRunMode {
    /// The step runs in a dry run too (builds, local file handling).
    #[default]
    Run,
    /// The command is only printed in a dry run (uploads, pushes).
    Echo,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Step {
    pub name: Option<String>,
    #[serde(rename = "if")]
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
    pub cwd: Option<String>,
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
    /// Names the step sets for later steps and jobs, by writing `name=value`
    /// lines to the file in `$SHIP_OUTPUT`. Dotted names nest:
    /// `version.tag=v1` is `{{ version.tag }}`. `true`/`false` become
    /// booleans. Host and msys2 steps only.
    #[serde(default)]
    pub outputs: Vec<String>,
}

impl Pipeline {
    pub fn load(path: &Path) -> Result<Pipeline> {
        let raw = std::fs::read_to_string(path)
            .with_context(|| format!("reading pipeline file {}", path.display()))?;
        Pipeline::parse(&raw).with_context(|| format!("in pipeline file {}", path.display()))
    }

    pub fn parse(raw: &str) -> Result<Pipeline> {
        let pipeline: Pipeline = serde_yaml::from_str(raw).context("parsing YAML")?;
        pipeline.validate()?;
        Ok(pipeline)
    }

    fn validate(&self) -> Result<()> {
        const ENGINE_OPTIONS: [&str; 8] = [
            "target", "job", "skip-job", "dry-run", "set", "config", "profile", "pipeline",
        ];
        const ENGINE_SHORTS: [char; 5] = ['T', 'j', 'd', 'f', 'p'];
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
            if let Some(env) = &job.runs_in {
                let environment = self.environments.get(env).ok_or_else(|| {
                    anyhow::anyhow!("job '{name}' runs in unknown environment '{env}'")
                })?;
                if environment.kind == EnvironmentKind::Container && environment.image.is_none() {
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
            let in_container = job
                .runs_in
                .as_ref()
                .and_then(|env| self.environments.get(env))
                .is_some_and(|env| env.kind == EnvironmentKind::Container);
            for (i, step) in job.steps.iter().enumerate() {
                let what = format!("step {} of job '{name}'", i + 1);
                if !step.outputs.is_empty() {
                    if step.run.is_none() {
                        bail!("{what} has `outputs` but is not a `run` step");
                    }
                    if in_container {
                        bail!("{what} has `outputs`, which container steps cannot set");
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
                        super::actions::validate(action, &step.with)
                            .with_context(|| format!("in {what}"))?;
                        if step.platform.is_some()
                            || !step.mounts.is_empty()
                            || !step.options.is_empty()
                        {
                            bail!("{what} is a `uses` step; platform/mounts/options only apply to `run` steps");
                        }
                    }
                }
            }
        }
        Ok(())
    }

    /// The selected jobs in execution order: every job after the jobs it
    /// needs (only those that are part of the selection count), otherwise in
    /// the order they were selected in (the target's list).
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
                if selected.iter().any(|j| j == need) {
                    visit(pipeline, selected, need, ordered, visiting)?;
                }
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
        // changelogs is needed by ppa but not selected: not pulled in.
        assert_eq!(
            p.ordered_jobs(&selected).unwrap(),
            vec!["workspace", "appimage", "ppa", "github"]
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
            ("environments:\n  c:\n    type: container\n    image: i\njobs:\n  a:\n    runs-in: c\n    steps: [{run: x, outputs: [y]}]\n", "container steps cannot set"),
        ] {
            let err = format!("{:#}", Pipeline::parse(yaml).unwrap_err());
            assert!(err.contains(expected), "{yaml}: {err}");
        }
    }
}
