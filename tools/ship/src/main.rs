//! ship: release pipeline runner. See README.md.
//!
//! `cmd/` holds the top-level commands, one directory each. The rest is the
//! library they share: `news` (release notes), `services` (signing service,
//! GitHub, the snapshots bucket with its catalog and appcast), `system`
//! (processes, containers, the source workspace) and `config`.

mod cmd;
mod config;
mod news;
mod services;
mod system;

use anyhow::Result;
use clap::Parser;
use tracing_subscriber::EnvFilter;

#[derive(Parser, Debug)]
#[command(name = "ship", version, about = "Release pipeline runner")]
struct Cli {
    #[command(subcommand)]
    command: cmd::Command,
}

#[tokio::main]
async fn main() -> Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info")),
        )
        .with_target(false)
        .with_writer(std::io::stderr)
        .init();

    system::process::install_interrupt_handler();
    cmd::run(Cli::parse().command).await
}
