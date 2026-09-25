//! `github-release`: create the draft GitHub release (unless it exists) and
//! upload assets to it.

use anyhow::{Context, Result};
use serde::Deserialize;

use super::params::{params, Flag, StringOrList};
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::services::github::GitHub;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    tag: String,
    title: String,
    /// File with the release notes.
    notes: String,
    #[serde(default)]
    prerelease: Flag,
    /// Files to upload; globs allowed.
    #[serde(default)]
    assets: Option<StringOrList>,
    /// The repository, e.g. `https://github.com/rcaelers/workrave.git`.
    repo: String,
    /// A token with access to the repository, e.g. `{{ secret('...') }}`.
    token: String,
}

pub struct GithubRelease;

impl Action for GithubRelease {
    fn name(&self) -> &'static str {
        "github-release"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            let notes = std::fs::read_to_string(&p.notes)
                .with_context(|| format!("reading release notes {}", p.notes))?;
            let assets = match &p.assets {
                Some(a) => a.paths()?,
                None => Vec::new(),
            };
            let repo = &p.repo;
            if env.dry_run {
                println!(
                    "DRYRUN: create draft GitHub release {} (title {}, prerelease {}) in {repo}",
                    p.tag,
                    p.title,
                    p.prerelease.value()
                );
                for asset in &assets {
                    println!("DRYRUN: upload {} to release {}", asset.display(), p.tag);
                }
                return Ok(Outcome::default());
            }
            let github = GitHub::new(repo, &p.token)?;
            let release = match github.find_release(&p.tag).await? {
                Some(existing) => {
                    tracing::info!(
                        "GitHub release {} already exists ({})",
                        p.tag,
                        existing.html_url
                    );
                    existing
                }
                None => {
                    github
                        .create_draft_release(&p.tag, &p.title, &notes, p.prerelease.value())
                        .await?
                }
            };
            for asset in &assets {
                github.upload_asset(&release, asset).await?;
            }
            Ok(Outcome::default())
        })
    }
}
