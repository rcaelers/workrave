//! A Windows Docker job, reached through SSH or run on the local Windows host.
//!
//! VM management is supplied by configuration hooks. Inputs are copied into
//! the container layer; all steps share that layer. Only declared outputs are
//! collected, including after a failed step.

use std::fs;
use std::io::Read;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::{SystemTime, UNIX_EPOCH};

use anyhow::{bail, Context, Result};
use base64::{engine::general_purpose::STANDARD, Engine};
use sha2::{Digest, Sha256};

use super::process::Cmd;

#[derive(Debug, Clone)]
pub struct Settings {
    pub ssh: String,
    pub start: String,
    pub stop: String,
    pub docker: String,
    pub image: String,
    pub context: Option<PathBuf>,
    pub init: String,
    pub options: Vec<String>,
    pub copy: Vec<(PathBuf, String)>,
    pub collect: Vec<(String, PathBuf)>,
}

pub struct Session {
    settings: Settings,
    name: String,
    guest: String,
    temporary: PathBuf,
    serial: AtomicUsize,
    started: bool,
    created: bool,
}

impl Session {
    pub fn new(settings: Settings) -> Result<Self> {
        if settings.ssh.is_empty() && !cfg!(windows) {
            bail!("a Windows container needs an SSH destination when Ship runs on macOS/Linux");
        }
        let suffix = SystemTime::now().duration_since(UNIX_EPOCH)?.as_nanos();
        let name = format!("ship-{}-{suffix}", std::process::id());
        let temporary = std::env::temp_dir().join(&name);
        fs::create_dir_all(&temporary)?;
        Ok(Self {
            guest: format!("C:/ship/{name}"),
            settings,
            name,
            temporary,
            serial: AtomicUsize::new(0),
            started: false,
            created: false,
        })
    }

    pub fn prepare(&mut self) -> Result<()> {
        self.started = true;
        if !self.settings.start.is_empty() {
            hook(&self.settings.start).run()?;
        }
        // The start hook must return with SSH ready. No hypervisor assumptions.
        self.bootstrap()?.run()?;
        self.prepare_image()?;
        let mut args = vec![
            "create".into(),
            "--isolation=process".into(),
            "--name".into(),
            self.name.clone(),
        ];
        args.extend(self.settings.options.clone());
        args.extend([
            "--entrypoint".into(),
            "powershell.exe".into(),
            self.settings.image.clone(),
            "-NoProfile".into(),
            "-NonInteractive".into(),
            "-Command".into(),
            "while ($true) { Start-Sleep -Seconds 3600 }".into(),
        ]);
        self.docker(&args, false)?.run()?;
        self.created = true;
        self.docker(&["start".into(), self.name.clone()], false)?
            .run()?;
        for (index, (source, destination)) in self.settings.copy.iter().enumerate() {
            self.copy_input(source, destination, index)?;
        }
        Ok(())
    }

    fn bootstrap(&self) -> Result<Cmd> {
        let script = format!(
            "$ErrorActionPreference='Stop'; New-Item -ItemType Directory -Force {} | Out-Null",
            ps_quote(&self.guest)
        );
        Ok(self.powershell_encoded(&script))
    }

    fn powershell_encoded(&self, script: &str) -> Cmd {
        let args = [
            "powershell.exe",
            "-NoProfile",
            "-NonInteractive",
            "-EncodedCommand",
        ];
        let encoded = encode(script);
        if self.settings.ssh.is_empty() {
            Cmd::new(args[0]).args(&args[1..]).arg(encoded)
        } else {
            let command = format!("{} {}", args.join(" "), encoded);
            Cmd::new("ssh").args(["-o", "BatchMode=yes", &self.settings.ssh, &command])
        }
    }

