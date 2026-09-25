//! `ship catalog`: merge a build's job catalogs into the artifact catalog in
//! the snapshots bucket (used by CI).

use std::path::PathBuf;

use anyhow::Result;

use super::log_and_succeed;
use crate::services::catalog::{run_catalog, CatalogOptions};

#[derive(clap::Args, Debug)]
pub struct CatalogCommand {
    #[arg(short = 'b', long, default_value = "v1.12")]
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

pub async fn run(c: CatalogCommand) -> Result<()> {
    let _ = (&c.name, c.file, &c.release);
    log_and_succeed(
        run_catalog(CatalogOptions {
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
