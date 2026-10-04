//! `sign`: sign files through the signing service.

use anyhow::{bail, Result};
use serde::Deserialize;

use super::params::{params, StringOrList};
use super::{Action, ActionEnv, ActionFuture, Outcome};

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// cosign (writes <file>.sigstore) | ed25519 (prints the signature) |
    /// authenticode (replaces the file) | catalog (signs the artifacts
    /// listed in the job-catalog*.json files under a directory)
    kind: String,
    /// Files, or directories for `catalog`; globs allowed.
    files: StringOrList,
}

pub struct Sign;

impl Action for Sign {
    fn name(&self) -> &'static str {
        "sign"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        let p: Params = params(with)?;
        match p.kind.as_str() {
            "cosign" | "ed25519" | "authenticode" | "catalog" => Ok(()),
            // Templated kinds are checked when the step runs.
            k if k.contains("{{") => Ok(()),
            other => bail!("sign: unknown kind '{other}' (cosign, ed25519, authenticode, catalog)"),
        }
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            let files = p.files.paths()?;
            if files.is_empty() {
                bail!("sign: no files match {:?}", p.files.items());
            }
            if env.dry_run {
                for file in &files {
                    println!("DRYRUN: sign {} {}", p.kind, file.display());
                }
                return Ok(Outcome::default());
            }
            let signing = env.signing()?;
            for file in &files {
                match p.kind.as_str() {
                    "cosign" => {
                        signing.cosign(file).await?;
                    }
                    "ed25519" => println!("{}", signing.ed25519(file).await?),
                    "authenticode" => signing.authenticode(file).await?,
                    "catalog" => signing.sign_catalogs(file).await?,
                    other => bail!("sign: unknown kind '{other}'"),
                }
            }
            Ok(Outcome::default())
        })
    }
}
