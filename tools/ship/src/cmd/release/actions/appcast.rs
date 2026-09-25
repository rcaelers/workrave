//! `appcast`: generate the auto-update appcast from the artifact catalog in
//! the snapshots bucket, into a local file.

use anyhow::Result;
use serde::Deserialize;

use super::params::{params, BucketParams};
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::services::appcast;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// Catalog directory in the bucket, e.g. `v1.12` or `staging/v1.12`.
    branch: String,
    /// Marks the appcast as `staging` when set to that; empty for production.
    #[serde(default)]
    environment: String,
    /// The appcast.xml to write.
    output: String,
    #[serde(flatten)]
    bucket: BucketParams,
}

pub struct Appcast;

impl Action for Appcast {
    fn name(&self) -> &'static str {
        "appcast"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            if env.dry_run {
                println!("DRYRUN: appcast --branch {} --file {}", p.branch, p.output);
                return Ok(Outcome::default());
            }
            appcast::run_appcast(appcast::AppcastOptions {
                branch: p.branch,
                bucket: p.bucket.bucket.clone(),
                environment: if p.environment == "production" {
                    String::new()
                } else {
                    p.environment.clone()
                },
                key: p.bucket.access_key.clone(),
                secret: p.bucket.secret_key.clone(),
                endpoint: p.bucket.endpoint.clone(),
                name: p.output.clone(),
                file: true,
                release: None,
                dry: false,
                input: None,
            })
            .await?;
            tracing::info!("Wrote {}", p.output);
            Ok(Outcome::default())
        })
    }
}
