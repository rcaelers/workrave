//! The machine configuration (`--config`, `$SHIP_CONFIG`, or
//! `~/.config/ship/ship.yaml`).
//!
//! A free-form YAML mapping; `ship` itself does not define its keys. The
//! release pipeline refers to them as `{{ config.<key> }}`,
//! so what a configuration must contain is defined there. The only
//! conventional key is `signing_service_url`, which `ship sign` and
//! `ship secret` use when no `--url` is given.
//!
//! `profiles.<name>` holds overrides that are deep-merged over the rest when
//! `--profile <name>` is given (e.g. to switch from a remote podman to a
//! local docker). A leading `~` in string values is expanded.

use std::path::{Path, PathBuf};

use anyhow::{anyhow, bail, Context, Result};
use serde_yaml::Value;

#[derive(Debug, Clone)]
pub struct Config {
    value: Value,
}

impl Config {
    /// The default configuration file location.
    pub fn default_path() -> Option<PathBuf> {
        if let Some(dir) = std::env::var_os("XDG_CONFIG_HOME") {
            return Some(PathBuf::from(dir).join("ship").join("ship.yaml"));
        }
        home_dir().map(|home| home.join(".config").join("ship").join("ship.yaml"))
    }

    /// Loads the configuration from `path` (or the default location) and
    /// applies `profile`, if any.
    pub fn load(path: Option<&Path>, profile: Option<&str>) -> Result<Config> {
        let path = match path {
            Some(p) => p.to_path_buf(),
            None => Config::default_path().ok_or_else(|| {
                anyhow!("cannot determine the home directory for the config file")
            })?,
        };
        let raw = std::fs::read_to_string(&path)
            .with_context(|| format!("reading config file {}", path.display()))?;
        tracing::info!("Using config file {}", path.display());
        Config::parse(&raw, profile).with_context(|| format!("in config file {}", path.display()))
    }

    pub fn parse(raw: &str, profile: Option<&str>) -> Result<Config> {
        let mut value: Value = serde_yaml::from_str(raw).context("parsing YAML")?;
        if value.is_null() {
            value = Value::Mapping(Default::default());
        }
        let Value::Mapping(mut map) = value else {
            bail!("the config file must be a mapping");
        };

        let profiles = map.remove("profiles");
        if let Some(name) = profile {
            let overrides = profiles
                .as_ref()
                .and_then(|p| p.get(name))
                .cloned()
                .ok_or_else(|| anyhow!("profile '{name}' not found"))?;
            let mut base = Value::Mapping(map);
            merge(&mut base, overrides);
            let Value::Mapping(merged) = base else {
                unreachable!()
            };
            map = merged;
        }

        let mut value = Value::Mapping(map);
        expand_tilde(&mut value, home_dir().as_deref());
        Ok(Config { value })
    }

    /// The whole configuration, for the template context.
    pub fn value(&self) -> &Value {
        &self.value
    }

    /// A string value by dotted key, e.g. `container.engine`.
    pub fn get_str(&self, key: &str) -> Option<&str> {
        let mut node = &self.value;
        for part in key.split('.') {
            node = node.get(part)?;
        }
        node.as_str()
    }

    pub fn signing_service_url(&self) -> Result<&str> {
        self.get_str("signing_service_url")
            .filter(|s| !s.is_empty())
            .ok_or_else(|| anyhow!("signing_service_url is not set in the config file"))
    }
}

fn home_dir() -> Option<PathBuf> {
    std::env::var_os("HOME")
        .or_else(|| std::env::var_os("USERPROFILE"))
        .map(PathBuf::from)
}

/// Deep-merges `overrides` into `base`: mappings are merged key by key,
/// everything else is replaced.
fn merge(base: &mut Value, overrides: Value) {
    match (base, overrides) {
        (Value::Mapping(base), Value::Mapping(overrides)) => {
            for (key, value) in overrides {
                match base.get_mut(&key) {
                    Some(existing) => merge(existing, value),
                    None => {
                        base.insert(key, value);
                    }
                }
            }
        }
        (base, overrides) => *base = overrides,
    }
}

/// Expands a leading `~` in every string value, so paths can be written as
/// `~/src/workrave`.
fn expand_tilde(value: &mut Value, home: Option<&Path>) {
    match value {
        Value::String(s) => {
            if let Some(home) = home {
                if s == "~" {
                    *s = home.to_string_lossy().into_owned();
                } else if let Some(rest) = s.strip_prefix("~/") {
                    *s = home.join(rest).to_string_lossy().into_owned();
                }
            }
        }
        Value::Mapping(map) => {
            for (_, v) in map.iter_mut() {
                expand_tilde(v, home);
            }
        }
        Value::Sequence(seq) => {
            for v in seq {
                expand_tilde(v, home);
            }
        }
        _ => {}
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const SAMPLE: &str = r#"
signing_service_url: https://studio.home.krandor.org:50051
workspace_dir: ~/tmp/builds/workrave-release
container:
  engine: podman
  remote_dir: .cache/workrave-build
linux:
  ppa_series: [stonking, resolute]
profiles:
  docker:
    container: { engine: docker }
    signing_service_url: https://studio.local:50051
"#;

    #[test]
    fn parses_and_looks_up_keys() {
        let config = Config::parse(SAMPLE, None).unwrap();
        assert_eq!(config.get_str("container.engine"), Some("podman"));
        assert_eq!(
            config.signing_service_url().unwrap(),
            "https://studio.home.krandor.org:50051"
        );
        assert_eq!(config.get_str("nope.x"), None);
        assert_eq!(config.get_str("linux.ppa_series"), None);
        assert!(config.value().get("profiles").is_none());
    }

    #[test]
    fn profile_overrides_are_deep_merged() {
        let config = Config::parse(SAMPLE, Some("docker")).unwrap();
        assert_eq!(config.get_str("container.engine"), Some("docker"));
        // Untouched by the profile: still the base value.
        assert_eq!(
            config.get_str("container.remote_dir"),
            Some(".cache/workrave-build")
        );
        assert_eq!(
            config.signing_service_url().unwrap(),
            "https://studio.local:50051"
        );
    }

    #[test]
    fn unknown_profile_is_an_error() {
        let err = Config::parse(SAMPLE, Some("nope")).unwrap_err();
        assert!(err.to_string().contains("profile 'nope' not found"));
    }

    #[test]
    fn tilde_is_expanded() {
        let config = Config::parse(SAMPLE, None).unwrap();
        let home = home_dir().unwrap();
        assert_eq!(
            config.get_str("workspace_dir"),
            Some(home.join("tmp/builds/workrave-release").to_str().unwrap())
        );
    }

    #[test]
    fn empty_file_is_empty_config() {
        let config = Config::parse("", None).unwrap();
        assert!(config.signing_service_url().is_err());
        assert!(Config::parse("- not a mapping\n", None).is_err());
    }
}