    /// Transfer a script rather than nesting encoded commands in SSH's command
    /// line: Windows' default SSH shell has an 8191-character command limit.
    fn powershell(&self, script: &str, cleanup: bool) -> Result<Cmd> {
        let serial = self.serial.fetch_add(1, Ordering::Relaxed);
        let file = self.temporary.join(format!("command-{serial}.ps1"));
        fs::write(
            &file,
            format!(
                "\u{feff}$ErrorActionPreference='Stop'\n$ProgressPreference='SilentlyContinue'\n[Console]::OutputEncoding=New-Object Text.UTF8Encoding $false\n{script}\n"
            ),
        )?;
        if self.settings.ssh.is_empty() {
            Ok(Cmd::new("powershell.exe")
                .args([
                    "-NoProfile",
                    "-NonInteractive",
                    "-ExecutionPolicy",
                    "Bypass",
                    "-File",
                ])
                .arg(file))
        } else {
            let guest = format!("{}/command-{serial}.ps1", self.guest);
            let transfer = self.scp_to(&file, &guest);
            if cleanup {
                transfer.run_cleanup()?;
            } else {
                transfer.run()?;
            }
            Ok(Cmd::new("ssh").args([
                "-o",
                "BatchMode=yes",
                &self.settings.ssh,
                &format!(
                    "powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File {}",
                    guest
                ),
            ]))
        }
    }

    fn scp_to(&self, source: &Path, destination: &str) -> Cmd {
        Cmd::new("scp")
            .args(["-q", "-o", "BatchMode=yes"])
            .arg(source)
            .arg(format!("{}:{destination}", self.settings.ssh))
    }

    fn push(&self, source: &Path, destination: &str) -> Result<()> {
        if self.settings.ssh.is_empty() {
            fs::copy(source, destination)?;
        } else {
            self.scp_to(source, destination).run()?;
        }
        Ok(())
    }

    fn pull(&self, source: &str, destination: &Path) -> Result<()> {
        if self.settings.ssh.is_empty() {
            fs::copy(source, destination)?;
        } else {
            Cmd::new("scp")
                .args(["-q", "-o", "BatchMode=yes"])
                .arg(format!("{}:{source}", self.settings.ssh))
                .arg(destination)
                .run_cleanup()?;
        }
        Ok(())
    }

    fn docker(&self, args: &[String], cleanup: bool) -> Result<Cmd> {
        let command = args
            .iter()
            .map(|s| windows_quote(s))
            .collect::<Vec<_>>()
            .join(" ");
        let script = format!("$p=New-Object System.Diagnostics.Process\n$p.StartInfo.UseShellExecute=$false\n$p.StartInfo.FileName={}\n$p.StartInfo.Arguments={}\n[void]$p.Start()\n$p.WaitForExit()\nexit $p.ExitCode", ps_quote(&self.settings.docker), ps_quote(&command));
        self.powershell(&script, cleanup)
    }

    fn exec(
        &self,
        script: &str,
        env: &[(String, String)],
        cwd: Option<&Path>,
        shell: &str,
        cleanup: bool,
    ) -> Result<Cmd> {
        let mut args = vec!["exec".into()];
        for (key, value) in env {
            args.extend(["--env".into(), format!("{key}={value}")]);
        }
        if let Some(cwd) = cwd {
            args.extend(["--workdir".into(), cwd.to_string_lossy().replace('\\', "/")]);
        }
        args.push(self.name.clone());
        match shell {
            "bash" | "sh" => args.extend([
                format!("C:/msys64/usr/bin/{shell}.exe"),
                "-lc".into(),
                format!(
                    "set -e{}\n{}\n{script}",
                    if shell == "bash" { "o pipefail" } else { "" },
                    self.settings.init
                ),
            ]),
            "powershell" | "pwsh" => {
                let assignments = env
                    .iter()
                    .map(|(key, value)| {
                        format!(
                            "[Environment]::SetEnvironmentVariable({}, {})",
                            ps_quote(key),
                            ps_quote(value)
                        )
                    })
                    .collect::<Vec<_>>()
                    .join("\n");
                let script = format!("$ErrorActionPreference='Stop'\n$LASTEXITCODE=0\n{assignments}\n{}\n{script}\nif ($LASTEXITCODE) {{ exit $LASTEXITCODE }}", self.settings.init);
                let encoded = encode(&script);
                args.extend([
                    format!("{shell}.exe"),
                    "-NoProfile".into(),
                    "-NonInteractive".into(),
                ]);
                // CreateProcess has a 32767-character command limit even when
                // SSH's shorter shell limit is avoided. Stage large steps.
                if encoded.len() > 16000 {
                    let serial = self.serial.fetch_add(1, Ordering::Relaxed);
                    let file = self.temporary.join(format!("step-{serial}.ps1"));
                    fs::write(&file, format!("\u{feff}{script}"))?;
                    let guest = format!("{}/step-{serial}.ps1", self.guest);
                    let target = format!("C:/ship-step-{serial}.ps1");
                    self.push(&file, &guest)?;
                    let copy = self.docker(
                        &["cp".into(), guest, format!("{}:{target}", self.name)],
                        cleanup,
                    )?;
                    if cleanup {
                        copy.run_cleanup()?;
                    } else {
                        copy.run()?;
                    }
                    args.extend([
                        "-ExecutionPolicy".into(),
                        "Bypass".into(),
                        "-File".into(),
                        target,
                    ]);
                } else {
                    args.extend(["-EncodedCommand".into(), encoded]);
                }
            }
            other => bail!("unknown Windows container shell '{other}'"),
        }
        self.docker(&args, cleanup)
    }

