//! Support for podman talking to a remote service.
//!
//! With `podman system connection` / `CONTAINER_HOST`, the podman client runs
//! containers on another machine, and `-v /host/dir:/container/dir` refers
//! to `/host/dir` *on that machine*. [`Mirror`] copies the local directories
//! there with rsync (over the same ssh connection podman uses) before a build
//! and brings the results back afterwards.

use std::path::{Path, PathBuf};

use anyhow::{anyhow, bail, Context, Result};
use serde::Deserialize;

use super::Engine;
use crate::system::process::Cmd;

#[derive(Deserialize)]
struct PodmanInfo {
    host: PodmanHost,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct PodmanHost {
    #[serde(default)]
    service_is_remote: bool,
    #[serde(default)]
    hostname: String,
}

fn podman_info() -> Result<Option<PodmanInfo>> {
    let raw = match Cmd::new("podman")
        .args(["info", "--format", "json"])
        .output_quiet()
    {
        Ok(raw) => raw,
        Err(e) => {
            tracing::debug!("podman info failed: {e:#}");
            return Ok(None);
        }
    };
    Ok(Some(
        serde_json::from_str(&raw).context("parsing `podman info` output")?,
    ))
}

/// Whether `engine` runs containers on another machine.
pub fn is_remote(engine: Engine) -> Result<bool> {
    if engine != Engine::Podman {
        return Ok(false);
    }
    Ok(podman_info()?
        .map(|info| info.host.service_is_remote)
        .unwrap_or(false))
}

/// The remote host's name, when `engine` is a remote podman.
pub fn remote_hostname(engine: Engine) -> Result<Option<String>> {
    if engine != Engine::Podman {
        return Ok(None);
    }
    Ok(podman_info()?
        .filter(|info| info.host.service_is_remote)
        .map(|info| info.host.hostname))
}

#[derive(Deserialize)]
#[serde(rename_all = "PascalCase")]
struct Connection {
    name: String,
    #[serde(rename = "URI")]
    uri: String,
    #[serde(default)]
    identity: String,
    #[serde(default)]
    default: bool,
}

/// How to ssh to the host podman is talking to.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SshTarget {
    /// `user@host`
    pub host: String,
    pub port: Option<u16>,
    pub identity: Option<PathBuf>,
}

impl SshTarget {
    /// Works out the ssh target the same way podman does: `CONTAINER_HOST`
    /// (+ `CONTAINER_SSHKEY`), else the `CONTAINER_CONNECTION` or default
    /// entry of `podman system connection list`.
    pub fn detect() -> Result<SshTarget> {
        let (uri, identity) = match std::env::var("CONTAINER_HOST") {
            Ok(uri) if !uri.is_empty() => (uri, std::env::var("CONTAINER_SSHKEY").ok()),
            _ => {
                let raw = Cmd::new("podman")
                    .args(["system", "connection", "list", "--format", "json"])
                    .output_quiet()?;
                let wanted = std::env::var("CONTAINER_CONNECTION").ok();
                parse_connection(&raw, wanted.as_deref())?
            }
        };
        SshTarget::from_uri(&uri, identity.filter(|i| !i.is_empty()).map(PathBuf::from))
    }

    /// Parses `ssh://user@host[:port]/path`.
    pub fn from_uri(uri: &str, identity: Option<PathBuf>) -> Result<SshTarget> {
        let rest = uri.strip_prefix("ssh://").ok_or_else(|| {
            anyhow!("cannot determine the ssh host of the remote podman service (uri: '{uri}')")
        })?;
        let hostport = rest.split('/').next().unwrap_or(rest);
        let (host, port) = match hostport.rsplit_once(':') {
            Some((host, port)) => (
                host.to_string(),
                Some(
                    port.parse::<u16>()
                        .with_context(|| format!("invalid port in '{uri}'"))?,
                ),
            ),
            None => (hostport.to_string(), None),
        };
        if host.is_empty() {
            bail!("no host in podman connection uri '{uri}'");
        }
        Ok(SshTarget {
            host,
            port,
            identity,
        })
    }

