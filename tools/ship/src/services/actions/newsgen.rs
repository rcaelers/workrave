//! `newsgen`: release notes from changes.yaml in one of the templates
//! (github, blog, debian-changelog, NEWS, appcast).

use std::path::PathBuf;

use anyhow::{Context, Result};
use serde::Deserialize;

use super::params::{params, Flag};
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::news::newsgen;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// The changes.yaml to render.
    input: String,
    template: String,
    output: String,
    /// Generate for this release (else the latest).
    release: Option<String>,
    /// Only that release, not everything since.
    #[serde(default)]
    single: Flag,
    #[serde(default)]
    latest: Flag,
    /// debian-changelog: Ubuntu series and PPA increment.
    #[serde(default)]
    series: String,
    #[serde(default)]
    increment: String,
}

pub struct Newsgen;

impl Action for Newsgen {
    fn name(&self) -> &'static str {
        "newsgen"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, _env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            let input = PathBuf::from(&p.input);
            let output = PathBuf::from(&p.output);
            if let Some(parent) = output.parent() {
                std::fs::create_dir_all(parent)?;
            }
            let increment: i64 = if p.increment.is_empty() {
                0
            } else {
                p.increment
                    .parse()
                    .with_context(|| format!("newsgen increment '{}'", p.increment))?
            };
            newsgen::run_newsgen(
                &input,
                &output,
                p.template,
                p.release.filter(|r| !r.is_empty()),
                p.single.value(),
                p.latest.value(),
                p.series,
                increment,
            )
            .await?;
            tracing::info!("Wrote {}", output.display());
            Ok(Outcome::default())
        })
    }
}
