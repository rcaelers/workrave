//! Portable access to the same operations used by legacy workflow `uses` steps.
use std::io::Read;
use std::path::PathBuf;

use anyhow::{Context, Result};
use clap::Args;

use super::ConfigArgs;
use crate::services::actions::{self, ActionEnv};

#[derive(Args, Debug)]
pub struct ActionCommand {
    /// Operation (newsgen, sign, github-release, s3-upload, catalog, appcast,
    /// check-config, check-no-homebrew-links)
    name: String,
    /// YAML or JSON parameter file; - reads stdin
    #[arg(long, conflicts_with = "params_env")]
    params: Option<PathBuf>,
    /// Environment variable containing YAML or JSON parameters
    #[arg(long, conflicts_with = "params")]
    params_env: Option<String>,
    /// Build local outputs, but only describe signing and publishing
    #[arg(long)]
    dry_run: bool,
    /// Signing service URL, required by sign outside a dry run
    #[arg(long, env = "SIGNING_SERVICE_URL")]
    url: Option<String>,
    /// Only check-config reads the configuration file
    #[command(flatten)]
    config: ConfigArgs,
}

pub async fn run(args: ActionCommand) -> Result<()> {
    actions::find(&args.name)?;
    let text = match (args.params, args.params_env) {
        (_, Some(name)) => std::env::var(&name).with_context(|| format!("reading {name}"))?,
        (Some(path), _) if path.as_os_str() == "-" => {
            let mut text = String::new();
            std::io::stdin().read_to_string(&mut text)?;
            text
        }
        (Some(path), _) => std::fs::read_to_string(&path)
            .with_context(|| format!("reading parameters {}", path.display()))?,
        _ => "{}".into(),
    };
    let params = serde_yaml::from_str(&text).context("parsing action parameters")?;
    actions::validate(&args.name, &params)?;
    let config = if args.name == "check-config" {
        serde_json::to_value(args.config.load()?.value())?
    } else {
        serde_json::Value::Null
    };
    actions::run(
        &args.name,
        &params,
        &ActionEnv {
            dry_run: args.dry_run,
            signing_service_url: args.url.as_deref(),
            config: &config,
        },
    )
    .await?;
    Ok(())
}
