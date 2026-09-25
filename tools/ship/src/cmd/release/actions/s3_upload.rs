//! `s3-upload`: upload files or directories to the snapshots bucket.

use anyhow::{bail, Result};
use serde::Deserialize;

use super::params::{params, BucketParams, StringOrList};
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::services::s3::content_type_for;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// Files or directories (uploaded recursively); globs allowed.
    files: StringOrList,
    /// Key prefix in the bucket, e.g. `v1.12`.
    prefix: String,
    #[serde(flatten)]
    bucket: BucketParams,
}

pub struct S3Upload;

impl Action for S3Upload {
    fn name(&self) -> &'static str {
        "s3-upload"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            let paths = p.files.paths()?;
            if paths.is_empty() {
                bail!("s3-upload: no files match {:?}", p.files.items());
            }
            let bucket = &p.bucket.bucket;
            if env.dry_run {
                for path in &paths {
                    println!(
                        "DRYRUN: upload {} to s3://{bucket}/{}/",
                        path.display(),
                        p.prefix
                    );
                }
                return Ok(Outcome::default());
            }
            let store = p.bucket.store();
            for path in &paths {
                if path.is_dir() {
                    store.upload_dir(path, &p.prefix).await?;
                } else {
                    let name = path.file_name().unwrap_or_default().to_string_lossy();
                    let key = format!("{}/{name}", p.prefix.trim_end_matches('/'));
                    tracing::info!("Uploading {} to s3://{bucket}/{key}", path.display());
                    store
                        .write(&key, tokio::fs::read(path).await?, content_type_for(path))
                        .await?;
                }
            }
            Ok(Outcome::default())
        })
    }
}
