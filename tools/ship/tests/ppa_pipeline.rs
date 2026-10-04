//! Check series selection and publishing options in the actual PPA pipeline.

use std::path::Path;
use std::process::Command;

fn ppa_commands(series: Option<&str>, flags: &[&str]) -> Vec<String> {
    let crate_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    let mut command = Command::new(env!("CARGO_BIN_EXE_ship"));
    command
        .args(["pipeline", "show", "--target", "linux", "--job", "ppa"])
        .arg("--pipeline")
        .arg(crate_dir.join("../local/release.yaml"))
        .arg("--config")
        .arg(crate_dir.join("../local/ship.example.yaml"))
        .args(["--ppa", "3"])
        .args(flags)
        .env_remove("SHIP_PROFILE");
    if let Some(series) = series {
        command
            .arg("--set")
            .arg(format!("config.linux.ppa_series={series}"));
    }
    let output = command.output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let plan = String::from_utf8(output.stdout).unwrap();
    assert!(plan.contains(":/workspace/debian"), "{plan}");
    assert_eq!(
        plan.matches("job container: podman run").count(),
        1,
        "{plan}"
    );
    let commands: Vec<_> = plan
        .lines()
        .filter(|line| line.contains("$ podman exec"))
        .collect();
    assert_eq!(
        commands
            .iter()
            .filter(|line| line.contains("/health"))
            .count(),
        1,
        "{plan}"
    );
    commands
        .into_iter()
        .filter(|line| line.contains("/workspace/scripts/local/ppa.sh"))
        .map(str::to_owned)
        .collect()
}

fn assert_series(commands: &[String], expected: &[&str]) {
    assert_eq!(commands.len(), expected.len(), "{commands:#?}");
    for (command, series) in commands.iter().zip(expected) {
        assert!(command.contains(&format!("DIST={series} ")), "{command}");
        assert!(!command.contains("WORKRAVE_PPA_SERIES"));
        assert!(command.contains("SIGNING_SERVICE_URL="));
        assert!(command.contains("/ppa.sh -p 3"));
    }
}

#[test]
fn runs_one_source_package_step_per_selected_series() {
    assert_series(&ppa_commands(None, &[]), &["stonking", "resolute", "noble"]);
    assert_series(&ppa_commands(Some(r#"["noble"]"#), &[]), &["noble"]);
    assert_series(&ppa_commands(Some("[]"), &[]), &[]);
}

#[test]
fn passes_dry_run_and_prerelease_flags_to_each_series() {
    let commands = ppa_commands(None, &["--dry-run", "--prerelease"]);
    assert_series(&commands, &["stonking", "resolute", "noble"]);
    for command in commands {
        assert!(command.contains("/ppa.sh -p 3 -d -P"), "{command}");
    }
}
