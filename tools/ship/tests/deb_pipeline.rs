//! Check series selection in the actual Workrave binary-package pipeline.

use std::path::Path;
use std::process::Command;

fn deb_plan(series: Option<&str>, enabled: bool) -> String {
    let crate_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    let mut command = Command::new(env!("CARGO_BIN_EXE_ship"));
    command
        .args(["pipeline", "show", "--target", "linux", "--job", "deb"])
        .arg("--pipeline")
        .arg(crate_dir.join("../local/release.yaml"))
        .arg("--config")
        .arg(crate_dir.join("../local/ship.example.yaml"))
        .arg("--set")
        .arg(format!("config.linux.build_deb={enabled}"))
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
    String::from_utf8(output.stdout).unwrap()
}

fn assert_series(plan: &str, expected: &[&str]) {
    let commands: Vec<_> = plan
        .lines()
        .filter(|line| {
            line.contains("$ podman exec") && line.contains("/workspace/scripts/local/pbuild.sh")
        })
        .collect();
    assert_eq!(commands.len(), expected.len(), "{plan}");
    for (command, series) in commands.iter().zip(expected) {
        assert!(command.contains(&format!("DIST={series} ")), "{command}");
        assert!(command.contains("/workspace/scripts/local/pbuild.sh"));
        assert!(!command.contains("WORKRAVE_PPA_SERIES"));
    }
}

#[test]
fn runs_a_separate_step_for_each_configured_series() {
    assert_series(&deb_plan(None, true), &["stonking", "resolute", "noble"]);
    assert_series(&deb_plan(Some(r#"["noble"]"#), true), &["noble"]);
}

#[test]
fn does_not_build_when_disabled_or_no_series_are_selected() {
    assert_series(&deb_plan(None, false), &[]);
    assert_series(&deb_plan(Some("[]"), true), &[]);
}
