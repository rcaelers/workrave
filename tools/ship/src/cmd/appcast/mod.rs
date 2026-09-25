//! `ship appcast`: generate the auto-update appcast from the artifact
//! catalog in the snapshots bucket.

use std::path::PathBuf;

use anyhow::Result;

use crate::services::appcast::{run_appcast, AppcastOptions};

#[derive(clap::Args, Debug)]
pub struct AppcastCommand {
    #[arg(short = 'b', long, default_value = "v1.12")]
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

pub async fn run(a: AppcastCommand) -> Result<()> {
    run_appcast(AppcastOptions {
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

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn errors_propagate() {
        let args = AppcastCommand {
            branch: "v1.12".to_string(),
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
        };
        assert!(run(args).await.is_err());
    }
}
