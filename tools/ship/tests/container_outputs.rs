//! Linux container outputs must work with remote engines and always clean up.
#![cfg(unix)]
use std::{fs, os::unix::fs::PermissionsExt, process::Command};

#[test]
fn container_outputs_control_steps_and_cleanup_survives_failures() {
    for failure in ["none", "step", "copy", "missing"] {
        let dir = std::env::temp_dir().join(format!(
            "ship-container-outputs-{}-{failure}",
            std::process::id()
        ));
        fs::create_dir_all(dir.join("bin")).unwrap();
        fs::create_dir_all(dir.join("containers")).unwrap();
        let engine = dir.join("bin/docker");
        fs::write(
            &engine,
            r#"#!/usr/bin/env python3
import os, pathlib, shutil, subprocess, sys
root = pathlib.Path(os.environ['FAKE_ENGINE'])
args = sys.argv[1:]
with (root/'calls').open('a') as log:
    log.write(repr(args)+'\n')
if args[0] == 'run':
    if '--name' not in args:
        print('MOUNT_OK')
        sys.exit(0)
    container = root/'containers'/args[args.index('--name')+1]
    container.mkdir()
    env = os.environ.copy()
    for index, arg in enumerate(args):
        if arg == '-e' and '=' in args[index+1]:
            key, value = args[index+1].split('=', 1)
            env[key] = value
    guest = str(pathlib.PurePosixPath(env['SHIP_OUTPUT']).parent)
    for key in ('SHIP_OUTPUT', 'GITHUB_OUTPUT', 'SHIP_ENV', 'GITHUB_ENV', 'SHIP_PATH', 'GITHUB_PATH'):
        env[key] = env[key].replace(guest, str(container))
    command = args[-1].replace(guest, str(container))
    result = subprocess.run(['sh', '-e', '-c', command], env=env)
    if '--rm' in args:
        shutil.rmtree(container)
    sys.exit(result.returncode)
elif args[0] == 'cp':
    if os.environ['FAILURE'] == 'copy':
        sys.exit(23)
    source = root/'containers'/args[1].split(':')[0]
    if not source.exists():
        sys.exit(24)
    shutil.copytree(source, args[2], dirs_exist_ok=True)
elif args[0] == 'rm':
    shutil.rmtree(root/'containers'/args[-1], ignore_errors=True)
else:
    sys.exit(25)
"#,
        )
        .unwrap();
        fs::set_permissions(&engine, fs::Permissions::from_mode(0o755)).unwrap();
        fs::write(dir.join("config.yaml"), "{}").unwrap();
        fs::write(
            dir.join("pipeline.yaml"),
            r#"
targets: {test: [sdk]}
environments:
  linux: {type: container, image: compiler, engine: docker, sync: none}
jobs:
  sdk:
    runs-in: linux
    steps:
      - run: |
          test "$FAILURE" != step || exit 37
          test "$FAILURE" != missing || exit 0
          printf 'sdk.cached=true\n' > "$SHIP_OUTPUT"
        env:
          FAILURE: '{{ env.FAILURE }}'
        outputs: [sdk.cached]
      - if: '{{ sdk.cached != true }}'
        run: echo 'must not build'
      - run: echo 'cache hit consumed'
"#,
        )
        .unwrap();
        let result = Command::new(env!("CARGO_BIN_EXE_ship"))
            .args(["release", "--target", "test"])
            .arg("--pipeline")
            .arg(dir.join("pipeline.yaml"))
            .arg("--config")
            .arg(dir.join("config.yaml"))
            .env(
                "PATH",
                format!(
                    "{}:{}",
                    dir.join("bin").display(),
                    std::env::var("PATH").unwrap()
                ),
            )
            .env("FAKE_ENGINE", &dir)
            .env("FAILURE", failure)
            .env_remove("SHIP_PROFILE")
            .output()
            .unwrap();
        assert_eq!(
            result.status.success(),
            failure == "none",
            "{}",
            String::from_utf8_lossy(&result.stderr)
        );
        let calls = fs::read_to_string(dir.join("calls")).unwrap();
        assert!(!calls.contains("must not build"));
        assert_eq!(calls.contains("cache hit consumed"), failure == "none");
        assert!(calls.contains("['rm', '-f'"));
        assert_eq!(fs::read_dir(dir.join("containers")).unwrap().count(), 0);
        fs::remove_dir_all(dir).unwrap();
    }
}
