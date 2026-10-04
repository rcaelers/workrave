//! `catalog`: merge this build's job catalogs into the artifact catalog in
//! the snapshots bucket.

use std::path::PathBuf;

use anyhow::Result;
use serde::Deserialize;

use super::params::{params, BucketParams};
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::services::catalog;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// Catalog directory in the bucket, e.g. `v1.12` or `staging/v1.12`.
    branch: String,
    /// Source checkout with the `_deploy/*/job-catalog*.json` files.
    workspace: String,
    #[serde(flatten)]
    bucket: BucketParams,
}

pub struct Catalog;

impl Action for Catalog {
    fn name(&self) -> &'static str {
        "catalog"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            if env.dry_run {
                println!(
                    "DRYRUN: catalog --branch {} --workspace {}",
                    p.branch, p.workspace
                );
                return Ok(Outcome::default());
            }
            catalog::run_catalog(catalog::CatalogOptions {
                branch: p.branch,
                bucket: p.bucket.bucket.clone(),
                key: p.bucket.access_key.clone(),
                secret: p.bucket.secret_key.clone(),
                workspace: PathBuf::from(p.workspace),
                endpoint: p.bucket.endpoint.clone(),
                dry: false,
                regenerate: false,
            })
            .await?;
            Ok(Outcome::default())
        })
    }
}
