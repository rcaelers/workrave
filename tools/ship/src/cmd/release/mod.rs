//! `ship release`: run the release pipeline, and `ship pipeline show`: print
//! what it would run.
//!
//! The pipeline file is given with --pipeline; see `schema.rs` for the
//! file format, `context.rs` for the template context, `runner.rs` for the
//! execution and `actions/` for the builtin actions.
//!
//! The command line is parsed in two passes: the options selecting the
//! configuration and pipeline files first, then — with the pipeline loaded —
//! everything, including the options the pipeline declares itself.

pub mod actions;
pub mod context;
pub mod runner;
pub mod schema;

use std::ffi::OsString;
use std::path::PathBuf;

use anyhow::{bail, Result};
use clap::{Arg, ArgAction, ArgMatches, Command};

use crate::config::Config;
use schema::Pipeline;

/// `ship release [OPTIONS]`; the options are parsed here, see [`parse`].
#[derive(clap::Args, Debug)]
#[command(disable_help_flag = true)]
pub struct ReleaseCommand {
    #[arg(trailing_var_arg = true, allow_hyphen_values = true, hide = true)]
    args: Vec<OsString>,
}

/// `ship pipeline show [OPTIONS]`, with the same options as `ship release`.
#[derive(clap::Args, Debug)]
#[command(disable_help_flag = true)]
pub struct PipelineCommand {
    #[arg(trailing_var_arg = true, allow_hyphen_values = true, hide = true)]
    args: Vec<OsString>,
}

pub async fn run(args: ReleaseCommand) -> Result<()> {
    let (config, pipeline, options) = parse("ship release", &args.args, false)?;
    runner::run(config, pipeline, options).await
}

pub async fn run_pipeline(args: PipelineCommand) -> Result<()> {
    let mut rest = args.args.iter();
    match rest.next().and_then(|a| a.to_str()) {
        Some("show") => {
            let rest: Vec<OsString> = rest.cloned().collect();
            let (config, pipeline, options) = parse("ship pipeline show", &rest, true)?;
            runner::run(config, pipeline, options).await
        }
        Some("-h") | Some("--help") | None => {
            println!("Usage: ship pipeline show [OPTIONS]\n\nPrint the jobs and the commands a release would run, without running anything.\nSee `ship pipeline show --help` for the options.");
            Ok(())
        }
        Some(other) => bail!("unknown pipeline command '{other}' (expected: show)"),
    }
}

/// The engine's own options, and the pipeline's if one is loaded.
fn command(name: &'static str, pipeline: Option<&Pipeline>) -> Command {
    let mut command = Command::new(name)
        .no_binary_name(true)
        .disable_version_flag(true)
        .arg(
            Arg::new("config")
                .short('f')
                .long("config")
                .env("SHIP_CONFIG")
                .value_name("FILE")
                .help("Configuration file (default: ~/.config/ship/ship.yaml)"),
        )
        .arg(
            Arg::new("profile")
                .short('p')
                .long("profile")
                .env("SHIP_PROFILE")
                .value_name("NAME")
                .help("Profile from the configuration file to apply"),
        )
        .arg(
            Arg::new("pipeline")
                .long("pipeline")
                .env("SHIP_PIPELINE")
                .value_name("FILE")
                .help("Pipeline file (default: release.yaml in the current directory)"),
        )
        .arg(
            Arg::new("target")
                .short('T')
                .long("target")
                .value_name("NAME")
                .help("Target from the pipeline file (default: linux [+ macos on a Mac], or windows)"),
        )
        .arg(
            Arg::new("job")
                .short('j')
                .long("job")
                .value_name("NAME")
                .action(ArgAction::Append)
                .help("Run only these jobs of the target"),
        )
        .arg(
            Arg::new("skip-job")
                .long("skip-job")
                .value_name("NAME")
                .action(ArgAction::Append)
                .help("Skip these jobs"),
        )
        .arg(
            Arg::new("dry-run")
                .short('d')
                .long("dry-run")
                .action(ArgAction::SetTrue)
                .help("Build everything, but only print uploads, signing and other side effects"),
        )
        .arg(
            Arg::new("set")
                .long("set")
                .value_name("KEY=VALUE")
                .action(ArgAction::Append)
                .value_delimiter(',')
                .help("Set an option ({{ options.KEY }}), or config.KEY to override the configuration"),
        );

    if let Some(pipeline) = pipeline {
        if !pipeline.options.is_empty() {
            command = command.next_help_heading("Pipeline options");
        }
        for (name, spec) in &pipeline.options {
            let mut arg = Arg::new(name.clone()).long(name.clone());
            if let Some(short) = spec.short {
                arg = arg.short(short);
            }
            if let Some(help) = &spec.help {
                arg = arg.help(help.clone());
            }
            arg = if spec.flag {
                arg.action(ArgAction::SetTrue)
            } else {
                arg.value_name("VALUE")
            };
            command = command.arg(arg);
        }
    }
    command
}

