//! A job container retains its filesystem and is removed on every exit path.
#![cfg(unix)]
use std::{fs, os::unix::fs::PermissionsExt, process::Command};

#[test]
fn persistent_container_state_outputs_and_failure_cleanup() {
    for failure in ["none", "step", "copy", "prepare"] {
        let dir = std::env::temp_dir().join(format!(
            "ship-job-container-{}-{failure}",
            std::process::id()
        ));
        fs::create_dir_all(dir.join("bin")).unwrap();
        fs::create_dir_all(dir.join("containers")).unwrap();
        let engine = dir.join("bin/docker");
        fs::write(
            &engine,
            r#"#!/usr/bin/env python3
import os, pathlib, shutil, subprocess, sys
root=pathlib.Path(os.environ['FAKE_ENGINE'])
args=sys.argv[1:]
with (root/'calls').open('a') as log: log.write(repr(args)+'\n')
if args[0]=='run':
    if '--name' not in args:
        print('MOUNT_OK'); sys.exit(0)
    assert '--detach' in args
    container=root/'containers'/args[args.index('--name')+1]
    container.mkdir()
    sys.exit(29 if os.environ['FAILURE']=='prepare' else 0)
elif args[0]=='exec':
    name=next(a for a in args if a.startswith('ship-job-'))
    container=root/'containers'/name
    assert container.is_dir()
    env=os.environ.copy()
    for i,arg in enumerate(args[:args.index(name)]):
        if arg=='-e':
            key,value=args[i+1].split('=',1); env[key]=value
    guest=str(pathlib.PurePosixPath(env['SHIP_OUTPUT']).parent)
    local=container/pathlib.PurePosixPath(guest).name
    for key in ('SHIP_OUTPUT','GITHUB_OUTPUT','SHIP_ENV','GITHUB_ENV','SHIP_PATH','GITHUB_PATH'):
        env[key]=env[key].replace(guest,str(local))
    script=args[-1].replace(guest,str(local)).replace('/job-state',str(container/'state'))
    sys.exit(subprocess.run(['sh','-e','-c',script],env=env).returncode)
elif args[0]=='cp':
    if os.environ['FAILURE']=='copy': sys.exit(23)
    name,guest=args[1].split(':',1)
    source=root/'containers'/name/pathlib.PurePosixPath(guest).name
    shutil.copytree(source,args[2],dirs_exist_ok=True)
elif args[0]=='rm':
    shutil.rmtree(root/'containers'/args[-1],ignore_errors=True)
else: sys.exit(25)
"#,
        )
        .unwrap();
        fs::set_permissions(&engine, fs::Permissions::from_mode(0o755)).unwrap();
        fs::write(dir.join("config.yaml"), "{}").unwrap();
        fs::write(
            dir.join("runners.yaml"),
            "runners:\n  builder: {type: linux-container-host, engine: docker, sync: none}\n",
        )
        .unwrap();
        fs::write(
            dir.join("pipeline.yaml"),
            r#"
runner-files: [runners.yaml]
targets: {test: [build]}
jobs:
  build:
    runs-on: builder
    container:
      image: compiler
      options: --cpus 2 --label 'test label=value with spaces'
      volumes: [test-cache:/cache]
    steps:
      - id: write
        run: |
          echo remembered > /job-state
          echo token=remembered >> "$SHIP_OUTPUT"
          echo SHARED=from-step >> "$SHIP_ENV"
          test "$FAILURE" != step || exit 37
        env: {FAILURE: '${{ env.FAILURE }}'}
      - run: |
          test "$(cat /job-state)" = remembered
          test '${{ steps.write.outputs.token }}' = remembered
          test "$SHARED" = from-step
          echo STATE_REUSED
      - if: failure()
        run: test "$(cat /job-state)" = remembered && echo RECOVERY_RAN
"#,
        )
        .unwrap();
        let result = Command::new(env!("CARGO_BIN_EXE_ship"))
            .args(["release", "--target", "test", "--config"])
            .arg(dir.join("config.yaml"))
            .arg("--pipeline")
            .arg(dir.join("pipeline.yaml"))
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
        let stderr = String::from_utf8_lossy(&result.stderr);
        let stdout = String::from_utf8_lossy(&result.stdout);
        assert_eq!(result.status.success(), failure == "none", "{stderr}");
        assert_eq!(
            stdout.contains("STATE_REUSED"),
            failure == "none",
            "{stdout}\n{stderr}"
        );
        assert_eq!(
            stdout.contains("RECOVERY_RAN"),
            matches!(failure, "step" | "copy"),
            "{stdout}\n{stderr}"
        );
        let calls = fs::read_to_string(dir.join("calls")).unwrap();
        assert_eq!(
            calls.lines().filter(|l| l.contains("'--detach'")).count(),
            1,
            "{calls}"
        );
        assert!(calls.contains("'test label=value with spaces'"));
        assert!(calls.contains("'--volume=test-cache:/cache'"));
        assert!(calls.contains("['rm', '-f'"));
        assert_eq!(fs::read_dir(dir.join("containers")).unwrap().count(), 0);
        fs::remove_dir_all(dir).unwrap();
    }
}
