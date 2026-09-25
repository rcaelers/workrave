//! Rendering of the pipeline's Jinja templates.
//!
//! Every string in the pipeline file is rendered against [`Vars`] when the
//! step runs: the user configuration, the version of the checkout, the
//! command line choices, the matrix values, the outputs of earlier steps, and a
//! few helper functions (`secret`, `exists`, `glob`, `today`) and filters
//! (`msys`).

use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

use anyhow::{anyhow, bail, Context, Result};
use minijinja::value::Value;
use minijinja::{Environment, Error, ErrorKind, UndefinedBehavior};
use serde::Serialize;

use crate::services::signing::SigningService;
use crate::system::process;
#[derive(Debug, Clone, Serialize)]
pub struct HostVars {
    /// `linux`, `macos` or `windows`
    pub os: String,
}

#[derive(Debug, Clone, Serialize)]
pub struct ShipVars {
    /// Path of the running ship binary, for build hooks.
    pub exe: PathBuf,
}

/// Everything templates can refer to.
#[derive(Debug, Clone, Serialize)]
pub struct Vars {
    pub config: serde_json::Value,
    /// The pipeline's options as given on the command line (or their
    /// defaults), plus `--set` values.
    pub options: serde_json::Value,
    /// `--dry-run`: builds happen, uploads and signing are only printed.
    pub dry_run: bool,
    pub host: HostVars,
    pub ship: ShipVars,
    pub env: HashMap<String, String>,
    /// The pipeline's top-level `vars:`, rendered (`true`/`false` become
    /// booleans, see [`parse_output_value`]).
    pub vars: HashMap<String, serde_json::Value>,
    /// Vars that could not be rendered yet (e.g. they need the version
    /// before the step that sets it ran), with the reason.
    #[serde(skip)]
    pub vars_errors: HashMap<String, String>,
    pub matrix: HashMap<String, serde_json::Value>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub item: Option<serde_json::Value>,
    /// Values set by steps (`outputs:`), merged into the top level of the
    /// context: an output `version.tag` is `{{ version.tag }}`.
    #[serde(skip)]
    pub outputs: serde_json::Map<String, serde_json::Value>,
}

/// Names a step output may not start with: they are the fixed context.
const RESERVED: [&str; 10] = [
    "config", "options", "dry_run", "host", "ship", "env", "vars", "matrix", "item", "secret",
];

impl Vars {
    /// The context as the templates see it: the fixed values plus the
    /// outputs, nested by their dotted names.
    pub fn template_value(&self) -> serde_json::Value {
        let mut root = serde_json::to_value(self).unwrap_or_default();
        for (key, value) in &self.outputs {
            insert_dotted(&mut root, key, value.clone());
        }
        root
    }

    /// Records a step output, e.g. `version.tag`.
    pub fn set_output(&mut self, key: &str, value: serde_json::Value) -> Result<()> {
        let head = key.split('.').next().unwrap_or_default();
        if key.is_empty() || RESERVED.contains(&head) {
            bail!(
                "'{key}' is not a valid output name (reserved: {})",
                RESERVED.join(", ")
            );
        }
        if !key.split('.').all(|part| {
            !part.is_empty() && part.chars().all(|c| c.is_ascii_alphanumeric() || c == '_')
        }) {
            bail!("'{key}' is not a valid output name (letters, digits, _ and . only)");
        }
        self.outputs.insert(key.to_string(), value);
        Ok(())
    }

    pub fn with_matrix(&self, matrix: HashMap<String, serde_json::Value>) -> Vars {
        Vars {
            matrix,
            ..self.clone()
        }
    }

    pub fn with_item(&self, item: serde_json::Value) -> Vars {
        Vars {
            item: Some(item),
            ..self.clone()
        }
    }
}

/// How `secret()` behaves.
#[derive(Clone)]
pub enum Secrets {
    /// Fetch from the signing service (cached).
    Fetch(SigningService),
    /// Dry run: empty values, nothing fetched.
    DryRun,
    /// `pipeline show`: a placeholder naming the secret.
    Placeholder,
    /// No signing service configured (`settings.signing_service_url`).
    Unavailable,
}

