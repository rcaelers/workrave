//! Check host/target selection in the actual Workrave release pipeline.

use std::path::Path;
use std::process::Command;

fn appimage_commands(platform: &str, cross_image: Option<&str>) -> Vec<String> {
    let crate_dir = Path::new(env!("CARGO_MANIFEST_DIR"));
    let mut command = Command::new(env!("CARGO_BIN_EXE_ship"));
    command
        .args(["pipeline", "show", "--target", "linux", "--job", "appimage"])
        .arg("--pipeline")
        .arg(crate_dir.join("../local/release.yaml"))
        .arg("--config")
        .arg(crate_dir.join("../local/ship.example.yaml"))
        .arg("--set")
        .arg(format!("config.container.platform={platform}"))
        .env_remove("SHIP_PROFILE");
    if let Some(image) = cross_image {
        command
            .arg("--set")
            .arg(format!("config.linux.cross_image={image}"));
    }
    let output = command.output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    let plan = String::from_utf8(output.stdout).unwrap();
    let commands: Vec<_> = plan
        .lines()
        .filter(|line| line.contains("$ podman run"))
        .map(str::to_owned)
        .collect();
    assert_eq!(commands.len(), 2, "{plan}");
    commands
}

#[test]
fn amd64_host_cross_compiles_arm64_with_native_tools() {
    let commands = appimage_commands("linux/amd64", None);
    assert!(commands[0].contains("--platform linux/amd64"));
    assert!(commands[0].contains("CONF_TARGET_ARCH=x86_64"));
    assert!(commands[0].contains("workrave-build:ubuntu-resolute"));
    assert!(commands[1].contains("--platform linux/amd64"));
    assert!(commands[1].contains("CONF_TARGET_ARCH=aarch64"));
    assert!(commands[1].contains("workrave-build:ubuntu-cross-aarch64"));
    for command in commands {
        assert!(!command.contains("/dev/fuse"));
        assert!(!command.contains("--cap-add"));
    }
}

#[test]
fn arm64_host_keeps_native_arm64_build() {
    let commands = appimage_commands("linux/arm64", None);
    assert!(commands[0].contains("--platform linux/amd64"));
    assert!(commands[1].contains("--platform linux/arm64"));
    assert!(commands[1].contains("CONF_TARGET_ARCH=aarch64"));
    for command in commands {
        assert!(command.contains("workrave-build:ubuntu-resolute"));
        assert!(!command.contains("ubuntu-cross-aarch64"));
    }
}

#[test]
fn cross_image_can_be_overridden_without_changing_native_image() {
    let commands = appimage_commands("linux/amd64", Some("cross-candidate"));
    assert!(commands[0].contains("workrave-build:ubuntu-resolute"));
    assert!(commands[1].contains("workrave-build:cross-candidate"));
}
