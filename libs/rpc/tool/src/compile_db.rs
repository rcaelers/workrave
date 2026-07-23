//! Resolves the real compiler flags (include paths, defines, `-std=`, ...)
//! for an annotated header by looking up its "anchor" translation unit (a
//! `.cc` that `#include`s it) in `compile_commands.json`.
//!
//! Headers are not translation units, so they're never keys in
//! `compile_commands.json` themselves — only `.cc`/`.cpp` files are. Reusing
//! the anchor's flags is the standard way clang-based tools solve this (the
//! same trick `clang-tidy`/`clangd` use for header-only edits).

use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};
use serde::Deserialize;

#[derive(Debug, Deserialize)]
struct CompileCommand {
    directory: String,
    file: String,
    #[serde(default)]
    arguments: Option<Vec<String>>,
    #[serde(default)]
    command: Option<String>,
}

/// Returns the compiler flags to use for parsing `anchor_source`'s header(s)
/// — the compiler executable, the source file itself, `-c`, and `-o <file>`
/// have all been stripped, since we reuse these flags against a different
/// input file (the header) than the one they were recorded for.
pub fn resolve_flags(compile_commands: &Path, anchor_source: &Path) -> Result<Vec<String>> {
    let data = std::fs::read_to_string(compile_commands)
        .with_context(|| format!("reading {}", compile_commands.display()))?;
    let commands: Vec<CompileCommand> = serde_json::from_str(&data)
        .with_context(|| format!("parsing {}", compile_commands.display()))?;

    let anchor = anchor_source
        .canonicalize()
        .with_context(|| format!("resolving anchor source {}", anchor_source.display()))?;

    for cmd in &commands {
        let file = PathBuf::from(&cmd.file);
        let file_abs = if file.is_absolute() {
            file
        } else {
            PathBuf::from(&cmd.directory).join(&file)
        };
        let Ok(file_abs) = file_abs.canonicalize() else {
            continue;
        };
        if file_abs == anchor {
            let raw_args = if let Some(args) = &cmd.arguments {
                args.clone()
            } else if let Some(command) = &cmd.command {
                shell_words::split(command)
                    .with_context(|| format!("splitting shell command for {}", cmd.file))?
            } else {
                bail!(
                    "compile_commands.json entry for {} has neither 'arguments' nor 'command'",
                    cmd.file
                );
            };
            return Ok(filter_flags(&raw_args, &cmd.file));
        }
    }

    bail!(
        "no entry for anchor source {} found in {}",
        anchor_source.display(),
        compile_commands.display()
    )
}

fn filter_flags(args: &[String], source_file: &str) -> Vec<String> {
    let mut out = Vec::new();
    let mut iter = args.iter().enumerate().peekable();
    while let Some((i, arg)) = iter.next() {
        if i == 0 {
            continue; // compiler executable
        }
        if arg == "-c" || arg == source_file {
            continue;
        }
        if arg == "-o" {
            iter.next();
            continue;
        }
        if let Some(stripped) = arg.strip_prefix("-o") {
            if !stripped.is_empty() {
                continue;
            }
        }
        out.push(arg.clone());
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;

    #[test]
    fn resolves_and_filters_flags() {
        let dir = tempfile::tempdir().unwrap();
        let src = dir.path().join("Core.cc");
        std::fs::write(&src, "// anchor\n").unwrap();

        let db_path = dir.path().join("compile_commands.json");
        let db = format!(
            r#"[{{"directory": "{}", "file": "Core.cc", "arguments": ["/usr/bin/c++", "-std=c++20", "-I/foo/include", "-DBAR=1", "-c", "-o", "Core.cc.o", "Core.cc"]}}]"#,
            dir.path().display()
        );
        let mut f = std::fs::File::create(&db_path).unwrap();
        f.write_all(db.as_bytes()).unwrap();

        let flags = resolve_flags(&db_path, &src).unwrap();
        assert_eq!(flags, vec!["-std=c++20", "-I/foo/include", "-DBAR=1"]);
    }

    /// Regression test: CMake/Ninja can emit compile_commands.json entries as
    /// a single shell-quoted `command` string (rather than an `arguments`
    /// array), and real invocations from this project include backslash-escaped
    /// quoted defines like `-DFOO=\"\"` (from CMake's `WORKRAVE_BINDIR32=""`).
    /// A naive whitespace split mangles these; shell_words must not.
    #[test]
    fn resolves_flags_from_quoted_command_string() {
        let dir = tempfile::tempdir().unwrap();
        let src = dir.path().join("Core.cc");
        std::fs::write(&src, "// anchor\n").unwrap();

        let db_path = dir.path().join("compile_commands.json");
        let db = format!(
            r#"[{{"directory": "{}", "file": "Core.cc", "command": "/usr/bin/c++ -DWORKRAVE_BINDIR32=\"\" -DWORKRAVE_NAME=\"a b\" -std=gnu++20 -c -o Core.cc.o Core.cc"}}]"#,
            dir.path().display()
        );
        std::fs::write(&db_path, db).unwrap();

        let flags = resolve_flags(&db_path, &src).unwrap();
        assert_eq!(
            flags,
            vec![
                "-DWORKRAVE_BINDIR32=",
                "-DWORKRAVE_NAME=a b",
                "-std=gnu++20"
            ]
        );
    }
}
