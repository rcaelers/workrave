//! Verify native editions, visible build stages and portable machine configuration.
use std::path::Path;
use std::process::Command;

fn show(arguments: &[&str]) -> String {
    let root = Path::new(env!("CARGO_MANIFEST_DIR"));
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(["pipeline", "show", "--target", "windows"])
        .args(arguments)
        .arg("--pipeline")
        .arg(root.join("../local/release.yaml"))
        .arg("--config")
        .arg(root.join("../local/ship.example.yaml"))
        .env_remove("SHIP_PROFILE")
        .output()
        .unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    String::from_utf8(output.stdout).unwrap()
}

#[test]
fn windows_editions_have_separate_native_build_stages_and_correct_paths() {
    let plan = show(&["--job", "win-build", "--no-sign", "--debug"]);
    for edition in ["gtk3", "qt"] {
        for configuration in ["Release", "Debug"] {
            assert!(plan.contains(&format!(
                "BUILD_DIR=C:/workspace/source/_build/{edition}/{configuration}"
            )));
            assert!(plan.contains(&format!(
                "OUTPUT_DIR=C:/workspace/source/_output/{edition}/{configuration}"
            )));
        }
    }
    for stage in [
        "cmake -S",
        "cmake --build",
        "ctest --test-dir",
        "cmake --install",
        "--target portable",
        "--target installer",
        "--component SBOM",
    ] {
        assert!(plan.contains(stage), "missing {stage}");
    }
    assert!(plan.contains("Windows container: workrave-build:windows-msys2 (SSH: windows-builder)"));
    assert!(plan.contains("Windows container: workrave-build:windows-conan (SSH: windows-builder)"));
    assert!(!plan.contains("incus"));
    assert!(!plan.contains("build-windows-qt.py"));
    assert!(!plan.contains("run-windows-container.py"));
    assert!(!plan.contains("-DWITH_SIGN=ON"));
}

#[test]
fn linux_conan_cache_is_preserved() {
    let plan = show(&["--job", "win-qt-sdk"]);
    assert!(plan.contains("--platform linux/amd64"));
    assert!(plan.contains("--volume=workrave-conan:/cache"));
    assert!(!plan.contains("prepare-windows-qt-sdk.py"));
    for stage in [
        "conan/setup.py",
        "conan install",
        "tools.graph:skip_binaries=False",
        "cmake --fresh",
        "--arch x86_64 --validate",
        "conan/export-sdk.py",
        "tar --dereference -I",
        "mv /workspace/sdk/sdk.tar.gz.tmp /workspace/sdk/sdk.tar.gz",
    ] {
        assert!(plan.contains(stage), "missing SDK stage: {stage}");
    }
    assert!(plan.contains("ghcr.io/rcaelers/workrave-build:llvm-mingw"));
}

#[test]
fn cache_recovery_needs_no_release_configuration() {
    let root = Path::new(env!("CARGO_MANIFEST_DIR"));
    let dir = std::env::temp_dir().join(format!("ship-recovery-test-{}", std::process::id()));
    std::fs::create_dir_all(&dir).unwrap();
    let config = serde_json::json!({"workspace_dir":dir.join("workspace"), "scripts_dir":root.join(".."),
        "container":{"engine":"podman","sync":"auto","remote_dir":".cache/ship"}, "windows":{},
        "execution":{"windows":{"ssh":"my-windows-builder"}}});
    std::fs::write(dir.join("config.yaml"), config.to_string()).unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args([
            "pipeline",
            "show",
            "--target",
            "windows-dependencies",
            "--set",
            "config.windows.conan_cache=recovery-linux",
            "--set",
            "config.windows.qt_sdk_cache=recovery-windows",
        ])
        .arg("--pipeline")
        .arg(root.join("../local/release.yaml"))
        .arg("--config")
        .arg(dir.join("config.yaml"))
        .env_remove("SHIP_PROFILE")
        .output()
        .unwrap();
    std::fs::remove_dir_all(dir).unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let plan = String::from_utf8(output.stdout).unwrap();
    assert!(
        plan.contains("Jobs: check-config-windows-dependencies, win-qt-sdk, win-qt-sdk-install")
    );
    assert!(plan.contains("--volume=recovery-linux:/cache"));
    assert!(plan.contains("SSH: my-windows-builder"));
    assert!(plan.contains("init-sdk.py C:/input/sdk.tar.gz"));
    assert!(!plan.contains("== workspace"));
    assert!(!plan.contains("== win-build"));
    assert!(!plan.contains("== win-upload"));
}