    fn ssh_options(&self) -> Vec<String> {
        let mut opts = vec!["-o".to_string(), "BatchMode=yes".to_string()];
        if let Some(port) = self.port {
            opts.push("-p".to_string());
            opts.push(port.to_string());
        }
        if let Some(identity) = &self.identity {
            opts.push("-i".to_string());
            opts.push(identity.to_string_lossy().into_owned());
        }
        opts
    }

    /// `ssh <options> <host>`, ready for a remote command.
    pub fn ssh(&self) -> Cmd {
        Cmd::new("ssh").args(self.ssh_options()).arg(&self.host)
    }

    /// The `-e` argument for rsync.
    fn rsh(&self) -> String {
        let mut parts = vec!["ssh".to_string()];
        parts.extend(self.ssh_options());
        parts.join(" ")
    }
}

/// Picks the wanted (or default) connection from `podman system connection
/// list --format json` and returns its URI and identity file.
pub fn parse_connection(json: &str, wanted: Option<&str>) -> Result<(String, Option<String>)> {
    let connections: Vec<Connection> =
        serde_json::from_str(json).context("parsing `podman system connection list` output")?;
    let connection = match wanted {
        Some(name) => connections.iter().find(|c| c.name == name),
        None => connections.iter().find(|c| c.default),
    }
    .ok_or_else(|| match wanted {
        Some(name) => anyhow!("podman connection '{name}' not found"),
        None => anyhow!("no default podman connection"),
    })?;
    let identity = (!connection.identity.is_empty()).then(|| connection.identity.clone());
    Ok((connection.uri.clone(), identity))
}

/// Mirrors local directories to the remote podman host.
#[derive(Debug, Clone)]
pub struct Mirror {
    target: SshTarget,
    /// Absolute directory on the remote host under which local paths are
    /// mirrored (`<remote home>/<container.remote_dir>`).
    root: String,
}

impl Mirror {
    pub fn detect(remote_dir: &str) -> Result<Mirror> {
        let target = SshTarget::detect()?;
        let home = target
            .ssh()
            .arg("printf %s \"$HOME\"")
            .output_quiet()
            .with_context(|| format!("cannot ssh to {}", target.host))?;
        if home.is_empty() {
            bail!("cannot determine the home directory on {}", target.host);
        }
        Ok(Mirror::new(target, &home, remote_dir))
    }

    pub fn new(target: SshTarget, remote_home: &str, remote_dir: &str) -> Mirror {
        Mirror {
            target,
            root: format!(
                "{}/{}",
                remote_home.trim_end_matches('/'),
                remote_dir.trim_matches('/')
            ),
        }
    }

    /// `/Users/me/src/x` -> `/home/me/.cache/workrave-build/Users/me/src/x`.
    pub fn remote_path(&self, local: &Path) -> PathBuf {
        let local = local.to_string_lossy();
        let local = local.trim_end_matches('/');
        PathBuf::from(format!("{}{}", self.root, local))
    }

    pub fn sync_up(&self, dir: &Path) -> Result<()> {
        let remote = self.remote_path(dir);
        tracing::info!(
            "Syncing {} to {}:{}",
            dir.display(),
            self.target.host,
            remote.display()
        );
        self.target
            .ssh()
            .arg(format!("mkdir -p '{}'", remote.display()))
            .run()?;

        let mut rsync = Cmd::new("rsync").args(["-a", "--delete"]);
        for exclude in ignored_paths(dir)? {
            rsync = rsync.arg(format!("--exclude={exclude}"));
        }
        rsync
            .arg("-e")
            .arg(self.target.rsh())
            .arg(format!("{}/", dir.display()))
            .arg(format!("{}:{}/", self.target.host, remote.display()))
            .run()
    }

    pub fn sync_down(&self, dir: &Path) -> Result<()> {
        let remote = self.remote_path(dir);
        tracing::info!(
            "Syncing {}:{} back to {}",
            self.target.host,
            remote.display(),
            dir.display()
        );
        // -u: never overwrite files that were modified locally in the meantime.
        // Runs as cleanup, i.e. also after Ctrl-C.
        Cmd::new("rsync")
            .args(["-au", "--exclude=/.git"])
            .arg("-e")
            .arg(self.target.rsh())
            .arg(format!("{}:{}/", self.target.host, remote.display()))
            .arg(format!("{}/", dir.display()))
            .run_cleanup()
    }
}

