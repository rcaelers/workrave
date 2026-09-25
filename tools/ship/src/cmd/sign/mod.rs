//! `ship sign`: sign files through the signing service. Also the CMake
//! signing hook (`WITH_SIGN_TOOL`).

use std::path::{Path, PathBuf};

use anyhow::{bail, Result};
use clap::Subcommand;

use super::SigningArgs;

#[derive(clap::Args, Debug)]
pub struct SignCommand {
    #[command(flatten)]
    signing: SigningArgs,
    #[command(subcommand)]
    command: Kind,
}

#[derive(Subcommand, Debug)]
enum Kind {
    /// create a sigstore bundle <file>.sigstore for each file
    Cosign { files: Vec<PathBuf> },
    /// print the base64 ed25519 signature of each file
    Ed25519 { files: Vec<PathBuf> },
    /// replace each file by its authenticode-signed version
    Authenticode { files: Vec<PathBuf> },
    /// ed25519-sign all artifacts listed in the job-catalog*.json files under a directory
    Catalog { dir: PathBuf },
}

pub async fn run(args: SignCommand) -> Result<()> {
    let service = args.signing.service()?;
    match args.command {
        Kind::Cosign { files } => {
            for file in require_files(&files)? {
                service.cosign(file).await?;
            }
        }
        Kind::Ed25519 { files } => {
            for file in require_files(&files)? {
                println!("{}", service.ed25519(file).await?);
            }
        }
        Kind::Authenticode { files } => {
            for file in require_files(&files)? {
                service.authenticode(file).await?;
            }
        }
        Kind::Catalog { dir } => service.sign_catalogs(&dir).await?,
    }
    Ok(())
}

fn require_files(files: &[PathBuf]) -> Result<Vec<&Path>> {
    if files.is_empty() {
        bail!("no files given");
    }
    for file in files {
        if !file.is_file() {
            bail!("{} does not exist", file.display());
        }
    }
    Ok(files.iter().map(PathBuf::as_path).collect())
}
