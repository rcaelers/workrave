//! `check-no-homebrew-links`: verify that nothing in a macOS build links
//! against Homebrew.

use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};
use serde::Deserialize;

use super::params::params;
use super::{Action, ActionEnv, ActionFuture, Outcome};
use crate::system::process::Cmd;

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct Params {
    /// The app bundle or output directory to check.
    dir: String,
}

pub struct CheckNoHomebrewLinks;

impl Action for CheckNoHomebrewLinks {
    fn name(&self) -> &'static str {
        "check-no-homebrew-links"
    }

    fn validate(&self, with: &serde_yaml::Value) -> Result<()> {
        params::<Params>(with).map(|_| ())
    }

    fn run<'a>(&'a self, with: &'a serde_yaml::Value, _env: &'a ActionEnv<'a>) -> ActionFuture<'a> {
        Box::pin(async move {
            let p: Params = params(with)?;
            check_no_homebrew_links(Path::new(&p.dir))?;
            Ok(Outcome::default())
        })
    }
}

/// The macOS dependencies tree is supposed to be self-contained (universal,
/// buildable on a clean machine), so nothing in the app bundle may link
/// against or rpath into Homebrew — that would silently make it not actually
/// self-contained. Mirrors workrave-dependencies/macos/check-no-homebrew-links.sh.
fn check_no_homebrew_links(bundle: &Path) -> Result<()> {
    const BLOCKED: [&str; 6] = [
        "/opt/homebrew",
        "/usr/local/Cellar",
        "/usr/local/opt",
        "/usr/local/lib",
        "/usr/local/bin",
        "/usr/local/share",
    ];
    let blocked = |s: &str| BLOCKED.iter().any(|b| s.contains(b));

    let mut found = false;
    for file in mach_o_files(bundle)? {
        let deps = Cmd::new("otool").arg("-L").arg(&file).output_quiet()?;
        for dep in deps
            .lines()
            .skip(1)
            .filter_map(|l| l.split_whitespace().next())
        {
            if blocked(dep) {
                tracing::error!("{} links against Homebrew: {dep}", file.display());
                found = true;
            }
        }
        let load = Cmd::new("otool").arg("-l").arg(&file).output_quiet()?;
        let lines: Vec<&str> = load.lines().collect();
        for (i, line) in lines.iter().enumerate() {
            if line.contains("LC_RPATH") {
                if let Some(path) = lines.get(i + 2).and_then(|l| l.split_whitespace().nth(1)) {
                    if blocked(path) {
                        tracing::error!("{} has an LC_RPATH into Homebrew: {path}", file.display());
                        found = true;
                    }
                }
            }
        }
    }
    if found {
        bail!(
            "Homebrew linkage detected in {}; see above",
            bundle.display()
        );
    }
    tracing::info!("No Homebrew linkage in {}", bundle.display());
    Ok(())
}

/// Every dylib or executable file under `dir` that is a Mach-O binary.
fn mach_o_files(dir: &Path) -> Result<Vec<PathBuf>> {
    let mut files = Vec::new();
    let mut stack = vec![dir.to_path_buf()];
    while let Some(current) = stack.pop() {
        for entry in
            std::fs::read_dir(&current).with_context(|| format!("listing {}", current.display()))?
        {
            let entry = entry?;
            let path = entry.path();
            let meta = entry.metadata()?;
            if meta.is_dir() {
                stack.push(path);
            } else if meta.is_file()
                && (path.extension().is_some_and(|e| e == "dylib") || is_executable(&meta))
            {
                let kind = Cmd::new("file")
                    .arg(&path)
                    .output_quiet()
                    .unwrap_or_default();
                if kind.contains("Mach-O") {
                    files.push(path);
                }
            }
        }
    }
    files.sort();
    Ok(files)
}

#[cfg(unix)]
fn is_executable(meta: &std::fs::Metadata) -> bool {
    use std::os::unix::fs::PermissionsExt;
    meta.permissions().mode() & 0o100 != 0
}

#[cfg(not(unix))]
fn is_executable(_meta: &std::fs::Metadata) -> bool {
    false
}
