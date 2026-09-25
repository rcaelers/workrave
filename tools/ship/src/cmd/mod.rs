//! The top-level commands, one directory each. Commands share only what is
//! in this file and the library modules of the crate; a command never uses
//! a sibling command.

pub mod appcast;
pub mod catalog;
pub mod newsgen;
pub mod release;
pub mod secret;
pub mod sign;

use std::path::PathBuf;

use anyhow::Result;
use clap::Subcommand;

use crate::config::Config;
use crate::services::signing::SigningService;

#[derive(Subcommand, Debug)]
pub enum Command {
    /// generate appcast
    Appcast(appcast::AppcastCommand),
    /// update artifacts catalog in S3 storage
    Catalog(catalog::CatalogCommand),
    /// generate release notes in different formats
    Newsgen(newsgen::NewsgenCommand),
    /// build and publish a release (runs the release pipeline)
    Release(release::ReleaseCommand),
    /// inspect the release pipeline
    Pipeline(release::PipelineCommand),
    /// sign files with the signing service
    Sign(sign::SignCommand),
    /// fetch a secret from the signing service
    Secret(secret::SecretCommand),
}

pub async fn run(command: Command) -> Result<()> {
    match command {
        Command::Appcast(args) => appcast::run(args).await,
        Command::Catalog(args) => catalog::run(args).await,
        Command::Newsgen(args) => newsgen::run(args).await,
        Command::Release(args) => release::run(args).await,
        Command::Pipeline(args) => release::run_pipeline(args).await,
        Command::Sign(args) => sign::run(args).await,
        Command::Secret(args) => secret::run(args).await,
    }
}

/// Options selecting the configuration file and profile.
#[derive(clap::Args, Debug, Clone)]
pub struct ConfigArgs {
    /// Configuration file (default: $SHIP_CONFIG, else ~/.config/ship/ship.yaml)
    #[arg(short = 'f', long, global = true, env = "SHIP_CONFIG")]
    config: Option<PathBuf>,
    /// Profile from the configuration file to apply
    #[arg(short = 'p', long, global = true, env = "SHIP_PROFILE")]
    profile: Option<String>,
}

impl ConfigArgs {
    pub fn load(&self) -> Result<Config> {
        Config::load(self.config.as_deref(), self.profile.as_deref())
    }
}

/// Options for commands that only need the signing service. The URL is taken
/// from --url, else SIGNING_SERVICE_URL, else the configuration file, so that
/// these commands also work as hooks inside builds that received the URL
/// through the environment.
#[derive(clap::Args, Debug, Clone)]
pub struct SigningArgs {
    #[command(flatten)]
    config: ConfigArgs,
    /// Signing service URL (default: SIGNING_SERVICE_URL, else the config file)
    #[arg(long, env = "SIGNING_SERVICE_URL")]
    url: Option<String>,
}

impl SigningArgs {
    pub fn service(&self) -> Result<SigningService> {
        let url = match &self.url {
            Some(url) => url.clone(),
            None => self.config.load()?.signing_service_url()?.to_string(),
        };
        SigningService::new(&url)
    }
}

/// The CI commands (catalog, newsgen) report errors but exit successfully,
/// as the TypeScript tool they replaced did.
pub fn log_and_succeed(result: Result<()>) -> Result<()> {
    if let Err(error) = result {
        eprintln!("{error:?}");
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use anyhow::anyhow;

    #[test]
    fn log_and_succeed_preserves_success() {
        assert!(log_and_succeed(Ok(())).is_ok());
    }

    #[test]
    fn log_and_succeed_swallows_errors_like_typescript_commands() {
        assert!(log_and_succeed(Err(anyhow!("expected failure"))).is_ok());
    }
}