    fn container_powershell(&self, script: &str, cleanup: bool) -> Result<Cmd> {
        self.docker(
            &[
                "exec".into(),
                self.name.clone(),
                "powershell.exe".into(),
                "-NoProfile".into(),
                "-NonInteractive".into(),
                "-EncodedCommand".into(),
                encode(script),
            ],
            cleanup,
        )
    }

    pub fn run(
        &self,
        script: &str,
        env: &[(String, String)],
        cwd: Option<&Path>,
        shell: &str,
        echo: bool,
    ) -> Result<()> {
        self.exec(script, env, cwd, shell, false)?.run_or_echo(echo)
    }

    pub fn prepare_command_files(&self, directory: &str) -> Result<()> {
        let directory = ps_quote(directory);
        self.container_powershell(&format!("$ErrorActionPreference='Stop'; New-Item -ItemType Directory -Force {directory} | Out-Null; foreach ($name in @('output', 'env', 'path')) {{ [IO.File]::WriteAllText(({directory} + '/' + $name), '', (New-Object Text.UTF8Encoding $false)) }}"), false)?.run()
    }

    pub fn command_files(
        &self,
        directory: &str,
    ) -> Result<std::collections::BTreeMap<String, String>> {
        let directory = ps_quote(directory);
        let json = self.container_powershell(&format!("$ErrorActionPreference='Stop'; [Console]::OutputEncoding=New-Object Text.UTF8Encoding $false; $files=@{{}}; foreach ($name in @('output', 'env', 'path')) {{ $files[$name]=[IO.File]::ReadAllText(({directory} + '/' + $name)) }}; $files | ConvertTo-Json -Compress; Remove-Item -Recurse -Force {directory}"), false)?.output_quiet()?;
        serde_json::from_str(&json).context("reading Windows step command files")
    }

    fn prepare_image(&self) -> Result<()> {
        let Some(context) = &self.settings.context else {
            return Ok(());
        };
        let hash = context_hash(context)?;
        let inspect = self
            .docker(
                &[
                    "image".into(),
                    "inspect".into(),
                    self.settings.image.clone(),
                ],
                false,
            )?
            .output_quiet();
        if let Ok(json) = inspect {
            let image: serde_json::Value = serde_json::from_str(&json)?;
            if image[0]["Config"]["Labels"]["org.workrave.sdk-context"].as_str() == Some(&hash) {
                return Ok(());
            }
        }
        let archive = self.temporary.join("image.tar.gz");
        Cmd::new("tar")
            .env("COPYFILE_DISABLE", "1")
            .arg("-czf")
            .arg(&archive)
            .arg("-C")
            .arg(context)
            .arg(".")
            .run()?;
        let guest = format!("{}/image.tar.gz", self.guest);
        self.push(&archive, &guest)?;
        self.powershell(&format!("New-Item -ItemType Directory -Force '{0}/image' | Out-Null; & tar.exe -xzf '{0}/image.tar.gz' -C '{0}/image'; if ($LASTEXITCODE) {{ exit $LASTEXITCODE }}", self.guest), false)?.run()?;
        self.docker(
            &[
                "build".into(),
                "--isolation=process".into(),
                "--tag".into(),
                self.settings.image.clone(),
                "--label".into(),
                format!("org.workrave.sdk-context={hash}"),
                format!("{}/image", self.guest),
            ],
            false,
        )?
        .run()
    }

