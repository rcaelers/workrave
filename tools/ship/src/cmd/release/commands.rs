//! Per-step command files. SHIP_* and GITHUB_* refer to the same files.

use std::collections::BTreeMap;
use std::path::PathBuf;
use std::sync::atomic::{AtomicUsize, Ordering};

use anyhow::{bail, Context, Result};

#[derive(Default)]
pub struct Commands {
    pub outputs: BTreeMap<String, String>,
    pub env: BTreeMap<String, String>,
    pub paths: Vec<String>,
}

impl Commands {
    pub fn parse(output: &str, env: &str, path: &str) -> Result<Self> {
        let env = parse_values(env).context("reading SHIP_ENV")?;
        for name in env.keys() {
            let upper = name.to_ascii_uppercase();
            if upper.starts_with("SHIP_")
                || upper.starts_with("GITHUB_")
                || upper.starts_with("RUNNER_")
            {
                bail!("SHIP_ENV cannot replace reserved variable '{name}'");
            }
        }
        Ok(Self {
            outputs: parse_values(output).context("reading SHIP_OUTPUT")?,
            env,
            paths: path
                .strip_prefix('\u{feff}')
                .unwrap_or(path)
                .lines()
                .filter(|line| !line.is_empty())
                .map(String::from)
                .collect(),
        })
    }
}

/// GitHub environment-file syntax, including empty values and multiline delimiters.
/// Values are strings; whitespace and embedded newlines are significant.
pub fn parse_values(text: &str) -> Result<BTreeMap<String, String>> {
    let text = text.strip_prefix('\u{feff}').unwrap_or(text);
    let mut lines = text.split_inclusive('\n');
    let mut values = BTreeMap::new();
    while let Some(raw) = lines.next() {
        let line = line_text(raw);
        if line.is_empty() {
            continue;
        }
        let equals = line.find('=');
        let delimiter = line.find("<<");
        let (name, value) = if delimiter.is_some() && (equals.is_none() || delimiter < equals) {
            let (name, marker) = line.split_once("<<").unwrap();
            if marker.is_empty() {
                bail!("empty delimiter in command file");
            }
            let mut value = String::new();
            loop {
                let next = lines
                    .next()
                    .context("unterminated multiline command-file value")?;
                if line_text(next) == marker {
                    break;
                }
                if !next.ends_with('\n') {
                    bail!("unterminated multiline command-file value");
                }
                value.push_str(next);
            }
            if value.ends_with('\n') {
                value.pop();
                if value.ends_with('\r') {
                    value.pop();
                }
            }
            (name, value)
        } else if let Some((name, value)) = line.split_once('=') {
            (name, value.to_owned())
        } else {
            bail!("command-file line must be name=value or name<<delimiter");
        };
        if name.is_empty() || name.contains('\0') || value.contains('\0') {
            bail!("invalid command-file name or value");
        }
        values.insert(name.to_owned(), value);
    }
    Ok(values)
}

fn line_text(raw: &str) -> &str {
    raw.strip_suffix('\n')
        .unwrap_or(raw)
        .strip_suffix('\r')
        .unwrap_or(raw.strip_suffix('\n').unwrap_or(raw))
}

pub struct Files {
    pub local: PathBuf,
    pub guest: String,
}

impl Files {
    pub fn new(guest_root: Option<&str>) -> Result<Self> {
        static SERIAL: AtomicUsize = AtomicUsize::new(0);
        let serial = SERIAL.fetch_add(1, Ordering::Relaxed);
        let time = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)?
            .as_nanos();
        let name = format!("ship-commands-{}-{time}-{serial}", std::process::id());
        let local = std::env::temp_dir().join(&name);
        let mut builder = std::fs::DirBuilder::new();
        #[cfg(unix)]
        {
            use std::os::unix::fs::DirBuilderExt;
            builder.mode(0o700);
        }
        builder.create(&local)?;
        let files = Self {
            guest: guest_root
                .map(|root| format!("{}/{name}", root.trim_end_matches('/')))
                .unwrap_or_else(|| local.to_string_lossy().into_owned()),
            local,
        };
        for name in ["output", "env", "path"] {
            std::fs::write(files.local.join(name), "")?;
        }
        Ok(files)
    }

    pub fn env(&self, msys: bool) -> Vec<(String, String)> {
        let mut env = Vec::new();
        for name in ["OUTPUT", "ENV", "PATH"] {
            let path = format!("{}/{}", self.guest, name.to_ascii_lowercase());
            let path = if msys {
                super::context::msys_path(&path)
            } else {
                path
            };
            for prefix in ["SHIP", "GITHUB"] {
                env.push((format!("{prefix}_{name}"), path.clone()));
            }
        }
        env
    }

    pub fn read(&self) -> Result<Commands> {
        let read = |name: &str| {
            std::fs::read_to_string(self.local.join(name))
                .with_context(|| format!("reading step command file {name}"))
        };
        Commands::parse(&read("output")?, &read("env")?, &read("path")?)
    }

    pub fn initialize_sh(&self) -> String {
        format!(
            "mkdir -m 700 -p {0}; : > {0}/output; : > {0}/env; : > {0}/path; ",
            sh_quote(&self.guest)
        )
    }
}

impl Drop for Files {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.local);
    }
}

pub fn sh_quote(value: &str) -> String {
    format!("'{}'", value.replace('\'', "'\\''"))
}

/// Apply accumulated SHIP_PATH entries to the runner's PATH, never the controller's.
pub fn prepend_path(script: &str, shell: &str, paths: &[String], windows: bool) -> String {
    if paths.is_empty() {
        return script.to_owned();
    }
    let mut paths = paths.to_vec();
    paths.reverse();
    if matches!(shell, "powershell" | "pwsh") {
        let value = paths
            .join(if windows { ";" } else { ":" })
            .replace('\'', "''");
        format!(
            "$env:PATH = '{value}{}' + $env:PATH\n{script}",
            if windows { ";" } else { ":" }
        )
    } else {
        let paths: Vec<_> = paths
            .iter()
            .map(|p| {
                if windows {
                    super::context::msys_path(p)
                } else {
                    p.clone()
                }
            })
            .collect();
        format!(
            "export PATH={}:\"$PATH\"\n{script}",
            sh_quote(&paths.join(":"))
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn preserves_output_strings_and_multiline_values() {
        let values = parse_values("\u{feff}flag=false\r\nempty=\r\nspaced= x = y \r\nbody<<END\r\na\r\n\r\nb\r\nEND\r\nempty_body<<X\nX\nflag=true\n").unwrap();
        assert_eq!(values["flag"], "true");
        assert_eq!(values["empty"], "");
        assert_eq!(values["spaced"], " x = y ");
        assert_eq!(values["body"], "a\r\n\r\nb");
        assert_eq!(values["empty_body"], "");
        for bad in [
            "x",
            "=value",
            "x<<",
            "x<<EOF\nmissing\n",
            "x<<EOF\nmissing",
            "x<<EOF\nEOF ",
        ] {
            assert!(parse_values(bad).is_err(), "{bad}");
        }
    }

    #[test]
    fn aliases_share_files_and_each_step_gets_fresh_files() {
        let first = Files::new(None).unwrap();
        let second = Files::new(None).unwrap();
        assert_ne!(first.guest, second.guest);
        let env = first.env(false);
        for pair in env.chunks(2) {
            assert_eq!(pair[0].1, pair[1].1);
        }
        assert!(first.read().unwrap().outputs.is_empty());
        assert!(Commands::parse("", "SHIP_OUTPUT=bad", "").is_err());
        assert!(Commands::parse("", "GITHUB_ENV=bad", "").is_err());
    }
}
