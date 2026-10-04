//! Shared file-pattern and legacy boolean helpers.
use anyhow::{Context, Result};

pub fn truthy(rendered: &str) -> bool {
    !matches!(
        rendered.trim().to_ascii_lowercase().as_str(),
        "" | "false" | "0" | "none" | "null" | "no"
    )
}

pub fn glob_paths(pattern: &str) -> Result<Vec<String>> {
    let mut paths: Vec<String> = glob::glob(pattern)
        .with_context(|| format!("invalid glob pattern {pattern}"))?
        .filter_map(|p| p.ok())
        .map(|p| p.to_string_lossy().into_owned())
        .collect();
    paths.sort();
    Ok(paths)
}