pub struct Renderer {
    env: Environment<'static>,
}

impl Renderer {
    pub fn new(secrets: Secrets) -> Renderer {
        let mut env = Environment::new();
        // Undefined variables render empty, but attribute access on an
        // undefined value is an error, so a typo like `verison.tag` fails.
        env.set_undefined_behavior(UndefinedBehavior::Lenient);
        env.set_keep_trailing_newline(true);

        let cache: Arc<Mutex<HashMap<String, String>>> = Arc::new(Mutex::new(HashMap::new()));
        env.add_function("secret", move |name: String| -> Result<String, Error> {
            match &secrets {
                Secrets::Placeholder => Ok(format!("<secret:{name}>")),
                Secrets::Unavailable => Err(Error::new(
                    ErrorKind::InvalidOperation,
                    format!(
                        "secret('{name}') needs settings.signing_service_url in the pipeline file"
                    ),
                )),
                Secrets::DryRun => {
                    tracing::debug!("DRYRUN: not fetching secret {name}");
                    Ok(String::new())
                }
                Secrets::Fetch(service) => {
                    if let Some(value) = cache.lock().unwrap().get(&name) {
                        return Ok(value.clone());
                    }
                    let service = service.clone();
                    let fetched = tokio::task::block_in_place(|| {
                        tokio::runtime::Handle::current().block_on(service.secret(&name))
                    })
                    .map_err(|e| Error::new(ErrorKind::InvalidOperation, format!("{e:#}")))?;
                    process::redact(&fetched);
                    cache.lock().unwrap().insert(name, fetched.clone());
                    Ok(fetched)
                }
            }
        });
        env.add_function("exists", |path: String| Path::new(&path).exists());
        env.add_function("glob", |pattern: String| -> Result<Vec<String>, Error> {
            glob_paths(&pattern)
                .map_err(|e| Error::new(ErrorKind::InvalidOperation, format!("{e:#}")))
        });
        env.add_function("today", |format: Option<String>| {
            chrono::Local::now()
                .format(format.as_deref().unwrap_or("%Y-%m-%d"))
                .to_string()
        });
        env.add_filter("msys", |path: String| msys_path(&path));

        Renderer { env }
    }

    pub fn render(&self, template: &str, vars: &Vars) -> Result<String> {
        self.render_value(template, &vars.template_value(), vars)
    }

    fn render_value(
        &self,
        template: &str,
        value: &serde_json::Value,
        vars: &Vars,
    ) -> Result<String> {
        // A `vars.x` that is not available yet would silently render empty.
        for name in referenced_vars(template) {
            if !vars.vars.contains_key(&name) {
                match vars.vars_errors.get(&name) {
                    Some(reason) => bail!("vars.{name} cannot be computed yet: {reason}"),
                    None => bail!("unknown vars.{name} in `{template}`"),
                }
            }
        }
        self.env
            .render_str(template, Value::from_serialize(value))
            .with_context(|| format!("rendering `{template}`"))
    }

    /// Renders a template and interprets the result as a condition: empty,
    /// `false`, `0` and `none` are false.
    pub fn condition(&self, template: &str, vars: &Vars) -> Result<bool> {
        Ok(truthy(&self.render(template, vars)?))
    }

    /// Renders a template that should produce a list (a YAML/JSON array or
    /// whitespace separated words).
    pub fn render_list(&self, template: &str, vars: &Vars) -> Result<Vec<serde_json::Value>> {
        let rendered = self.render(template, vars)?;
        let trimmed = rendered.trim();
        if trimmed.starts_with('[') {
            let items: Vec<serde_json::Value> = serde_json::from_str(trimmed)
                .or_else(|_| serde_yaml::from_str(trimmed))
                .with_context(|| format!("`{template}` did not render to a list: {trimmed}"))?;
            Ok(items)
        } else {
            Ok(trimmed
                .split_whitespace()
                .map(|s| serde_json::Value::String(s.to_string()))
                .collect())
        }
    }

