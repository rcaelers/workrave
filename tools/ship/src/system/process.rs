//! Running external programs.
//!
//! Every subprocess the release tooling starts goes through [`Cmd`], so that
//! failures always report the exact command line, and so that a dry run can
//! print what it would have done instead of doing it.

use std::ffi::{OsStr, OsString};
use std::path::{Path, PathBuf};
use std::process::{Child, Command, Stdio};
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use std::sync::Mutex;
use std::time::Duration;

use anyhow::{anyhow, bail, Context, Result};

/// Set by the first Ctrl-C: the running command is stopped and every
/// following [`Cmd::run`] fails, so the pipeline unwinds through its cleanup
/// (mirrored directories are synced back, containers removed).
static INTERRUPTED: AtomicBool = AtomicBool::new(false);

/// The pid of the command currently running, if any.
static RUNNING: AtomicU32 = AtomicU32::new(0);

pub fn interrupted() -> bool {
    INTERRUPTED.load(Ordering::SeqCst)
}

/// Installs the Ctrl-C handler: the first Ctrl-C interrupts the running
/// command and lets the cleanup run, a second one exits immediately.
pub fn install_interrupt_handler() {
    tokio::spawn(async {
        if tokio::signal::ctrl_c().await.is_err() {
            return;
        }
        INTERRUPTED.store(true, Ordering::SeqCst);
        eprintln!("\nInterrupted; stopping and cleaning up (Ctrl-C again to abort immediately)");
        stop_running();
        if tokio::signal::ctrl_c().await.is_ok() {
            eprintln!("\nAborted");
            std::process::exit(130);
        }
    });
}

/// Stops the running command: SIGINT first (so e.g. podman stops its
/// container), SIGKILL if it is still there after a few seconds.
fn stop_running() {
    let pid = RUNNING.load(Ordering::SeqCst);
    if pid == 0 {
        return;
    }
    #[cfg(unix)]
    {
        // The child is the leader of its own process group: signal the group.
        // SAFETY: plain signal delivery to a process group we created.
        unsafe {
            libc::kill(-(pid as libc::pid_t), libc::SIGINT);
        }
        std::thread::spawn(move || {
            std::thread::sleep(Duration::from_secs(5));
            if RUNNING.load(Ordering::SeqCst) == pid {
                unsafe {
                    libc::kill(-(pid as libc::pid_t), libc::SIGKILL);
                }
            }
        });
    }
    #[cfg(not(unix))]
    {
        let _ = Command::new("taskkill")
            .args(["/PID", &pid.to_string(), "/T", "/F"])
            .status();
    }
}

/// Waits for a spawned child, keeping it interruptible.
fn wait(mut child: Child, display: &str) -> Result<std::process::ExitStatus> {
    RUNNING.store(child.id(), Ordering::SeqCst);
    let status = child.wait();
    RUNNING.store(0, Ordering::SeqCst);
    let status = status.with_context(|| format!("waiting for `{display}`"))?;
    if interrupted() {
        bail!("interrupted");
    }
    Ok(status)
}

/// Values that must never appear in logs (secrets fetched from the signing
/// service). [`Cmd::display`] replaces them with `***`.
static REDACTED: Mutex<Vec<String>> = Mutex::new(Vec::new());

pub fn redact(secret: &str) {
    if secret.is_empty() {
        return;
    }
    let mut list = REDACTED.lock().unwrap();
    if !list.iter().any(|s| s == secret) {
        list.push(secret.to_string());
    }
}

/// `text` with every registered secret replaced by `***`.
pub fn redacted(text: &str) -> String {
    let list = REDACTED.lock().unwrap();
    let mut out = text.to_string();
    for secret in list.iter() {
        out = out.replace(secret, "***");
    }
    out
}

#[derive(Debug, Clone)]
pub struct Cmd {
    program: OsString,
    args: Vec<OsString>,
    envs: Vec<(OsString, OsString)>,
    cwd: Option<PathBuf>,
}

impl Cmd {
    pub fn new(program: impl AsRef<OsStr>) -> Self {
        Self {
            program: program.as_ref().to_os_string(),
            args: Vec::new(),
            envs: Vec::new(),
            cwd: None,
        }
    }

    pub fn arg(mut self, arg: impl AsRef<OsStr>) -> Self {
        self.args.push(arg.as_ref().to_os_string());
        self
    }

    pub fn args<I, S>(mut self, args: I) -> Self
    where
        I: IntoIterator<Item = S>,
        S: AsRef<OsStr>,
    {
        self.args
            .extend(args.into_iter().map(|a| a.as_ref().to_os_string()));
        self
    }

    pub fn env(mut self, key: impl AsRef<OsStr>, value: impl AsRef<OsStr>) -> Self {
        self.envs
            .push((key.as_ref().to_os_string(), value.as_ref().to_os_string()));
        self
    }

    pub fn envs<I, K, V>(mut self, envs: I) -> Self
    where
        I: IntoIterator<Item = (K, V)>,
        K: AsRef<OsStr>,
        V: AsRef<OsStr>,
    {
        for (k, v) in envs {
            self = self.env(k, v);
        }
        self
    }

