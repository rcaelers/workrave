//! The builtin actions a step can `use`: `- uses: <name>` with `with:`
//! parameters. One file per action; each implements [`Action`] and is
//! registered in [`ALL`]. Actions share nothing but [`ActionEnv`] and the
//! parameter helpers in `params.rs`.

mod appcast;
mod catalog;
mod check_config;
mod check_no_homebrew_links;
mod github_release;
mod newsgen;
mod params;
mod s3_upload;
mod sign;

use std::future::Future;
use std::pin::Pin;

use anyhow::{bail, Result};

use crate::services::signing::SigningService;

/// What the runner needs to know after an action ran.
#[derive(Debug, Default)]
pub struct Outcome {
    /// Values for later steps, like a `run:` step's `outputs`.
    pub outputs: Vec<(String, serde_json::Value)>,
}

pub struct ActionEnv<'a> {
    pub dry_run: bool,
    /// `settings.signing_service_url`, for the `sign` action.
    pub signing_service_url: Option<&'a str>,
    /// The machine configuration, for `check-config`.
    pub config: &'a serde_json::Value,
}

impl ActionEnv<'_> {
    pub fn signing(&self) -> Result<SigningService> {
        match self.signing_service_url {
            Some(url) => SigningService::new(url),
            None => bail!("settings.signing_service_url is not set in the pipeline file"),
        }
    }
}

pub type ActionFuture<'a> = Pin<Box<dyn Future<Output = Result<Outcome>> + Send + 'a>>;

pub trait Action: Sync {
    /// The name used in `uses:`.
    fn name(&self) -> &'static str;
    /// Checks the shape of the (still unrendered) `with:` parameters when the
    /// pipeline is loaded, so mistakes fail before anything runs.
    fn validate(&self, with: &serde_yaml::Value) -> Result<()>;
    /// Runs the action with the rendered parameters.
    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a>;
    /// Whether `pipeline show` runs the action too (only for checks without
    /// side effects).
    fn runs_in_show(&self) -> bool {
        false
    }
}

pub static ALL: &[&dyn Action] = &[
    &newsgen::Newsgen,
    &sign::Sign,
    &github_release::GithubRelease,
    &s3_upload::S3Upload,
    &catalog::Catalog,
    &appcast::Appcast,
    &check_config::CheckConfig,
    &check_no_homebrew_links::CheckNoHomebrewLinks,
];

pub fn find(name: &str) -> Result<&'static dyn Action> {
    ALL.iter()
        .copied()
        .find(|a| a.name() == name)
        .ok_or_else(|| {
            let known: Vec<&str> = ALL.iter().map(|a| a.name()).collect();
            anyhow::anyhow!("unknown action '{name}' (known: {})", known.join(", "))
        })
}

pub fn validate(name: &str, with: &serde_yaml::Value) -> Result<()> {
    find(name)?.validate(with)
}

pub async fn run(name: &str, with: &serde_yaml::Value, env: &ActionEnv<'_>) -> Result<Outcome> {
    let action = find(name)?;
    if with.is_null() || with.is_mapping() {
        action.run(with, env).await
    } else {
        bail!("`with` of action '{name}' must be a mapping");
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn registry_names_are_unique() {
        let mut names: Vec<&str> = ALL.iter().map(|a| a.name()).collect();
        names.sort();
        names.dedup();
        assert_eq!(names.len(), ALL.len());
        assert!(find("newsgen").is_ok());
        assert!(find("nope").is_err());
    }
}
