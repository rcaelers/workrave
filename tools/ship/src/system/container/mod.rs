//! Running the build images with podman or docker.
//!
//! Builds run inside `ghcr.io/rcaelers/workrave-build:<image>` containers with
//! the source, deploy and scripts directories bind-mounted. When podman talks
//! to a remote service, those directories are mirrored to the remote host
//! first (see [`remote`]); [`Mounts`] hides that difference: callers give it
//! the local directories and ask it for the host-side path of each mount.

pub mod remote;

use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Engine {
    Podman,
    Docker,
}

impl Engine {
    pub fn program(self) -> &'static str {
        match self {
            Engine::Podman => "podman",
            Engine::Docker => "docker",
        }
    }
}

impl std::str::FromStr for Engine {
    type Err = anyhow::Error;
    fn from_str(s: &str) -> Result<Engine> {
        match s {
            "podman" | "" => Ok(Engine::Podman),
            "docker" => Ok(Engine::Docker),
            other => bail!("unknown container engine '{other}' (podman or docker)"),
        }
    }
}

/// How local directories reach the container host when podman talks to a
/// remote service.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SyncMode {
    /// rsync when podman is remote and the directories are not visible.
    Auto,
    /// Always rsync (requires a remote podman).
    Rsync,
    /// Never rsync; fail if the mounts do not work.
    None,
}

impl std::str::FromStr for SyncMode {
    type Err = anyhow::Error;
    fn from_str(s: &str) -> Result<SyncMode> {
        match s {
            "auto" | "" => Ok(SyncMode::Auto),
            "rsync" => Ok(SyncMode::Rsync),
            "none" => Ok(SyncMode::None),
            other => bail!("unknown container sync mode '{other}' (auto, rsync or none)"),
        }
    }
}

/// How an environment's containers run.
#[derive(Debug, Clone)]
pub struct ContainerSettings {
    pub engine: Engine,
    pub sync: SyncMode,
    /// Directory on the remote host (relative to its home) under which
    /// local directories are mirrored.
    pub remote_dir: String,
}

impl Default for ContainerSettings {
    fn default() -> Self {
        ContainerSettings {
            engine: Engine::Podman,
            sync: SyncMode::Auto,
            remote_dir: ".cache/ship".to_string(),
        }
    }
}
use crate::system::process::Cmd;

/// One `<engine> run` invocation.
#[derive(Debug, Clone)]
pub struct ContainerRun {
    image: String,
    platform: Option<String>,
    /// Local directory -> path inside the container.
    mounts: Vec<(PathBuf, String)>,
    env: Vec<(String, String)>,
    /// Extra `run` flags, e.g. `--privileged`.
    flags: Vec<String>,
    command: Vec<String>,
}

impl ContainerRun {
    pub fn new(image: impl Into<String>) -> Self {
        Self {
            image: image.into(),
            platform: None,
            mounts: Vec::new(),
            env: Vec::new(),
            flags: Vec::new(),
            command: Vec::new(),
        }
    }

    pub fn platform(mut self, platform: impl Into<String>) -> Self {
        self.platform = Some(platform.into());
        self
    }

    /// Names the container, so it can be removed after an interrupt.
    pub fn name(mut self, name: impl Into<String>) -> Self {
        self.flags.push("--name".to_string());
        self.flags.push(name.into());
        self
    }

    pub fn mount(mut self, local: impl Into<PathBuf>, guest: impl Into<String>) -> Self {
        self.mounts.push((local.into(), guest.into()));
        self
    }

    pub fn env(mut self, key: impl Into<String>, value: impl Into<String>) -> Self {
        self.env.push((key.into(), value.into()));
        self
    }

    pub fn envs<I, K, V>(mut self, envs: I) -> Self
    where
        I: IntoIterator<Item = (K, V)>,
        K: Into<String>,
        V: Into<String>,
    {
        for (k, v) in envs {
            self = self.env(k, v);
        }
        self
    }