/// Git-ignored paths under `dir` (relative to it, anchored with a leading
/// `/`), or nothing when `dir` is not inside a git work tree.
///
/// Ignored files are not uploaded: local build trees can be huge. And since
/// `rsync --delete` leaves excluded paths alone on the receiver, the build
/// tree on the remote host survives between runs.
fn ignored_paths(dir: &Path) -> Result<Vec<String>> {
    let in_worktree = Cmd::new("git")
        .arg("-C")
        .arg(dir)
        .args(["rev-parse", "--is-inside-work-tree"])
        .succeeds()?;
    if !in_worktree {
        return Ok(Vec::new());
    }
    let listing = Cmd::new("git")
        .arg("-C")
        .arg(dir)
        .args([
            "ls-files",
            "--others",
            "--ignored",
            "--exclude-standard",
            "--directory",
        ])
        .output_quiet()?;
    Ok(listing
        .lines()
        .filter(|l| !l.is_empty())
        .map(|l| format!("/{l}"))
        .collect())
}

#[cfg(test)]
mod tests {
    use super::*;

    const CONNECTIONS: &str = r#"[
        {"Name":"forge","URI":"ssh://robc@forge:22/run/user/1000/podman/podman.sock","Identity":"/Users/robc/.ssh/id_ed25519","Default":true,"ReadWrite":true},
        {"Name":"podman-machine-default","URI":"ssh://core@127.0.0.1:58325/run/user/501/podman/podman.sock","Identity":"/Users/robc/.local/share/containers/podman/machine/machine","IsMachine":true,"Default":false,"ReadWrite":true}
    ]"#;

    #[test]
    fn picks_default_connection() {
        let (uri, identity) = parse_connection(CONNECTIONS, None).unwrap();
        assert_eq!(uri, "ssh://robc@forge:22/run/user/1000/podman/podman.sock");
        assert_eq!(identity.as_deref(), Some("/Users/robc/.ssh/id_ed25519"));
    }

    #[test]
    fn picks_named_connection() {
        let (uri, _) = parse_connection(CONNECTIONS, Some("podman-machine-default")).unwrap();
        assert!(uri.starts_with("ssh://core@127.0.0.1:58325/"));
        assert!(parse_connection(CONNECTIONS, Some("nope")).is_err());
    }

    #[test]
    fn parses_ssh_uri() {
        let target = SshTarget::from_uri(
            "ssh://robc@forge:22/run/user/1000/podman/podman.sock",
            Some(PathBuf::from("/k")),
        )
        .unwrap();
        assert_eq!(target.host, "robc@forge");
        assert_eq!(target.port, Some(22));
        assert_eq!(
            target.ssh().display(),
            "ssh -o BatchMode=yes -p 22 -i /k robc@forge"
        );
        assert_eq!(target.rsh(), "ssh -o BatchMode=yes -p 22 -i /k");

        let plain = SshTarget::from_uri("ssh://robc@forge/x", None).unwrap();
        assert_eq!(plain.port, None);
        assert_eq!(plain.ssh().display(), "ssh -o BatchMode=yes robc@forge");

        assert!(SshTarget::from_uri("unix:///run/podman.sock", None).is_err());
    }

    #[test]
    fn remote_path_mirrors_local_absolute_path() {
        let target = SshTarget::from_uri("ssh://robc@forge/x", None).unwrap();
        let mirror = Mirror::new(target, "/home/robc", ".cache/workrave-build");
        assert_eq!(
            mirror.remote_path(Path::new("/Users/robc/src/workrave/tools/")),
            PathBuf::from("/home/robc/.cache/workrave-build/Users/robc/src/workrave/tools")
        );
    }

    #[test]
    fn ignored_paths_are_anchored() {
        // The ship crate directory is inside the workrave checkout; its
        // target directory is ignored.
        let dir = Path::new(env!("CARGO_MANIFEST_DIR"));
        let ignored = ignored_paths(dir).unwrap();
        assert!(ignored.iter().any(|p| p == "/target/"), "{ignored:?}");
        assert!(ignored.iter().all(|p| p.starts_with('/')));
    }
}