    pub fn cwd(mut self, dir: impl AsRef<Path>) -> Self {
        self.cwd = Some(dir.as_ref().to_path_buf());
        self
    }

    /// The command line as it would be typed in a shell, for logging.
    /// Secrets registered with [`redact`] are masked.
    pub fn display(&self) -> String {
        let mut parts = Vec::with_capacity(self.args.len() + 1);
        parts.push(shell_quote(&self.program.to_string_lossy()));
        parts.extend(self.args.iter().map(|a| shell_quote(&a.to_string_lossy())));
        redacted(&parts.join(" "))
    }

    fn command(&self) -> Command {
        let mut command = Command::new(&self.program);
        command.args(&self.args);
        // Each command gets its own process group, so an interrupt reaches
        // everything it spawned (a shell's `sleep`, rsync's ssh, ...).
        #[cfg(unix)]
        {
            use std::os::unix::process::CommandExt;
            command.process_group(0);
        }
        for (k, v) in &self.envs {
            command.env(k, v);
        }
        if let Some(cwd) = &self.cwd {
            command.current_dir(cwd);
        }
        command
    }

    /// Runs the command with inherited stdio and fails on a non-zero exit.
    /// Fails without running once Ctrl-C was pressed, except for cleanup
    /// commands ([`Cmd::run_cleanup`]).
    pub fn run(&self) -> Result<()> {
        if interrupted() {
            bail!("interrupted");
        }
        self.run_cleanup()
    }

    /// Like [`Cmd::run`], but also runs after Ctrl-C: for commands that put
    /// things back in order (syncing results back, removing containers).
    pub fn run_cleanup(&self) -> Result<()> {
        tracing::info!("+ {}", self.display());
        let child = self
            .command()
            .spawn()
            .with_context(|| format!("starting `{}`", self.display()))?;
        let status = wait(child, &self.display())?;
        if !status.success() {
            bail!("`{}` failed with {}", self.display(), status);
        }
        Ok(())
    }

    /// Runs the command, or only prints it when `dry_run` is set. Use this
    /// for every side effect a dry run must not perform.
    pub fn run_or_echo(&self, dry_run: bool) -> Result<()> {
        if dry_run {
            println!("DRYRUN: {}", self.display());
            Ok(())
        } else {
            self.run()
        }
    }

    /// Runs the command and returns whether it succeeded, without failing.
    /// Output is discarded; use this for probes.
    pub fn succeeds(&self) -> Result<bool> {
        tracing::debug!("+ {}", self.display());
        let status = self
            .command()
            .stdin(Stdio::null())
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()
            .with_context(|| format!("starting `{}`", self.display()))?;
        Ok(status.success())
    }

    /// Runs the command and returns its trimmed stdout; stderr is captured
    /// too and included in the error, so diagnostics only show on failure.
    pub fn output_quiet(&self) -> Result<String> {
        tracing::debug!("+ {}", self.display());
        let output = self
            .command()
            .stdin(Stdio::null())
            .output()
            .with_context(|| format!("starting `{}`", self.display()))?;
        if !output.status.success() {
            return Err(anyhow!(
                "`{}` failed with {}: {}",
                self.display(),
                output.status,
                String::from_utf8_lossy(&output.stderr).trim()
            ));
        }
        Ok(String::from_utf8_lossy(&output.stdout)
            .trim_end()
            .to_string())
    }
}

fn shell_quote(s: &str) -> String {
    if !s.is_empty()
        && s.chars()
            .all(|c| c.is_ascii_alphanumeric() || "-_./=:@+,%".contains(c))
    {
        s.to_string()
    } else {
        format!("'{}'", s.replace('\'', "'\\''"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn display_quotes_only_when_needed() {
        let cmd = Cmd::new("podman")
            .args(["run", "--rm", "-e", "WORKRAVE_PPA_SERIES=stonking resolute"])
            .arg("it's");
        assert_eq!(
            cmd.display(),
            "podman run --rm -e 'WORKRAVE_PPA_SERIES=stonking resolute' 'it'\\''s'"
        );
    }

    #[test]
    fn display_masks_secrets() {
        redact("hunter2");
        let cmd = Cmd::new("curl").args(["-H", "Authorization: Bearer hunter2"]);
        assert_eq!(cmd.display(), "curl -H 'Authorization: Bearer ***'");
    }

    #[test]
    fn output_returns_trimmed_stdout() {
        let out = Cmd::new("sh")
            .args(["-c", "echo hello"])
            .output_quiet()
            .unwrap();
        assert_eq!(out, "hello");
    }

    #[test]
    fn failure_reports_command_line() {
        let err = Cmd::new("sh").args(["-c", "exit 3"]).run().unwrap_err();
        assert!(err.to_string().contains("`sh -c 'exit 3'` failed"), "{err}");
    }

    #[test]
    fn succeeds_does_not_fail_on_nonzero_exit() {
        assert!(!Cmd::new("sh").args(["-c", "exit 1"]).succeeds().unwrap());
        assert!(Cmd::new("sh").args(["-c", "exit 0"]).succeeds().unwrap());
    }
}