    pub fn flags<I, S>(mut self, flags: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: Into<String>,
    {
        self.flags.extend(flags.into_iter().map(Into::into));
        self
    }

    pub fn command<I, S>(mut self, command: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: Into<String>,
    {
        self.command = command.into_iter().map(Into::into).collect();
        self
    }

    /// The command line to run this container, with local mount sources
    /// translated by `mounts`.
    pub fn to_cmd(&self, engine: Engine, mounts: &Mounts) -> Cmd {
        let mut cmd = Cmd::new(engine.program()).args(["run", "--rm"]);
        if let Some(platform) = &self.platform {
            cmd = cmd.arg("--platform").arg(platform);
        }
        cmd = cmd.args(&self.flags);
        for (local, guest) in &self.mounts {
            cmd = cmd
                .arg("-v")
                .arg(format!("{}:{}", mounts.host_path(local).display(), guest));
        }
        for (key, value) in &self.env {
            cmd = cmd.arg("-e").arg(format!("{key}={value}"));
        }
        cmd.arg(&self.image).args(&self.command)
    }
}

/// Removes a (possibly still running) container; errors are ignored, the
/// container is normally gone already (`--rm`).
pub fn remove_container(engine: Engine, name: &str) {
    let mut cmd = Cmd::new(engine.program()).args(["rm", "-f"]);
    if engine == Engine::Podman {
        cmd = cmd.arg("--ignore");
    }
    let _ = cmd.arg(name).run_cleanup();
}

/// Verifies the engine can run `image` for each platform. Running a foreign
/// architecture needs qemu-user-static binfmt handlers on the container host.
pub fn check_platforms(engine: Engine, image: &str, platforms: &[&str]) -> Result<()> {
    for platform in platforms {
        let ok = Cmd::new(engine.program())
            .args(["run", "--rm", "--platform", platform, image, "true"])
            .succeeds()?;
        if !ok {
            let mut message = format!(
                "{} cannot run {image} for platform {platform}.\n",
                engine.program()
            );
            if let Some(host) = remote::remote_hostname(engine)? {
                message += &format!("On the podman host ('{host}'), install the ");
            } else {
                message += "Install the ";
            }
            message += "binfmt handlers for foreign architectures, e.g. on Debian/Ubuntu:\n    sudo apt install qemu-user-static binfmt-support";
            bail!(message);
        }
    }
    Ok(())
}

#[derive(Debug, PartialEq, Eq)]
enum Visibility {
    Visible,
    Missing,
    /// The container could not be run at all (engine error, host path
    /// missing on a remote podman, ...).
    Failed(String),
}

/// Checks whether the local directories are visible inside a container, by
/// dropping a unique marker file in each and looking for it through the mount.
fn mounts_visible(
    engine: Engine,
    image: &str,
    platform: Option<&str>,
    dirs: &[PathBuf],
) -> Result<Visibility> {
    let marker = format!(
        ".container-mount-check.{}.{}",
        std::process::id(),
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .map(|d| d.subsec_nanos())
            .unwrap_or(0)
    );
    let mut cmd = Cmd::new(engine.program()).args(["run", "--rm"]);
    if let Some(platform) = platform {
        cmd = cmd.args(["--platform", platform]);
    }
    let mut test = String::from("true");
    for (i, dir) in dirs.iter().enumerate() {
        std::fs::write(dir.join(&marker), b"")
            .with_context(|| format!("creating marker file in {}", dir.display()))?;
        cmd = cmd
            .arg("-v")
            .arg(format!("{}:/mnt/check{i}", dir.display()));
        test += &format!(" && test -f /mnt/check{i}/{marker}");
    }
    cmd = cmd.arg(image).args([
        "sh",
        "-c",
        &format!("if {test}; then echo MOUNT_OK; else echo MOUNT_MISSING; fi"),
    ]);

    let result = cmd.output_quiet();
    for dir in dirs {
        let _ = std::fs::remove_file(dir.join(&marker));
    }
    Ok(match result {
        Ok(out) if out.trim() == "MOUNT_OK" => Visibility::Visible,
        Ok(out) if out.trim() == "MOUNT_MISSING" => Visibility::Missing,
        Ok(out) => Visibility::Failed(format!("unexpected output: {out}")),
        Err(e) => Visibility::Failed(format!("{e:#}")),
    })
}

/// The local directories a set of container runs needs, and how they reach
/// the container host.
pub struct Mounts {
    dirs: Vec<PathBuf>,
    mirror: Option<remote::Mirror>,
}

impl Mounts {
    /// Mounts that use the local directories as they are, without any check
    /// (for showing the plan).
    pub fn direct(dirs: Vec<PathBuf>) -> Mounts {
        Mounts { dirs, mirror: None }
    }

    /// Makes `dirs` available on the container host: either they are visible
    /// directly, or they are mirrored to the remote podman host with rsync
    /// according to `config.sync`.
    pub fn prepare(
        engine: Engine,
        config: &ContainerSettings,
        image: &str,
        platform: Option<&str>,
        dirs: Vec<PathBuf>,
    ) -> Result<Mounts> {
        for dir in &dirs {
            if !dir.is_dir() {
                bail!("{} is not a directory", dir.display());
            }
        }

        let use_rsync = match config.sync {
            SyncMode::None => false,
            SyncMode::Rsync => {
                if !remote::is_remote(engine)? {
                    bail!(
                        "container.sync is rsync, but {} is not talking to a remote service",
                        engine.program()
                    );
                }
                true
            }
            SyncMode::Auto => {
                if remote::is_remote(engine)? {
                    match mounts_visible(engine, image, platform, &dirs)? {
                        Visibility::Visible => {
                            tracing::info!("Local directories are visible on the remote podman host; not syncing");
                            false
                        }
                        Visibility::Missing | Visibility::Failed(_) => {
                            tracing::info!("Local directories are not visible on the remote podman host; syncing with rsync");
                            true
                        }
                    }
                } else {
                    false
                }
            }
        };

        let mirror = if use_rsync {
            let mirror = remote::Mirror::detect(&config.remote_dir)?;
            for dir in &dirs {
                mirror.sync_up(dir)?;
            }
            Some(mirror)
        } else {
            match mounts_visible(engine, image, platform, &dirs)? {
                Visibility::Visible => None,
                Visibility::Missing => {
                    let list = dirs
                        .iter()
                        .map(|d| format!("    {}", d.display()))
                        .collect::<Vec<_>>()
                        .join("\n");
                    bail!(
                        "the following directories are not visible inside the {} container:\n{list}{}",
                        engine.program(),
                        remote_hint(engine)?
                    );
                }
                Visibility::Failed(error) => {
                    bail!(
                        "failed to run {image} with {}: {error}{}",
                        engine.program(),
                        remote_hint(engine)?
                    );
                }
            }
        };

        Ok(Mounts { dirs, mirror })
    }

    /// The path to use on the host side of a mount for local directory `dir`.
    /// `dir` may be a subdirectory of one of the prepared directories.
    pub fn host_path(&self, dir: &Path) -> PathBuf {
        match &self.mirror {
            Some(mirror) => mirror.remote_path(dir),
            None => dir.to_path_buf(),
        }
    }

    /// Brings results back from the remote host, if mirrored.
    pub fn finish(&self) -> Result<()> {
        if let Some(mirror) = &self.mirror {
            for dir in &self.dirs {
                mirror.sync_down(dir)?;
            }
        }
        Ok(())
    }
}

fn remote_hint(engine: Engine) -> Result<String> {
    Ok(match remote::remote_hostname(engine)? {
        Some(host) => format!(
            "\npodman is using a remote service on '{host}'; bind mounts refer to paths on that machine.\n\
             Set container.sync to rsync (or auto) to mirror the directories there, use docker,\n\
             or select another podman connection (podman system connection default <name>) to run locally."
        ),
        None => String::new(),
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn run_command_line() {
        let mounts = Mounts {
            dirs: vec![],
            mirror: None,
        };
        let run = ContainerRun::new("ghcr.io/x/build:img")
            .platform("linux/amd64")
            .flags(["--cap-add", "SYS_ADMIN"])
            .mount("/src", "/workspace/source")
            .env("WORKRAVE_PPA_SERIES", "stonking resolute")
            .command(["/workspace/source/tools/ci/build.sh"]);
        assert_eq!(
            run.to_cmd(Engine::Podman, &mounts).display(),
            "podman run --rm --platform linux/amd64 --cap-add SYS_ADMIN -v /src:/workspace/source \
             -e 'WORKRAVE_PPA_SERIES=stonking resolute' ghcr.io/x/build:img /workspace/source/tools/ci/build.sh"
        );
    }
}