/// Loads the configuration and the pipeline named by `args`, then parses
/// `args` against the pipeline's options too.
fn parse(
    name: &'static str,
    args: &[OsString],
    show: bool,
) -> Result<(Config, Pipeline, runner::RunOptions)> {
    // Pass 1: only the file-selecting options matter; the rest (including
    // the pipeline's own options) is parsed once the pipeline is known.
    let first = FileOptions::scan(args);
    let config = Config::load(
        first.config.as_deref().map(std::path::Path::new),
        first.profile.as_deref(),
    )?;
    let pipeline_path = first
        .pipeline
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("release.yaml"));
    tracing::info!("Using pipeline file {}", pipeline_path.display());
    let pipeline = Pipeline::load(&pipeline_path)?;

    // Pass 2: everything, with proper errors and --help.
    let matches = command(name, Some(&pipeline))
        .try_get_matches_from(args.iter().cloned())
        .unwrap_or_else(|e| e.exit());
    let options = run_options(&matches, &pipeline, show)?;
    Ok((config, pipeline, options))
}

/// The options that select the configuration and pipeline files, found by
/// a plain scan of the command line (before the pipeline's options are
/// known) — `--config FILE`, `--config=FILE`, `-f FILE`, `-fFILE` and the
/// environment variables.
#[derive(Debug, Default, PartialEq)]
struct FileOptions {
    config: Option<String>,
    profile: Option<String>,
    pipeline: Option<String>,
}

impl FileOptions {
    fn scan(args: &[OsString]) -> FileOptions {
        let mut found = FileOptions {
            config: std::env::var("SHIP_CONFIG").ok().filter(|s| !s.is_empty()),
            profile: std::env::var("SHIP_PROFILE").ok().filter(|s| !s.is_empty()),
            pipeline: std::env::var("SHIP_PIPELINE")
                .ok()
                .filter(|s| !s.is_empty()),
        };
        let args: Vec<String> = args
            .iter()
            .map(|a| a.to_string_lossy().into_owned())
            .collect();
        let mut i = 0;
        while i < args.len() {
            let arg = &args[i];
            let next = args.get(i + 1).cloned();
            let take = |value: Option<String>, slot: &mut Option<String>| {
                if let Some(v) = value {
                    *slot = Some(v);
                }
            };
            match arg.as_str() {
                "--config" | "-f" => {
                    take(next, &mut found.config);
                    i += 1;
                }
                "--profile" | "-p" => {
                    take(next, &mut found.profile);
                    i += 1;
                }
                "--pipeline" => {
                    take(next, &mut found.pipeline);
                    i += 1;
                }
                a if a.starts_with("--config=") => {
                    take(Some(a["--config=".len()..].to_string()), &mut found.config)
                }
                a if a.starts_with("--profile=") => take(
                    Some(a["--profile=".len()..].to_string()),
                    &mut found.profile,
                ),
                a if a.starts_with("--pipeline=") => take(
                    Some(a["--pipeline=".len()..].to_string()),
                    &mut found.pipeline,
                ),
                a if a.starts_with("-f") && a.len() > 2 => {
                    take(Some(a[2..].to_string()), &mut found.config)
                }
                a if a.starts_with("-p") && a.len() > 2 => {
                    take(Some(a[2..].to_string()), &mut found.profile)
                }
                _ => {}
            }
            i += 1;
        }
        found
    }
}

