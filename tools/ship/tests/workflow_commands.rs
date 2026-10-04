//! Exercise workflow command files and scopes through real shell steps.
#![cfg(unix)]

use std::process::{Command, Output};
use std::sync::atomic::{AtomicUsize, Ordering};

fn run(pipeline: &str) -> Output {
    static NEXT: AtomicUsize = AtomicUsize::new(0);
    let dir = std::env::temp_dir().join(format!(
        "ship-workflow-test-{}-{}",
        std::process::id(),
        NEXT.fetch_add(1, Ordering::Relaxed)
    ));
    std::fs::create_dir_all(dir.join("tools")).unwrap();
    let dir = dir.canonicalize().unwrap();
    std::fs::write(dir.join("config.yaml"), "{}").unwrap();
    std::fs::write(
        dir.join("pipeline.yaml"),
        pipeline.replace("@ROOT@", &dir.to_string_lossy()),
    )
    .unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(["release", "--target", "test", "--config"])
        .arg(dir.join("config.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        .env_remove("SHIP_PROFILE")
        .env_remove("BUILD_TEST_VALUE")
        .output()
        .unwrap();
    std::fs::remove_dir_all(&dir).unwrap();
    output
}

fn succeeds(pipeline: &str) {
    let output = run(pipeline);
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
}

#[test]
fn strings_multiline_aliases_env_path_and_output_scopes() {
    succeeds(
        r#"
targets: {test: [produce, consume, isolated]}
env: {BUILD_TEST_VALUE: workflow}
defaults:
  run:
    shell: bash
    working-directory: '@ROOT@'
jobs:
  produce:
    env: {BUILD_TEST_VALUE: job}
    outputs:
      flag: '{{ steps.cache.outputs.flag }}'
      body: '{{ steps.cache.outputs.body }}'
    steps:
      - id: cache
        run: |
          test "$SHIP_OUTPUT" = "$GITHUB_OUTPUT"
          test "$SHIP_ENV" = "$GITHUB_ENV"
          test "$SHIP_PATH" = "$GITHUB_PATH"
          test "$BUILD_TEST_VALUE" = job
          test '{{ env.BUILD_TEST_VALUE }}' = job
          printf 'flag=false\nempty=\nbody<<EOF\n line one \nline=two\nEOF\n' >> "$GITHUB_OUTPUT"
          printf 'BUILD_TEST_VALUE=written\nLAST_FILE=%s\n' "$SHIP_OUTPUT" >> "$SHIP_ENV"
          printf '#!/bin/sh\nprintf "from path"\n' > tools/example-command
          chmod +x tools/example-command
          printf '%s\n' "$PWD/tools" >> "$GITHUB_PATH"
          test "$BUILD_TEST_VALUE" = job
      - run: |
          test "$SHIP_OUTPUT" != "$LAST_FILE"
          test ! -s "$SHIP_OUTPUT"
          test "$BUILD_TEST_VALUE" = written
          test '{{ env.BUILD_TEST_VALUE }}' = written
          test '{{ steps.cache.outputs.flag }}' = false
          test '{{ steps.cache.outputs.flag == false }}' = False
          test '{{ steps.cache.outputs.empty }}' = ''
          test "$(example-command)" = 'from path'
          printf 'BUILD_TEST_VALUE=after-step\n' >> "$GITHUB_ENV"
      - run: |
          test "$BUILD_TEST_VALUE" = after-step-step
          test '{{ env.BUILD_TEST_VALUE }}' = after-step-step
        env: {BUILD_TEST_VALUE: '{{ env.BUILD_TEST_VALUE }}-step'}
      - id: skipped
        if: 'false'
        run: exit 51
      - run: |
          test "$BUILD_TEST_VALUE" = after-step
          test '{{ steps.skipped.outcome }}' = skipped
  consume:
    needs: produce
    steps:
      - run: |
          test '{{ needs.produce.outputs.flag }}' = false
          test '{{ needs.produce.result }}' = success
          test '{{ steps | length }}' = 0
          test "$BUILD_TEST_VALUE" = workflow
          test -z "${LAST_FILE-}"
          ! command -v example-command
          test "$BODY" = $' line one \nline=two'
        env: {BODY: '{{ needs.produce.outputs.body }}'}
  isolated:
    steps:
      - run: |
          test '{{ needs | length }}' = 0
          test '{{ steps | length }}' = 0
          test "$BUILD_TEST_VALUE" = workflow
"#,
    );
}

#[test]
fn matrix_entries_do_not_share_steps_or_environment_changes() {
    succeeds(
        r#"
targets: {test: [build, use]}
jobs:
  build:
    strategy: {matrix: {arch: [first, second]}}
    outputs: {arch: '{{ steps.compile.outputs.arch }}'}
    steps:
      - id: compile
        run: |
          test '{{ steps | length }}' = 0
          test -z "${BUILD_TEST_VALUE-}"
          echo 'arch={{ matrix.arch }}' >> "$SHIP_OUTPUT"
          echo 'BUILD_TEST_VALUE=set' >> "$SHIP_ENV"
  use:
    needs: build
    steps:
      - run: test '{{ needs.build.outputs.arch }}' = second
"#,
    );
}

#[test]
fn shell_and_directory_precedence_and_failure_behavior() {
    succeeds(
        r#"
targets: {test: [build]}
defaults:
  run: {shell: bash, working-directory: '@ROOT@'}
jobs:
  build:
    defaults:
      run: {working-directory: '@ROOT@/tools'}
    steps:
      - run: test "$PWD" = '@ROOT@/tools' && [[ -n "$BASH_VERSION" ]]
      - working-directory: '@ROOT@'
        shell: sh
        run: test "$PWD" = '@ROOT@'
      - cwd: '@ROOT@'
        run: test "$PWD" = '@ROOT@'
"#,
    );
    for (shell, script) in [
        ("bash", "false | true\necho must-not-run"),
        ("sh", "false\necho must-not-run"),
    ] {
        let output = run(&format!("targets: {{test: [build]}}\njobs:\n  build:\n    steps:\n      - shell: {shell}\n        run: |\n          {}\n", script.replace('\n', "\n          ")));
        assert!(!output.status.success());
        assert!(!String::from_utf8_lossy(&output.stdout).contains("must-not-run"));
    }
}

#[test]
fn invalid_ids_and_ambiguous_legacy_output_declarations_are_rejected() {
    for steps in [
        "[{id: duplicate, run: true}, {id: duplicate, run: true}]",
        "[{id: 1invalid, run: true}]",
        "[{id: example, run: true, outputs: [value]}]",
        "[{id: example, run: true, foreach: '[1, 2]'}]",
    ] {
        let output = run(&format!(
            "targets: {{test: [build]}}\njobs:\n  build:\n    steps: {steps}\n"
        ));
        assert!(!output.status.success());
    }
}
