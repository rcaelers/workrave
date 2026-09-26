//! Exercise reporting through the CLI, including exit status and stderr.
//! These fixtures use the host's bash, just like host pipeline steps.
#![cfg(unix)]

use std::process::{Command, Output};
use std::sync::atomic::{AtomicUsize, Ordering};

fn run(pipeline: &str, args: &[&str]) -> Output {
    static NEXT: AtomicUsize = AtomicUsize::new(0);
    let dir = std::env::temp_dir().join(format!(
        "ship-report-test-{}-{}",
        std::process::id(),
        NEXT.fetch_add(1, Ordering::Relaxed)
    ));
    std::fs::create_dir_all(&dir).unwrap();
    std::fs::write(dir.join("config.yaml"), "{}").unwrap();
    std::fs::write(dir.join("pipeline.yaml"), pipeline).unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(args)
        .args(["--target", "test", "--config"])
        .arg(dir.join("config.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        // The final report must still be visible when info logs are disabled.
        .env("RUST_LOG", "off")
        .output();
    std::fs::remove_dir_all(dir).unwrap();
    output.unwrap()
}

fn stderr(output: &Output) -> String {
    String::from_utf8(output.stderr.clone()).unwrap()
}

#[test]
fn reports_step_times_matrix_iterations_and_skips() {
    let output = run(
        r#"
targets: {test: [build, skip-job]}
jobs:
  build:
    strategy: {matrix: {flavor: [debug, release]}}
    steps:
      - name: compile
        run: sleep 0.02
      - uses: check-config
        with: {required: []}
        foreach: '{{ ["first", "second"] }}'
      - name: skip-step
        if: 'false'
        run: exit 10
      - name: empty-loop
        foreach: '{{ [] }}'
        run: exit 11
  skip-job:
    if: 'false'
    steps: [{run: 'exit 12'}]
"#,
        &["release"],
    );
    let report = stderr(&output);
    assert!(output.status.success(), "{report}");
    assert_eq!(report.matches("Build report").count(), 1);
    assert!(report.contains("build [flavor=debug]"), "{report}");
    assert!(report.contains("build [flavor=release]"), "{report}");
    assert_eq!(report.matches("check-config [item 1]").count(), 2);
    assert_eq!(report.matches("check-config [item 2]").count(), 2);
    for line in report.lines().filter(|line| line.contains("compile")) {
        let fields: Vec<_> = line.split_whitespace().collect();
        assert_eq!(fields[0], "ok");
        let seconds: f64 = fields[1].trim_end_matches('s').parse().unwrap();
        assert!(seconds >= 0.02, "{line}");
    }
    assert_eq!(report.matches("compile").count(), 2);
    for label in ["skip-step", "empty-loop", "skip-job"] {
        let line = report.lines().find(|line| line.contains(label)).unwrap();
        assert_eq!(
            line.split_whitespace().take(2).collect::<Vec<_>>(),
            ["skipped", "-"]
        );
    }
    assert!(
        report.contains("Total: ") && report.ends_with("(ok)\n"),
        "{report}"
    );
}

#[test]
fn reports_completed_and_failed_steps_before_returning_the_error() {
    let output = run(
        r#"
targets: {test: [build]}
jobs:
  build:
    steps:
      - name: prepare
        run: 'echo version.tag=v1 >> "$SHIP_OUTPUT"'
        outputs: [version.tag]
      - name: compile
        run: 'test "{{ version.tag }}" = v1 && exit 7'
      - name: unreached
        run: exit 8
"#,
        &["release"],
    );
    let report = stderr(&output);
    assert!(!output.status.success());
    let prepare = report
        .lines()
        .find(|line| line.contains("1. prepare"))
        .unwrap();
    assert!(prepare.starts_with("ok"), "{report}");
    let compile = report
        .lines()
        .find(|line| line.contains("2. compile"))
        .unwrap();
    assert!(compile.starts_with("failed"), "{report}");
    assert!(!report.contains("unreached"), "{report}");
    assert!(report.contains("(failed)"), "{report}");
    assert!(report.contains("in compile of job build"), "{report}");
    assert!(report.contains("exit status: 7"), "{report}");
}

#[test]
fn reports_errors_before_command_execution() {
    for step in [
        "{name: broken, if: '{{ missing.value }}', run: 'exit 9'}",
        "{name: broken, foreach: '{{ missing.value }}', run: 'exit 9'}",
        "{name: broken, run: '{{ missing.value }}'}",
        "{name: broken, run: 'true', outputs: [missing]}",
    ] {
        let output = run(
            &format!("targets: {{test: [build]}}\njobs:\n  build:\n    steps: [{step}]\n"),
            &["release"],
        );
        let report = stderr(&output);
        assert!(!output.status.success(), "{step}\n{report}");
        let line = report
            .lines()
            .find(|line| line.contains("1. broken"))
            .unwrap();
        assert!(line.starts_with("failed"), "{report}");
        assert!(report.contains("(failed)"), "{report}");
    }
}

const DRY_RUN_PIPELINE: &str = r#"
targets: {test: [build]}
jobs:
  build:
    steps:
      - name: compile
        run: 'true'
      - name: publish
        run: 'exit 17'
        dry-run: echo
"#;

#[test]
fn dry_run_distinguishes_executed_and_echoed_steps() {
    let output = run(DRY_RUN_PIPELINE, &["release", "--dry-run"]);
    let report = stderr(&output);
    assert!(output.status.success(), "{report}");
    assert!(report.contains("Build report (dry run)"), "{report}");
    let compile = report
        .lines()
        .find(|line| line.contains("1. compile"))
        .unwrap();
    let publish = report
        .lines()
        .find(|line| line.contains("2. publish"))
        .unwrap();
    assert!(compile.starts_with("ok"), "{report}");
    assert!(publish.starts_with("echoed"), "{report}");
}

#[test]
fn pipeline_show_has_no_timing_report() {
    let output = run(DRY_RUN_PIPELINE, &["pipeline", "show"]);
    assert!(output.status.success(), "{}", stderr(&output));
    assert!(!stderr(&output).contains("Build report"));
    let plan = String::from_utf8(output.stdout).unwrap();
    assert!(!plan.contains("Build report"));
    assert!(plan.contains("exit 17"), "{plan}");
}
