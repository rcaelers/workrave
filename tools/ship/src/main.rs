mod appcast;
mod catalog;
mod markdown_text;
mod newsgen;
mod s3;
mod templates;

use std::path::PathBuf;

use anyhow::Result;
use clap::{Parser, Subcommand};
use tracing_subscriber::EnvFilter;

#[derive(Parser, Debug)]
#[command(name = "ship", version, about = "Workrave release tooling")]
struct Cli {
    #[command(subcommand)]
    command: Command,
}

#[derive(Subcommand, Debug)]
enum Command {
    /// generate appcast
    Appcast(AppcastArgs),
    /// update artifacts catalog in S3 storage
    Catalog(CatalogArgs),
    /// generate release notes in different formats
    Newsgen(NewsgenArgs),
}

#[derive(clap::Args, Debug)]
struct AppcastArgs {
    #[arg(short = 'b', long, default_value = "v1.11")]
    branch: String,
    #[arg(short = 'B', long, default_value = "snapshots")]
    bucket: String,
    #[arg(short = 'e', long, default_value = "")]
    environment: String,
    #[arg(short = 'k', long, default_value = "github")]
    key: String,
    #[arg(short = 's', long, env = "SNAPSHOTS_SECRET_ACCESS_KEY")]
    secret: String,
    #[arg(short = 'E', long, default_value = "https://snapshots.workrave.org/")]
    endpoint: String,
    #[arg(short = 'n', long, default_value = "appcast.xml")]
    name: String,
    /// output to file instead of S3 bucket
    #[arg(long, default_value_t = false)]
    file: bool,
    /// First release to generate
    #[arg(short = 'r', long)]
    release: Option<String>,
    /// Dry run. Result is not uploaded to storage
    #[arg(short = 'd', long, default_value_t = false)]
    dry: bool,
    /// YAML input file containing release notes
    #[arg(short = 'i', long)]
    input: Option<PathBuf>,
}

#[derive(clap::Args, Debug)]
struct CatalogArgs {
    #[arg(short = 'b', long, default_value = "v1.11")]
    branch: String,
    #[arg(short = 'B', long, default_value = "snapshots")]
    bucket: String,
    #[arg(short = 'k', long, default_value = "github")]
    key: String,
    #[arg(short = 's', long, env = "SNAPSHOTS_SECRET_ACCESS_KEY")]
    secret: String,
    #[arg(short = 'w', long, env = "WORKSPACE")]
    workspace: PathBuf,
    #[arg(short = 'E', long, default_value = "https://snapshots.workrave.org/")]
    endpoint: String,
    /// Output filename. Accepted for TypeScript tool compatibility; ignored by catalog.
    #[arg(short = 'n', long, default_value = "appcast.xml")]
    name: String,
    /// Output to local file instead of S3 bucket. Accepted for TypeScript tool compatibility; ignored by catalog.
    #[arg(long, default_value_t = false)]
    file: bool,
    /// Generate release notes starting from this release. Accepted for TypeScript tool compatibility; ignored by catalog.
    #[arg(long)]
    release: Option<String>,
    #[arg(short = 'd', long, default_value_t = false)]
    dry: bool,
    #[arg(short = 'r', long, default_value_t = false)]
    regenerate: bool,
}

#[derive(clap::Args, Debug)]
struct NewsgenArgs {
    /// YAML input file containing release notes
    #[arg(short = 'i', long)]
    input: PathBuf,
    /// Output file
    #[arg(short = 'o', long)]
    output: PathBuf,
    /// PPA increment for debian changelog
    #[arg(short = 'k', long, default_value_t = 1)]
    increment: i64,
    /// Ubuntu release name for debian changelog
    #[arg(short = 'U', long, default_value = "focal")]
    ubuntu: String,
    /// Generate release notes starting from this release
    #[arg(long)]
    release: Option<String>,
    /// Generate only release notes for the specified release
    #[arg(long, default_value_t = false)]
    single: bool,
    /// Generate only release notes for the latest release
    #[arg(long, default_value_t = false)]
    latest: bool,
    /// Release notes template to use
    #[arg(short = 'T', long, default_value = "NEWS")]
    template: String,
    #[arg(short = 'v', long, default_value_t = false)]
    verbose: bool,
}

#[tokio::main]
async fn main() -> Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info")),
        )
        .with_target(false)
        .init();

    let cli = Cli::parse();
    run_cli(cli).await
}

async fn run_cli(cli: Cli) -> Result<()> {
    match cli.command {
        Command::Appcast(a) => {
            appcast::run_appcast(appcast::AppcastOptions {
                branch: a.branch,
                bucket: a.bucket,
                environment: a.environment,
                key: a.key,
                secret: a.secret,
                endpoint: a.endpoint,
                name: a.name,
                file: a.file,
                release: a.release,
                dry: a.dry,
                input: a.input,
            })
            .await
        }
        Command::Catalog(c) => {
            let _ = (&c.name, c.file, &c.release);
            log_and_succeed(
                catalog::run_catalog(catalog::CatalogOptions {
                    branch: c.branch,
                    bucket: c.bucket,
                    key: c.key,
                    secret: c.secret,
                    workspace: c.workspace,
                    endpoint: c.endpoint,
                    dry: c.dry,
                    regenerate: c.regenerate,
                })
                .await,
            )
        }
        Command::Newsgen(n) => log_and_succeed(
            newsgen::run_newsgen(
                &n.input,
                &n.output,
                n.template,
                n.release,
                n.single,
                n.latest,
                n.ubuntu,
                n.increment,
            )
            .await,
        ),
    }
}

fn log_and_succeed(result: Result<()>) -> Result<()> {
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

    #[tokio::test]
    async fn newsgen_errors_exit_successfully_like_typescript() {
        let cli = Cli {
            command: Command::Newsgen(NewsgenArgs {
                input: PathBuf::from("/path/that/does/not/exist.yaml"),
                output: PathBuf::from("/path/that/does/not/exist.out"),
                increment: 1,
                ubuntu: "focal".to_string(),
                release: None,
                single: false,
                latest: false,
                template: "NEWS".to_string(),
                verbose: false,
            }),
        };

        assert!(run_cli(cli).await.is_ok());
    }

    #[tokio::test]
    async fn appcast_errors_still_propagate_like_typescript() {
        let cli = Cli {
            command: Command::Appcast(AppcastArgs {
                branch: "v1.11".to_string(),
                bucket: "snapshots".to_string(),
                environment: String::new(),
                key: "github".to_string(),
                secret: "secret".to_string(),
                endpoint: "http://127.0.0.1:1".to_string(),
                name: "appcast.xml".to_string(),
                file: false,
                release: None,
                dry: false,
                input: None,
            }),
        };

        assert!(run_cli(cli).await.is_err());
    }
}
