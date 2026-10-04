//! Exercise the SSH/SCP boundary: shared container state, outputs and failure cleanup.
#![cfg(unix)]
use std::{fs, os::unix::fs::PermissionsExt, process::Command};

#[test]
fn failed_remote_step_collects_outputs_and_runs_stop_hook() {
    let dir = std::env::temp_dir().join(format!("ship-session-test-{}", std::process::id()));
    fs::create_dir_all(dir.join("bin")).unwrap();
    fs::create_dir_all(dir.join("input")).unwrap();
    let input = dir.join("input/example.po");
    fs::write(&input, "translation source").unwrap();
    if cfg!(target_os = "macos") {
        assert!(Command::new("xattr")
            .args(["-w", "com.workrave.ship-test", "metadata"])
            .arg(&input)
            .status()
            .unwrap()
            .success());
    }
    let fake = r#"#!/usr/bin/env python3
import base64, json, os, pathlib, re, shlex, shutil, sys, tarfile
root=pathlib.Path(os.environ['FAKE_WINDOWS'])
def mapped(path):
    return root/'guest'/path.replace(':','').lstrip('/')
args=sys.argv[1:]
if pathlib.Path(sys.argv[0]).name=='scp':
    source,target=args[-2:]
    if source.startswith('fake-windows:'): source=str(mapped(source.split(':',1)[1]))
    if target.startswith('fake-windows:'): target=str(mapped(target.split(':',1)[1]))
    pathlib.Path(target).parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(source,target)
    sys.exit(0)
command=shlex.split(args[-1])
if '-EncodedCommand' in command:
    sys.exit(0) # create the host staging directory
script=mapped(command[-1]).read_text(encoding='utf-8-sig')
match=re.search(r"\$p.StartInfo.Arguments='(.*)'",script)
if match:
    native=match[1].replace("''", "'")
    with (root/'calls').open('a') as out: out.write(native+'\n')
    if native.startswith('"exec"'):
        inner=base64.b64decode(native.rsplit(' ',1)[1].strip('"')).decode('utf-16le')
        if "Write-Output 'directory'" in inner:
            print('directory')
        if 'Get-Content C:/ship-output.txt' in inner:
            print('token=remembered')
        if "Write-Output 'remembered'" in inner:
            (root/'output_used').write_text('yes')
        if 'exit 37' in inner: sys.exit(37)
    if native.startswith('"cp"') and ':C:/logs' in native:
        target=re.findall(r'"([^"]+)"',native)[-1]
        mapped(target).mkdir(parents=True,exist_ok=True)
        (mapped(target)/'failure.log').write_text('partial build diagnostics')
else:
    match=re.search(r"tar.exe -czf '([^']+)' -C '([^']+)'",script)
    if match:
        archive,source=map(mapped,match.groups())
        with tarfile.open(archive,'w:gz') as out: out.add(source,arcname='.')
"#;
    for command in ["ssh", "scp"] {
        let path = dir.join("bin").join(command);
        fs::write(&path, fake).unwrap();
        fs::set_permissions(path, fs::Permissions::from_mode(0o755)).unwrap();
    }
    fs::write(dir.join("config.yaml"), "{}").unwrap();
    let pipeline = format!(
        r#"
environments:
  windows:
    type: windows-container
    image: test-compiler
    ssh: fake-windows
    start: echo start >> '{0}/hooks'
    stop: echo stop >> '{0}/hooks'
    copy:
      '{0}/input': C:/input
    collect:
      C:/logs: '{0}/recovered'
targets: {{test: [build]}}
jobs:
  build:
    runs-in: windows
    steps:
      - run: Add-Content $env:SHIP_OUTPUT 'token=remembered'
        outputs: [token]
      - run: Write-Output '{{{{ token }}}}'
      - run: exit 37
      - run: Write-Output 'must not run'
"#,
        dir.display()
    );
    fs::write(dir.join("pipeline.yaml"), pipeline).unwrap();
    let path = format!(
        "{}:{}",
        dir.join("bin").display(),
        std::env::var("PATH").unwrap()
    );
    let output = Command::new(env!("CARGO_BIN_EXE_ship"))
        .args(["release", "--target", "test"])
        .arg("--config")
        .arg(dir.join("config.yaml"))
        .arg("--pipeline")
        .arg(dir.join("pipeline.yaml"))
        .env("PATH", path)
        .env("FAKE_WINDOWS", &dir)
        .env_remove("SHIP_PROFILE")
        .output()
        .unwrap();
    assert!(!output.status.success());
    assert!(
        dir.join("output_used").exists(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_eq!(
        fs::read_to_string(dir.join("hooks")).unwrap(),
        "start\nstop\n"
    );
    assert_eq!(
        fs::read_to_string(dir.join("recovered/failure.log")).unwrap(),
        "partial build diagnostics"
    );
    let calls = fs::read_to_string(dir.join("calls")).unwrap();
    assert_eq!(
        calls
            .lines()
            .filter(|line| line.starts_with("\"create\""))
            .count(),
        1
    );
    assert!(calls
        .lines()
        .any(|line| line.starts_with("\"rm\" \"--force\"")));
    assert!(!calls.contains("must not run"));
    let guest = fs::read_dir(dir.join("guest/C/ship"))
        .unwrap()
        .next()
        .unwrap()
        .unwrap()
        .path();
    assert!(Command::new("python3")
        .args(["-c", "import sys, tarfile; names=tarfile.open(sys.argv[1]).getnames(); assert not any('/._' in name or name.startswith('._') for name in names), names"])
        .arg(guest.join("input-0"))
        .status()
        .unwrap()
        .success());
    fs::remove_dir_all(dir).unwrap();
}
