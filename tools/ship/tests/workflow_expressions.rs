//! Black-box workflow semantics, including failure recovery and dependency data.
#![cfg(unix)]
use std::process::{Command, Output};
use std::sync::atomic::{AtomicUsize, Ordering};

fn run(pipeline: &str, arguments: &[&str]) -> Output {
    static NEXT: AtomicUsize = AtomicUsize::new(0);
    let dir = std::env::temp_dir().join(format!(
        "ship-expressions-test-{}-{}",
        std::process::id(),
        NEXT.fetch_add(1, Ordering::Relaxed)
    ));
    std::fs::create_dir_all(&dir).unwrap();
    std::fs::write(dir.join("config.yaml"), "{}").unwrap();
    std::fs::write(dir.join("pipeline.yaml"), pipeline).unwrap();
    let result = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(["release", "--target", "test", "--config"])
        .arg(dir.join("config.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        .args(arguments)
        .env_remove("SHIP_PROFILE")
        .output()
        .unwrap();
    std::fs::remove_dir_all(dir).unwrap();
    result
}
fn output(result: &Output) -> String {
    String::from_utf8_lossy(&result.stdout).into_owned()
}
fn errors(result: &Output) -> String {
    String::from_utf8_lossy(&result.stderr).into_owned()
}

#[test]
fn unsupported_expressions_fail_before_any_job_runs() {
    for condition in ["false && unsupported()", "${{ false && unsupported() }}"] {
        let pipeline = format!(
            "targets: {{test: [first]}}\njobs:\n  first:\n    steps:\n      - run: echo MUST_NOT_RUN\n  unselected:\n    steps:\n      - if: {condition}\n        run: echo MUST_NOT_RUN\n"
        );
        let result = run(&pipeline, &[]);
        assert!(!result.status.success());
        assert!(
            errors(&result).contains("unsupported"),
            "{}",
            errors(&result)
        );
        assert!(!output(&result).contains("MUST_NOT_RUN"));
    }
}

#[test]
fn typed_outputs_dynamic_matrix_and_bare_conditions() {
    let result = run(
        r#"
options:
  debug: {flag: true}
targets: {test: [build]}
jobs:
  prepare:
    outputs:
      matrix: ${{ steps.produce.outputs.matrix }}
    steps:
      - id: produce
        run: |
          echo 'matrix={"arch":["amd64","arm64"],"configuration":["Release"],"include":[{"arch":"amd64","extra":"yes"}]}' >> "$SHIP_OUTPUT"
          echo 'cached=false' >> "$SHIP_OUTPUT"
      - if: steps.produce.outputs.cached
        run: echo NONEMPTY_STRING_IS_TRUE
      - if: fromJSON(steps.produce.outputs.cached)
        run: echo MUST_NOT_RUN
      - if: false
        run: echo MUST_NOT_RUN
      - if: inputs.debug && !cancelled()
        run: echo DEBUG_SELECTED
  build:
    needs: prepare
    strategy:
      matrix: ${{ fromJSON(needs.prepare.outputs.matrix) }}
    steps:
      - run: echo BUILD_${{ matrix.arch }}_${{ matrix.configuration }}_${{ matrix.extra }}
"#,
        &["--debug"],
    );
    assert!(result.status.success(), "{}", errors(&result));
    let out = output(&result);
    assert!(out.contains("NONEMPTY_STRING_IS_TRUE"));
    assert!(out.contains("DEBUG_SELECTED"));
    assert!(out.contains("BUILD_amd64_Release_yes"));
    assert!(out.contains("BUILD_arm64_Release_"));
    assert!(!out.contains("MUST_NOT_RUN"));
}

#[test]
fn dependency_failures_skip_dependents_but_allow_recovery_and_independent_jobs() {
    let result = run(
        r#"
targets: {test: [dependent, independent, recovery]}
jobs:
  build:
    steps:
      - id: compile
        run: exit 37
      - run: echo MUST_NOT_RUN
      - if: failure() && steps.compile.outcome == 'failure'
        run: echo STEP_RECOVERY
      - if: always()
        run: echo STEP_ALWAYS
      - foreach: ${{ fromJSON('["skip", "recover"]') }}
        if: failure() && item == 'recover'
        run: echo ITEM_RECOVERY
  dependent:
    needs: build
    steps:
      - run: echo MUST_NOT_RUN
  independent:
    steps:
      - run: echo INDEPENDENT_RAN
  recovery:
    needs: dependent
    if: failure() && needs.dependent.result == 'skipped'
    steps:
      - run: echo ANCESTOR_FAILURE_DETECTED
"#,
        &[],
    );
    assert!(!result.status.success());
    let out = output(&result);
    for marker in [
        "STEP_RECOVERY",
        "STEP_ALWAYS",
        "ITEM_RECOVERY",
        "INDEPENDENT_RAN",
        "ANCESTOR_FAILURE_DETECTED",
    ] {
        assert!(out.contains(marker), "{out}\n{}", errors(&result));
    }
    assert!(!out.contains("MUST_NOT_RUN"));
    assert!(errors(&result).contains("exit status: 37"));
}

#[test]
fn skipped_prerequisites_and_explicit_skip_have_real_status() {
    let pipeline = r#"
targets: {test: [dependent, recovery]}
jobs:
  prerequisite:
    steps: [{run: 'echo PREREQUISITE_RAN'}]
  dependent:
    needs: prerequisite
    steps: [{run: 'echo MUST_NOT_RUN'}]
  recovery:
    needs: dependent
    if: always() && !failure()
    steps: [{run: 'echo SKIP_RECOVERY'}]
"#;
    let result = run(pipeline, &["--skip-job", "prerequisite"]);
    assert!(result.status.success(), "{}", errors(&result));
    assert!(output(&result).contains("SKIP_RECOVERY"));
    assert!(!output(&result).contains("MUST_NOT_RUN"));
    assert!(!output(&result).contains("PREREQUISITE_RAN"));
}

#[test]
fn job_condition_precedes_matrix_and_invalid_matrix_context_is_rejected() {
    let result = run(
        r#"
targets: {test: [build]}
jobs:
  build:
    if: false
    strategy: {matrix: "${{ fromJSON('invalid') }}"}
    steps: [{run: 'echo MUST_NOT_RUN'}]
"#,
        &[],
    );
    assert!(result.status.success(), "{}", errors(&result));
    let result = run(
        r#"
targets: {test: [build]}
jobs:
  build:
    if: matrix.arch == 'amd64'
    strategy: {matrix: {arch: [amd64]}}
    steps: [{run: 'true'}]
"#,
        &[],
    );
    assert!(!result.status.success());
    assert!(errors(&result).contains("before matrix expansion"));
}

#[test]
fn matrix_include_exclude_and_fail_fast() {
    let pipeline = r#"
targets: {test: [build]}
jobs:
  build:
    strategy:
      fail-fast: REPLACE_FAIL_FAST
      max-parallel: 1
      matrix:
        arch: [amd64, arm64]
        configuration: [Release, Debug]
        exclude: [{arch: arm64, configuration: Debug}]
        include:
          - {extra: initial}
          - {arch: amd64, extra: replaced}
          - {arch: extra, configuration: Release}
    steps:
      - run: |
          echo ENTRY_${{ matrix.arch }}_${{ matrix.configuration }}_${{ matrix.extra }}
          test '${{ matrix.arch }}' != amd64
"#;
    let full = run(&pipeline.replace("REPLACE_FAIL_FAST", "false"), &[]);
    assert!(!full.status.success());
    let out = output(&full);
    for entry in [
        "ENTRY_amd64_Release_replaced",
        "ENTRY_amd64_Debug_replaced",
        "ENTRY_arm64_Release_initial",
        "ENTRY_extra_Release_",
    ] {
        assert!(out.contains(entry), "{out}\n{}", errors(&full));
    }
    assert!(!out.contains("ENTRY_arm64_Debug"));
    let fast = run(&pipeline.replace("REPLACE_FAIL_FAST", "true"), &[]);
    assert!(!fast.status.success());
    assert_eq!(
        output(&fast)
            .lines()
            .filter(|line| line.starts_with("ENTRY_"))
            .count(),
        1
    );
}

#[test]
fn foreach_conditions_see_the_current_item() {
    let result = run(
        r#"
targets: {test: [build]}
jobs:
  build:
    steps:
      - foreach: ${{ fromJSON('["first", "second"]') }}
        if: item == 'second'
        run: echo ITEM_${{ item }}
"#,
        &[],
    );
    assert!(result.status.success(), "{}", errors(&result));
    assert_eq!(output(&result).trim(), "ITEM_second");
}

#[test]
fn failed_steps_still_export_outputs_for_recovery() {
    let result = run(
        r#"
targets: {test: [recover]}
jobs:
  build:
    outputs: {diagnostic: '${{ steps.compile.outputs.diagnostic }}'}
    steps:
      - id: compile
        run: |
          echo 'diagnostic=partial' >> "$SHIP_OUTPUT"
          exit 23
      - if: failure()
        run: echo STEP_${{ steps.compile.outputs.diagnostic }}
  recover:
    needs: build
    if: failure()
    steps:
      - run: echo JOB_${{ needs.build.outputs.diagnostic }}
"#,
        &[],
    );
    assert!(!result.status.success());
    assert!(
        output(&result).contains("STEP_partial"),
        "{}",
        errors(&result)
    );
    assert!(
        output(&result).contains("JOB_partial"),
        "{}",
        errors(&result)
    );
    assert!(errors(&result).contains("exit status: 23"));
}
