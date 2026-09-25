//! `ship newsgen`: release notes from changes.yaml (NEWS, GitHub, blog,
//! Debian changelog, ...).

use std::path::PathBuf;

use anyhow::Result;

use super::log_and_succeed;
use crate::news::newsgen::run_newsgen;

#[derive(clap::Args, Debug)]
pub struct NewsgenCommand {
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

pub async fn run(n: NewsgenCommand) -> Result<()> {
    log_and_succeed(
        run_newsgen(
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
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn errors_exit_successfully_like_typescript() {
        let args = NewsgenCommand {
            input: PathBuf::from("/path/that/does/not/exist.yaml"),
            output: PathBuf::from("/path/that/does/not/exist.out"),
            increment: 1,
            ubuntu: "focal".to_string(),
            release: None,
            single: false,
            latest: false,
            template: "NEWS".to_string(),
            verbose: false,
        };
        assert!(run(args).await.is_ok());
    }
}
