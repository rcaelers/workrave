//! `ship secret`: fetch a secret from the signing service (debugging).

use anyhow::Result;

use super::SigningArgs;

#[derive(clap::Args, Debug)]
pub struct SecretCommand {
    #[command(flatten)]
    signing: SigningArgs,
    /// Secret name, e.g. secrets.tokens.github_pat
    name: String,
}

pub async fn run(args: SecretCommand) -> Result<()> {
    let service = args.signing.service()?;
    println!("{}", service.secret(&args.name).await?);
    Ok(())
}
