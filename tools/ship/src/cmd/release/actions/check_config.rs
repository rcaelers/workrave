//! `check-config`: verify that the machine configuration has every key a
//! target needs, and report all missing ones at once.

use anyhow::{bail, Result};
use serde::Deserialize;

use super::params::{params, StringOrList};
use super::{Action, ActionEnv, ActionFuture, Outcome};

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// Dotted keys that must be present and not empty, e.g. `container.engine`.
    required: StringOrList,
    /// Keys that may be absent or empty; listed for documentation.
    #[serde(default)]
    optional: Option<StringOrList>,
    /// Where to look for the keys, e.g. an example configuration file.
    #[serde(default)]
    hint: String,
}

pub struct CheckConfig;

impl Action for CheckConfig {
    fn name(&self) -> &'static str {
        "check-config"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn runs_in_show(&self) -> bool {
        true
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            let missing = missing_keys(env.config, &p.required.items());
            if !missing.is_empty() {
                bail!(
                    "the configuration is missing: {}{}",
                    missing.join(", "),
                    if p.hint.is_empty() {
                        String::new()
                    } else {
                        format!(" (see {})", p.hint)
                    }
                );
            }
            tracing::info!(
                "Configuration complete ({} keys{})",
                p.required.items().len(),
                match &p.optional {
                    Some(o) => format!(", {} optional", o.items().len()),
                    None => String::new(),
                }
            );
            Ok(Outcome::default())
        })
    }
}

/// The keys of `required` that are absent, null, or an empty string/list.
pub fn missing_keys(config: &serde_json::Value, required: &[String]) -> Vec<String> {
    required
        .iter()
        .filter(|key| {
            let mut node = Some(config);
            for part in key.split('.') {
                node = node.and_then(|n| n.get(part));
            }
            match node {
                None | Some(serde_json::Value::Null) => true,
                Some(serde_json::Value::String(s)) => s.is_empty(),
                Some(serde_json::Value::Array(a)) => a.is_empty(),
                Some(_) => false,
            }
        })
        .cloned()
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reports_absent_null_and_empty_keys() {
        let config = serde_json::json!({
            "a": "x", "b": "", "c": null, "d": { "e": "y", "f": [] }, "g": false
        });
        let required: Vec<String> = ["a", "b", "c", "d.e", "d.f", "d.nope", "h", "g"]
            .iter()
            .map(|s| s.to_string())
            .collect();
        assert_eq!(
            missing_keys(&config, &required),
            vec!["b", "c", "d.f", "d.nope", "h"]
        );
    }
}