    fn copy_input(&self, source: &Path, destination: &str, index: usize) -> Result<()> {
        let source = source
            .canonicalize()
            .with_context(|| format!("input {}", source.display()))?;
        let guest = format!("{}/input-{index}", self.guest);
        if source.is_file() {
            self.push(&source, &guest)?;
            let parent = destination
                .rsplit_once('/')
                .map(|(parent, _)| parent)
                .context("copy destination needs a parent directory")?;
            self.container_powershell(
                &format!(
                    "New-Item -ItemType Directory -Force {} | Out-Null",
                    ps_quote(parent)
                ),
                false,
            )?
            .run()?;
            return self
                .docker(
                    &["cp".into(), guest, format!("{}:{destination}", self.name)],
                    false,
                )?
                .run();
        }
        let archive = self.temporary.join(format!("input-{index}.tar.gz"));
        let mut tar = Cmd::new("tar")
            // BSD tar otherwise emits AppleDouble companions for macOS xattrs.
            // Other tar implementations ignore this environment variable.
            .env("COPYFILE_DISABLE", "1")
            .arg("-czf")
            .arg(&archive)
            .arg("-C")
            .arg(&source);
        // Include working changes, exclude ignored build trees. A release
        // checkout's Git history is needed by CMake and git archive.
        let git = Cmd::new("git")
            .arg("-C")
            .arg(&source)
            .args([
                "ls-files",
                "-z",
                "--cached",
                "--others",
                "--exclude-standard",
            ])
            .output_quiet();
        if let Ok(files) = git {
            let mut names: Vec<&str> = files
                .split('\0')
                .filter(|p| !p.is_empty() && source.join(p).exists())
                .collect();
            if source.join(".git").is_dir() {
                names.push(".git");
            }
            let listing = self.temporary.join(format!("files-{index}"));
            fs::write(&listing, names.join("\0") + "\0")?;
            tar = tar.arg("--null").arg("-T").arg(listing);
        } else {
            tar = tar.arg(".");
        }
        tar.run()?;
        self.push(&archive, &guest)?;
        self.container_powershell(
            &format!(
                "New-Item -ItemType Directory -Force {0} | Out-Null",
                ps_quote(destination)
            ),
            false,
        )?
        .run()?;
        self.docker(
            &[
                "cp".into(),
                guest,
                format!("{}:C:/ship-input.tar.gz", self.name),
            ],
            false,
        )?
        .run()?;
        self.container_powershell(&format!("& tar.exe -xzf C:/ship-input.tar.gz -C {}; if ($LASTEXITCODE) {{ exit $LASTEXITCODE }}", ps_quote(destination)), false)?.run()
    }

    fn collect(&self) -> Result<()> {
        for (index, (source, destination)) in self.settings.collect.iter().enumerate() {
            let probe = format!("if (Test-Path {0} -PathType Container) {{ Write-Output 'directory' }} elseif (Test-Path {0}) {{ throw 'collect requires a directory' }}", ps_quote(source));
            if self
                .container_powershell(&probe, true)?
                .output_quiet()?
                .trim()
                != "directory"
            {
                continue;
            }
            let guest = format!("{}/output-{index}", self.guest);
            self.docker(
                &[
                    "cp".into(),
                    format!("{}:{source}", self.name),
                    guest.clone(),
                ],
                true,
            )?
            .run_cleanup()?;
            let archive = format!("{guest}.tar.gz");
            self.powershell(
                &format!(
                    "& tar.exe -czf {} -C {} .; if ($LASTEXITCODE) {{ exit $LASTEXITCODE }}",
                    ps_quote(&archive),
                    ps_quote(&guest)
                ),
                true,
            )?
            .run_cleanup()?;
            let local = self.temporary.join(format!("output-{index}.tar.gz"));
            self.pull(&archive, &local)?;
            fs::create_dir_all(destination)?;
            Cmd::new("tar")
                .arg("-xzf")
                .arg(local)
                .arg("-C")
                .arg(destination)
                .run_cleanup()?;
        }
        Ok(())
    }

