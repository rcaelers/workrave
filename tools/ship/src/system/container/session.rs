//! One Linux job container, shared by all its run steps.
use std::path::Path;
use std::sync::atomic::{AtomicUsize, Ordering};

use anyhow::Result;

use super::{ContainerRun, Engine, Mounts};
use crate::system::process::Cmd;

pub struct Session {
    engine: Engine,
    name: String,
    create: ContainerRun,
}

impl Session {
    pub fn new(engine: Engine, run: ContainerRun) -> Self {
        static NEXT: AtomicUsize = AtomicUsize::new(0);
        let name = format!(
            "ship-job-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        );
        Self {
            engine,
            create: run
                .remove_on_exit(false)
                .name(&name)
                .flags(["--detach", "--entrypoint", "/bin/sh"])
                .command([
                    "-c",
                    "trap 'exit 0' TERM INT; while :; do sleep 3600 & wait $!; done",
                ]),
            name,
        }
    }

    pub fn prepare(&self, mounts: &Mounts, show: bool) -> Result<()> {
        let cmd = self.create.to_cmd(self.engine, mounts);
        if show {
            println!("   job container: {}", cmd.display());
            Ok(())
        } else {
            cmd.run()
        }
    }

    pub fn command(&self, command: &[String], env: &[(String, String)], cwd: Option<&Path>) -> Cmd {
        let mut cmd = Cmd::new(self.engine.program()).arg("exec");
        for (key, value) in env {
            cmd = cmd.arg("-e").arg(format!("{key}={value}"));
        }
        if let Some(cwd) = cwd {
            cmd = cmd.arg("--workdir").arg(cwd);
        }
        cmd.arg(&self.name).args(command)
    }

    pub fn collect_commands(&self, guest: &str, local: &Path) -> Result<()> {
        Cmd::new(self.engine.program())
            .args(["cp", &format!("{}:{guest}/.", self.name)])
            .arg(local)
            .run()
    }

    pub fn finish(&self) -> Result<()> {
        let mut cmd = Cmd::new(self.engine.program()).args(["rm", "-f"]);
        if self.engine == Engine::Podman {
            cmd = cmd.arg("--ignore");
        }
        cmd.arg(&self.name).run_cleanup()
    }
}
