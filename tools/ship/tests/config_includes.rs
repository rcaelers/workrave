//! Config includes and lifecycle hooks through the public CLI.
use std::{fs, path::PathBuf, process::Command};

fn directory(name: &str) -> PathBuf {
    let path = std::env::temp_dir().join(format!("ship-include-{name}-{}", std::process::id()));
    fs::create_dir_all(path.join("machines")).unwrap();
    path
}

#[test]
fn nested_includes_apply_profiles_and_parent_overrides() {
    let dir = directory("merge");
    fs::write(
        dir.join("machines/base.yaml"),
        "execution: {windows: {ssh: build-host, docker: docker.exe}}\n",
    )
    .unwrap();
    fs::write(dir.join("machines/windows.yaml"), "include: base.yaml\nprofiles:\n  proxmox:\n    execution:\n      windows:\n        start: echo start-proxmox\n        stop: echo stop-proxmox\n").unwrap();
    fs::write(
        dir.join("config.yaml"),
        "include: machines/windows.yaml\nexecution: {windows: {docker: C:/custom/docker.exe}}\n",
    )
    .unwrap();
    fs::write(dir.join("pipeline.yaml"), "environments:\n  win:\n    type: windows-container\n    image: compiler\n    ssh: '{{ config.execution.windows.ssh }}'\n    engine: '{{ config.execution.windows.docker }}'\n    start: '{{ config.execution.windows.start }}'\n    stop: '{{ config.execution.windows.stop }}'\ntargets: {test: [build]}\njobs:\n  build:\n    runs-in: win\n    steps: [{run: echo build}]\n").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args([
            "pipeline",
            "show",
            "--target",
            "test",
            "--profile",
            "proxmox",
        ])
        .arg("--config")
        .arg(dir.join("config.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        .output()
        .unwrap();
    fs::remove_dir_all(dir).unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let output = String::from_utf8(output.stdout).unwrap();
    assert!(output.contains("SSH: build-host"));
    assert!(output.contains("start: echo start-proxmox"));
    assert!(output.contains("stop after cleanup: echo stop-proxmox"));
}

#[test]
fn cyclic_includes_fail_before_running_hooks() {
    let dir = directory("cycle");
    fs::write(dir.join("a.yaml"), "include: b.yaml\n").unwrap();
    fs::write(dir.join("b.yaml"), "include: a.yaml\n").unwrap();
    fs::write(
        dir.join("pipeline.yaml"),
        "targets: {test: [build]}\njobs: {build: {steps: [{run: echo never}]}}\n",
    )
    .unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(["release", "--target", "test"])
        .arg("--config")
        .arg(dir.join("a.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        .output()
        .unwrap();
    fs::remove_dir_all(dir).unwrap();
    assert!(!output.status.success());
    assert!(String::from_utf8_lossy(&output.stderr).contains("include cycle"));
}