fn run_options(
    matches: &ArgMatches,
    pipeline: &Pipeline,
    show: bool,
) -> Result<runner::RunOptions> {
    let strings = |id: &str| -> Vec<String> {
        matches
            .get_many::<String>(id)
            .map(|v| v.cloned().collect())
            .unwrap_or_default()
    };

    let mut options = serde_json::Map::new();
    let mut config_overrides = Vec::new();
    for (name, spec) in &pipeline.options {
        let given = if spec.flag {
            matches.get_flag(name).then(|| match &spec.value {
                Some(v) => serde_json::to_value(v).unwrap_or(serde_json::Value::Bool(true)),
                None => serde_json::Value::Bool(true),
            })
        } else {
            matches
                .get_one::<String>(name)
                .map(|v| context::parse_output_value(v))
        };
        if let Some(value) = given {
            match &spec.config {
                Some(key) => config_overrides.push((key.clone(), value)),
                None => {
                    options.insert(name.clone(), value);
                }
            }
        }
    }
    for assignment in strings("set") {
        let Some((key, value)) = assignment.split_once('=') else {
            bail!("--set expects KEY=VALUE, got '{assignment}'");
        };
        let value = context::parse_output_value(value);
        match key.strip_prefix("config.") {
            Some(config_key) => config_overrides.push((config_key.to_string(), value)),
            None => {
                options.insert(key.to_string(), value);
            }
        }
    }

    Ok(runner::RunOptions {
        target: matches.get_one::<String>("target").cloned(),
        jobs: strings("job"),
        skip_jobs: strings("skip-job"),
        dry_run: matches.get_flag("dry-run"),
        show,
        options,
        config_overrides,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    fn pipeline() -> Pipeline {
        Pipeline::parse(
            "options:\n  commit: {short: t, help: x}\n  prerelease: {flag: true, short: P, config: prerelease}\n  no-sign: {flag: true, config: windows.sign, value: false}\n  ppa: {config: linux.ppa_increment}\n  staging: {flag: true}\njobs: {}\n",
        )
        .unwrap()
    }

    #[test]
    fn pipeline_options_and_set_values() {
        let p = pipeline();
        let matches = command("ship release", Some(&p))
            .try_get_matches_from([
                "-t",
                "abc",
                "-P",
                "--no-sign",
                "--ppa",
                "2",
                "--set",
                "extra.x=true",
                "--set",
                "config.linux.image=x",
                "-d",
            ])
            .unwrap();
        let opts = run_options(&matches, &p, false).unwrap();
        assert_eq!(opts.options["commit"], serde_json::json!("abc"));
        assert_eq!(opts.options["extra.x"], serde_json::json!(true));
        assert!(opts.options.get("prerelease").is_none());
        assert!(opts.options.get("staging").is_none());
        assert_eq!(
            opts.config_overrides,
            vec![
                ("prerelease".to_string(), serde_json::json!(true)),
                ("windows.sign".to_string(), serde_json::json!(false)),
                ("linux.ppa_increment".to_string(), serde_json::json!("2")),
                ("linux.image".to_string(), serde_json::json!("x")),
            ]
        );
        assert!(opts.dry_run);
    }

    #[test]
    fn unknown_options_are_errors_in_pass_two() {
        let p = pipeline();
        assert!(command("ship release", Some(&p))
            .try_get_matches_from(["--nope"])
            .is_err());
        // But ignored by the file scan.
        let args: Vec<OsString> = [
            "-t",
            "abc",
            "--pipeline",
            "x.yaml",
            "-P",
            "-fcfg.yaml",
            "--profile=docker",
        ]
        .iter()
        .map(OsString::from)
        .collect();
        let first = FileOptions::scan(&args);
        assert_eq!(first.pipeline.as_deref(), Some("x.yaml"));
        assert_eq!(first.config.as_deref(), Some("cfg.yaml"));
        assert_eq!(first.profile.as_deref(), Some("docker"));
    }

    #[test]
    fn help_lists_pipeline_options() {
        let p = pipeline();
        let help = command("ship release", Some(&p)).render_help().to_string();
        assert!(help.contains("Pipeline options"), "{help}");
        assert!(help.contains("-t, --commit <VALUE>"), "{help}");
        assert!(help.contains("-P, --prerelease"), "{help}");
    }
}