    /// Renders every string inside a YAML value (for `with:` parameters).
    pub fn render_yaml(&self, value: &serde_yaml::Value, vars: &Vars) -> Result<serde_yaml::Value> {
        Ok(match value {
            serde_yaml::Value::String(s) => serde_yaml::Value::String(self.render(s, vars)?),
            serde_yaml::Value::Sequence(items) => serde_yaml::Value::Sequence(
                items
                    .iter()
                    .map(|v| self.render_yaml(v, vars))
                    .collect::<Result<_>>()?,
            ),
            serde_yaml::Value::Mapping(map) => {
                let mut out = serde_yaml::Mapping::new();
                for (k, v) in map {
                    out.insert(self.render_yaml(k, vars)?, self.render_yaml(v, vars)?);
                }
                serde_yaml::Value::Mapping(out)
            }
            other => other.clone(),
        })
    }
}

/// Sets `a.b.c` in a JSON object tree, creating objects on the way.
pub fn insert_dotted(root: &mut serde_json::Value, key: &str, value: serde_json::Value) {
    let mut node = root;
    let parts: Vec<&str> = key.split('.').collect();
    for part in &parts[..parts.len() - 1] {
        if !node.is_object() {
            *node = serde_json::Value::Object(Default::default());
        }
        node = node
            .as_object_mut()
            .unwrap()
            .entry(*part)
            .or_insert_with(|| serde_json::Value::Object(Default::default()));
    }
    if !node.is_object() {
        *node = serde_json::Value::Object(Default::default());
    }
    node.as_object_mut()
        .unwrap()
        .insert(parts[parts.len() - 1].to_string(), value);
}

/// An output value as written by a step: `true`/`false` become booleans
/// (so they work in `if:`), everything else stays a string.
pub fn parse_output_value(raw: &str) -> serde_json::Value {
    match raw.trim() {
        "true" | "True" => serde_json::Value::Bool(true),
        "false" | "False" => serde_json::Value::Bool(false),
        other => serde_json::Value::String(other.to_string()),
    }
}

/// Parses `name=value` lines written to the outputs file of a step.
pub fn parse_outputs(text: &str) -> Result<Vec<(String, serde_json::Value)>> {
    let mut outputs = Vec::new();
    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let Some((key, value)) = line.split_once('=') else {
            bail!("output line is not name=value: `{line}`");
        };
        outputs.push((key.trim().to_string(), parse_output_value(value)));
    }
    Ok(outputs)
}

/// The `vars.<name>` references in a template.
fn referenced_vars(template: &str) -> Vec<String> {
    static RE: std::sync::OnceLock<regex::Regex> = std::sync::OnceLock::new();
    let re = RE.get_or_init(|| regex::Regex::new(r"\bvars\.([A-Za-z_][A-Za-z0-9_]*)").unwrap());
    re.captures_iter(template)
        .map(|c| c[1].to_string())
        .collect()
}

pub fn truthy(rendered: &str) -> bool {
    !matches!(
        rendered.trim().to_ascii_lowercase().as_str(),
        "" | "false" | "0" | "none" | "null" | "no"
    )
}

pub fn glob_paths(pattern: &str) -> Result<Vec<String>> {
    let mut paths: Vec<String> = glob::glob(pattern)
        .with_context(|| format!("invalid glob pattern {pattern}"))?
        .filter_map(|p| p.ok())
        .map(|p| p.to_string_lossy().into_owned())
        .collect();
    paths.sort();
    Ok(paths)
}

/// `C:\Users\robc\src` -> `/c/Users/robc/src`; other paths only get forward
/// slashes. MSYS2 programs expect this form.
pub fn msys_path(path: &str) -> String {
    let s = path.replace('\\', "/");
    let bytes = s.as_bytes();
    if bytes.len() >= 2 && bytes[1] == b':' && bytes[0].is_ascii_alphabetic() {
        let drive = (bytes[0] as char).to_ascii_lowercase();
        format!("/{drive}{}", &s[2..])
    } else {
        s
    }
}