    pub fn finish(&self) -> Result<()> {
        let mut result = if self.created { self.collect() } else { Ok(()) };
        if self.created {
            if let Err(error) = self
                .docker(&["rm".into(), "--force".into(), self.name.clone()], true)
                .and_then(|cmd| cmd.run_cleanup())
            {
                tracing::warn!("removing Windows container: {error:#}");
                if result.is_ok() {
                    result = Err(error.context("removing Windows container"));
                }
            }
        }
        if self.started {
            if let Err(error) = self
                .powershell(
                    &format!(
                        "Remove-Item -Recurse -Force {} -ErrorAction SilentlyContinue",
                        ps_quote(&self.guest)
                    ),
                    true,
                )
                .and_then(|cmd| cmd.run_cleanup())
            {
                tracing::warn!("removing remote inputs: {error:#}");
                if result.is_ok() {
                    result = Err(error.context("removing remote inputs"));
                }
            }
            if !self.settings.stop.is_empty() {
                if let Err(error) = hook(&self.settings.stop).run_cleanup() {
                    tracing::warn!("stopping Windows environment: {error:#}");
                    if result.is_ok() {
                        result = Err(error.context("stopping Windows environment"));
                    }
                }
            }
        }
        result
    }
}

impl Drop for Session {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.temporary);
    }
}

fn hook(script: &str) -> Cmd {
    if cfg!(windows) {
        Cmd::new("powershell.exe").args(["-NoProfile", "-Command", script])
    } else {
        Cmd::new("bash").args(["-e", "-c", script])
    }
}

pub fn encode(script: &str) -> String {
    let bytes: Vec<u8> = script.encode_utf16().flat_map(u16::to_le_bytes).collect();
    STANDARD.encode(bytes)
}

pub fn ps_quote(value: &str) -> String {
    format!("'{}'", value.replace('\'', "''"))
}

fn windows_quote(value: &str) -> String {
    let mut out = String::from("\"");
    let mut slashes = 0;
    for ch in value.chars() {
        if ch == '\\' {
            slashes += 1;
            continue;
        }
        out.push_str(&"\\".repeat(if ch == '"' { slashes * 2 + 1 } else { slashes }));
        out.push(ch);
        slashes = 0;
    }
    out.push_str(&"\\".repeat(slashes * 2));
    out.push('"');
    out
}

pub fn file_hash(path: &Path) -> Result<String> {
    let mut file = fs::File::open(path)?;
    let mut digest = Sha256::new();
    let mut buffer = [0u8; 65536];
    loop {
        let size = file.read(&mut buffer)?;
        if size == 0 {
            break;
        }
        digest.update(&buffer[..size]);
    }
    Ok(digest
        .finalize()
        .iter()
        .map(|byte| format!("{byte:02x}"))
        .collect())
}

fn context_hash(root: &Path) -> Result<String> {
    fn walk(root: &Path, result: &mut Vec<PathBuf>) -> Result<()> {
        for entry in fs::read_dir(root)? {
            let path = entry?.path();
            if path.is_dir() {
                walk(&path, result)?;
            } else {
                result.push(path);
            }
        }
        Ok(())
    }
    let mut files = Vec::new();
    walk(root, &mut files)?;
    files.sort_by_key(|p| {
        p.strip_prefix(root)
            .unwrap()
            .to_string_lossy()
            .replace('\\', "/")
    });
    let mut hash = Sha256::new();
    for file in files {
        hash.update(
            file.strip_prefix(root)?
                .to_string_lossy()
                .replace('\\', "/")
                .as_bytes(),
        );
        hash.update(fs::read(file)?);
    }
    Ok(hash
        .finalize()
        .iter()
        .map(|byte| format!("{byte:02x}"))
        .collect())
}

pub fn show(settings: &Settings) {
    println!(
        "   Windows container: {} (SSH: {})",
        settings.image,
        if settings.ssh.is_empty() {
            "local Windows host"
        } else {
            &settings.ssh
        }
    );
    if !settings.start.is_empty() {
        println!("   start: {}", settings.start);
    }
    for (local, guest) in &settings.copy {
        println!("   copy {} -> {guest}", local.display());
    }
    if !settings.stop.is_empty() {
        println!("   stop after cleanup: {}", settings.stop);
    }
}

pub fn show_step(script: &str, env: &[(String, String)], shell: &str) {
    println!("   $ ({shell}) {script}");
    for (key, value) in env {
        println!("       {key}={}", super::process::redacted(value));
    }
}
