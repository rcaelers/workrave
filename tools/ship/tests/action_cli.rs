//! Portable operations use the same implementation and propagate failures.
use std::{
    fs,
    io::Write,
    process::{Command, Stdio},
};

fn command() -> Command {
    let mut cmd = Command::new(env!("CARGO_BIN_EXE_ship"));
    cmd.arg("action")
        .env_remove("SHIP_PROFILE")
        .env("SHIP_CONFIG", "/nonexistent-ship-config");
    cmd
}

#[test]
fn portable_actions_accept_files_stdin_and_environment_without_a_workflow() {
    let dir = std::env::temp_dir().join(format!("ship-actions-{}", std::process::id()));
    fs::create_dir_all(&dir).unwrap();
    let notes = dir.join("release notes.txt");
    fs::write(&notes, "Test release notes").unwrap();
    let params = serde_json::json!({"repo":"https://github.com/example/project.git","token":"unused", "tag":"v1", "title":"test", "notes":notes,"assets":[notes]});
    fs::write(dir.join("params.json"), params.to_string()).unwrap();
    for mode in ["file", "stdin", "env"] {
        let mut cmd = command();
        cmd.args(["github-release", "--dry-run"]);
        match mode {
            "file" => {
                cmd.arg("--params").arg(dir.join("params.json"));
            }
            "env" => {
                cmd.args(["--params-env", "ACTION_TEST_INPUT"])
                    .env("ACTION_TEST_INPUT", params.to_string());
            }
            _ => {
                cmd.args(["--params", "-"]).stdin(Stdio::piped());
            }
        }
        let mut child = cmd
            .stdout(Stdio::piped())
            .stderr(Stdio::piped())
            .spawn()
            .unwrap();
        if mode == "stdin" {
            child
                .stdin
                .take()
                .unwrap()
                .write_all(params.to_string().as_bytes())
                .unwrap();
        }
        let result = child.wait_with_output().unwrap();
        assert!(
            result.status.success(),
            "{}",
            String::from_utf8_lossy(&result.stderr)
        );
        assert!(String::from_utf8_lossy(&result.stdout)
            .contains("DRYRUN: create draft GitHub release v1"));
    }
    let failed = command()
        .args(["sign", "--dry-run", "--params-env", "ACTION_TEST_INPUT"])
        .env(
            "ACTION_TEST_INPUT",
            serde_json::json!({"kind":"cosign","files":[dir.join("missing")] }).to_string(),
        )
        .output()
        .unwrap();
    assert!(!failed.status.success());
    assert!(String::from_utf8_lossy(&failed.stderr).contains("no files match"));
    let invalid = command()
        .args(["sign", "--params-env", "ACTION_TEST_INPUT"])
        .env("ACTION_TEST_INPUT", r#"{"kind":"typo","files":[]}"#)
        .output()
        .unwrap();
    assert!(!invalid.status.success());
    assert!(String::from_utf8_lossy(&invalid.stderr).contains("unknown kind"));
    fs::remove_dir_all(dir).unwrap();
}