/// The user configuration as template data. Null values are dropped so that
/// an unset optional key renders as an empty string instead of `none`.
pub fn config_value(config: &serde_yaml::Value) -> Result<serde_json::Value> {
    fn strip_nulls(value: serde_json::Value) -> serde_json::Value {
        match value {
            serde_json::Value::Object(map) => serde_json::Value::Object(
                map.into_iter()
                    .filter(|(_, v)| !v.is_null())
                    .map(|(k, v)| (k, strip_nulls(v)))
                    .collect(),
            ),
            serde_json::Value::Array(items) => {
                serde_json::Value::Array(items.into_iter().map(strip_nulls).collect())
            }
            other => other,
        }
    }
    let json: serde_json::Value =
        serde_json::to_value(config).map_err(|e| anyhow!("converting config: {e}"))?;
    Ok(strip_nulls(json))
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    pub fn sample_vars() -> Vars {
        let mut vars = Vars {
            config: serde_json::json!({
                "signing_service_url": "https://sign:1",
                "workspace_dir": "/w",
                "linux": { "ppa_series": ["stonking", "resolute"], "build_deb": false },
                "container": { "image_repository": "ghcr.io/x/build" }
            }),
            options: serde_json::json!({
                "commit": "abc", "version": "v1_12_0_alpha_1", "prerelease": false, "staging": false, "ppa": ""
            }),
            dry_run: true,
            host: HostVars {
                os: "macos".to_string(),
            },
            ship: ShipVars {
                exe: PathBuf::from("/bin/ship"),
            },
            env: HashMap::from([("PATH".to_string(), "/bin".to_string())]),
            vars: HashMap::from([
                ("deploy".to_string(), serde_json::json!("/w/deploy")),
                ("ppa_increment".to_string(), serde_json::json!("1")),
                ("prerelease".to_string(), serde_json::json!(false)),
            ]),
            vars_errors: HashMap::new(),
            matrix: HashMap::new(),
            item: None,
            outputs: Default::default(),
        };
        vars.set_output("version.tag", serde_json::json!("v1_12_0_alpha_1"))
            .unwrap();
        vars.set_output("version.workrave", serde_json::json!("1.12.0-alpha.1"))
            .unwrap();
        vars
    }

    #[test]
    fn renders_context_values() {
        let r = Renderer::new(Secrets::Placeholder);
        let v = sample_vars();
        assert_eq!(
            r.render("{{ vars.deploy }}/{{ version.tag }}", &v).unwrap(),
            "/w/deploy/v1_12_0_alpha_1"
        );
        assert_eq!(
            r.render("{{ config.linux.ppa_series | join(' ') }}", &v)
                .unwrap(),
            "stonking resolute"
        );
        assert_eq!(
            r.render(
                "ppa.sh -p {{ vars.ppa_increment }} {{ '-d' if dry_run }} {{ '-P' if options.prerelease }}",
                &v
            )
            .unwrap(),
            "ppa.sh -p 1 -d "
        );
        assert_eq!(
            r.render("{{ secret('secrets.x') }}", &v).unwrap(),
            "<secret:secrets.x>"
        );
        assert_eq!(r.render("{{ env.PATH }}", &v).unwrap(), "/bin");
        assert_eq!(r.render("{{ config.linux.debian_dir }}", &v).unwrap(), "");
    }

    #[test]
    fn outputs_are_nested_and_validated() {
        let mut v = sample_vars();
        v.set_output("version.is_release", serde_json::json!(true))
            .unwrap();
        v.set_output("build.id", serde_json::json!("x")).unwrap();
        let r = Renderer::new(Secrets::Placeholder);
        assert_eq!(
            r.render(
                "{{ version.tag }} {{ build.id }} {{ 'rel' if version.is_release }}",
                &v
            )
            .unwrap(),
            "v1_12_0_alpha_1 x rel"
        );
        assert!(v.set_output("config.x", serde_json::json!(1)).is_err());
        assert!(v.set_output("bad-name", serde_json::json!(1)).is_err());
        assert!(v.set_output("", serde_json::json!(1)).is_err());

        let parsed =
            parse_outputs("version.tag=v1\n# comment\n\nversion.is_release=true\n").unwrap();
        assert_eq!(
            parsed[0],
            ("version.tag".to_string(), serde_json::json!("v1"))
        );
        assert_eq!(
            parsed[1],
            ("version.is_release".to_string(), serde_json::json!(true))
        );
        assert!(parse_outputs("no equals sign").is_err());
    }

    #[test]
    fn typos_are_errors() {
        let r = Renderer::new(Secrets::Placeholder);
        assert!(r.render("{{ verison.tag }}", &sample_vars()).is_err());
    }

    #[test]
    fn unavailable_vars_are_errors() {
        let r = Renderer::new(Secrets::Placeholder);
        let mut v = sample_vars();
        let err = r
            .render("mkdir {{ vars.release_dir }}", &v)
            .unwrap_err()
            .to_string();
        assert!(err.contains("unknown vars.release_dir"), "{err}");
        v.vars_errors
            .insert("release_dir".into(), "version unknown".into());
        let err = r
            .render("{{ vars.release_dir }}", &v)
            .unwrap_err()
            .to_string();
        assert!(
            err.contains("cannot be computed yet: version unknown"),
            "{err}"
        );
        v.vars
            .insert("release_dir".into(), serde_json::json!("/d/v1"));
        assert_eq!(r.render("{{ vars.release_dir }}", &v).unwrap(), "/d/v1");
        assert_eq!(r.render("{{ 'x' if vars.prerelease }}", &v).unwrap(), "");
    }

    #[test]
    fn conditions() {
        let r = Renderer::new(Secrets::Placeholder);
        let v = sample_vars();
        assert!(r.condition("{{ host.os == 'macos' }}", &v).unwrap());
        assert!(!r.condition("{{ config.linux.build_deb }}", &v).unwrap());
        assert!(!r.condition("{{ options.prerelease }}", &v).unwrap());
        assert!(r.condition("{{ options.ppa or 3 }}", &v).unwrap());
        assert_eq!(r.render("{{ options.commit }}", &v).unwrap(), "abc");
        assert!(r.condition("{{ dry_run }}", &v).unwrap());
        // minijinja prints booleans Python-style.
        assert_eq!(r.render("{{ dry_run }}", &v).unwrap(), "True");
        assert!(truthy("True"));
        assert!(!truthy("False"));
        assert!(!r.condition("{{ config.linux.debian_dir }}", &v).unwrap());
        assert!(r.condition("yes", &v).unwrap());
    }

    #[test]
    fn lists_and_yaml() {
        let r = Renderer::new(Secrets::Placeholder);
        let v = sample_vars();
        let list = r.render_list("{{ config.linux.ppa_series }}", &v).unwrap();
        assert_eq!(
            list,
            vec![serde_json::json!("stonking"), serde_json::json!("resolute")]
        );
        let words = r.render_list("a b  c", &v).unwrap();
        assert_eq!(words.len(), 3);

        let with: serde_yaml::Value = serde_yaml::from_str(
            "{ output: \"{{ vars.deploy }}/x\", flags: [\"{{ version.tag }}\"], n: 1 }",
        )
        .unwrap();
        let rendered = r.render_yaml(&with, &v).unwrap();
        assert_eq!(
            rendered["output"],
            serde_yaml::Value::String("/w/deploy/x".into())
        );
        assert_eq!(
            rendered["flags"][0],
            serde_yaml::Value::String("v1_12_0_alpha_1".into())
        );
        assert_eq!(rendered["n"], serde_yaml::Value::Number(1.into()));
    }

    #[test]
    fn msys_filter_and_config_nulls() {
        assert_eq!(msys_path(r"C:\Users\robc\src"), "/c/Users/robc/src");
        assert_eq!(msys_path("/c/already"), "/c/already");
        let config: serde_yaml::Value =
            serde_yaml::from_str("a: 1\nb: null\nc: {d: null, e: x}\n").unwrap();
        let value = config_value(&config).unwrap();
        assert_eq!(value, serde_json::json!({"a": 1, "c": {"e": "x"}}));
        let r = Renderer::new(Secrets::Placeholder);
        assert_eq!(
            r.render("{{ 'C:\\\\x\\\\y' | msys }}", &sample_vars())
                .unwrap(),
            "/c/x/y"
        );
    }
}
