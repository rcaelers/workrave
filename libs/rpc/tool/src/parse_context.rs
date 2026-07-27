//! Loads the compiler-independent parse context produced by CMake.
//!
//! Each non-empty line is a `kind=value` record. CMake owns discovery of the
//! effective target properties; this module only translates those semantic
//! properties into libclang arguments.

use std::collections::HashSet;
use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};

pub fn load(path: &Path) -> Result<Vec<String>> {
    let contents = std::fs::read_to_string(path)
        .with_context(|| format!("reading parse context {}", path.display()))?;
    let mut args = Vec::new();
    let mut framework_roots = HashSet::new();

    for (index, raw_line) in contents.lines().enumerate() {
        let line = raw_line.trim_end_matches('\r');
        if line.is_empty() || line.starts_with('#') {
            continue;
        }

        let Some((kind, value)) = line.split_once('=') else {
            bail!(
                "{}:{}: expected a kind=value record",
                path.display(),
                index + 1
            );
        };

        match kind {
            "standard" => push_joined(&mut args, "-std=", value, path, index)?,
            "include" => {
                push_joined(&mut args, "-I", value, path, index)?;
                if let Some(root) = framework_root(Path::new(value)) {
                    if framework_roots.insert(root.clone()) {
                        args.push(format!("-F{}", root.display()));
                    }
                }
            }
            "framework" => push_joined(&mut args, "-F", value, path, index)?,
            "define" => push_joined(&mut args, "-D", value, path, index)?,
            "undefine" => push_joined(&mut args, "-U", value, path, index)?,
            "forced-include" => push_pair(&mut args, "-include", value, path, index)?,
            "sysroot" => push_pair(&mut args, "-isysroot", value, path, index)?,
            "target" => push_joined(&mut args, "--target=", value, path, index)?,
            _ => bail!(
                "{}:{}: unknown parse-context record kind {:?}",
                path.display(),
                index + 1,
                kind
            ),
        }
    }

    Ok(args)
}

fn push_joined(
    args: &mut Vec<String>,
    prefix: &str,
    value: &str,
    path: &Path,
    index: usize,
) -> Result<()> {
    require_value(value, path, index)?;
    args.push(format!("{prefix}{value}"));
    Ok(())
}

fn push_pair(
    args: &mut Vec<String>,
    option: &str,
    value: &str,
    path: &Path,
    index: usize,
) -> Result<()> {
    require_value(value, path, index)?;
    args.push(option.to_string());
    args.push(value.to_string());
    Ok(())
}

fn require_value(value: &str, path: &Path, index: usize) -> Result<()> {
    if value.is_empty() {
        bail!(
            "{}:{}: parse-context value must not be empty",
            path.display(),
            index + 1
        );
    }
    Ok(())
}

fn framework_root(include: &Path) -> Option<PathBuf> {
    if include.file_name()? != "Headers" {
        return None;
    }
    let framework = include.parent()?;
    if framework.extension()? != "framework" {
        return None;
    }
    framework.parent().map(Path::to_path_buf)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn loads_typed_semantic_context() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("parse-context.txt");
        std::fs::write(
            &path,
            "standard=gnu++20\ninclude=/project/include\ndefine=HAVE_RPC=1\n\
             forced-include=config.h\nsysroot=/sdk\ntarget=aarch64-linux-gnu\n",
        )
        .unwrap();

        assert_eq!(
            load(&path).unwrap(),
            vec![
                "-std=gnu++20",
                "-I/project/include",
                "-DHAVE_RPC=1",
                "-include",
                "config.h",
                "-isysroot",
                "/sdk",
                "--target=aarch64-linux-gnu"
            ]
        );
    }

    #[test]
    fn derives_framework_search_root_from_cmake_include_directory() {
        let dir = tempfile::tempdir().unwrap();
        let path = dir.path().join("parse-context.txt");
        std::fs::write(
            &path,
            "include=/opt/qt/lib/QtCore.framework/Headers\n\
             include=/opt/qt/lib/QtDBus.framework/Headers\n",
        )
        .unwrap();

        assert_eq!(
            load(&path).unwrap(),
            vec![
                "-I/opt/qt/lib/QtCore.framework/Headers",
                "-F/opt/qt/lib",
                "-I/opt/qt/lib/QtDBus.framework/Headers"
            ]
        );
    }
}
