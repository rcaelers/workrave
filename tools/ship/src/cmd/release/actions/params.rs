//! Helpers for reading a step's `with:` parameters.

use std::path::{Path, PathBuf};

use anyhow::{anyhow, Result};
use serde::Deserialize;

use crate::cmd::release::context::{glob_paths, truthy};

/// Deserializes `with` into the action's parameter struct. A missing `with`
/// is an empty mapping.
pub fn params<T: for<'de> Deserialize<'de>>(with: &serde_yaml::Value) -> Result<T> {
    let value = if with.is_null() {
        serde_yaml::Value::Mapping(Default::default())
    } else {
        with.clone()
    };
    serde_yaml::from_value(value).map_err(|e| anyhow!("invalid `with` parameters: {e}"))
}

/// A parameter that may be written as a bool or as a (templated) string.
#[derive(Debug, Clone, Deserialize)]
#[serde(untagged)]
pub enum Flag {
    Bool(bool),
    Text(String),
}

impl Default for Flag {
    fn default() -> Self {
        Flag::Bool(false)
    }
}

impl Flag {
    pub fn value(&self) -> bool {
        match self {
            Flag::Bool(b) => *b,
            Flag::Text(s) => truthy(s),
        }
    }
}

/// The S3-compatible bucket an action talks to.
#[derive(Debug, Clone, Deserialize)]
pub struct BucketParams {
    /// e.g. `https://snapshots.workrave.org/`
    pub endpoint: String,
    pub bucket: String,
    #[serde(rename = "access-key")]
    pub access_key: String,
    #[serde(rename = "secret-key")]
    pub secret_key: String,
}

impl BucketParams {
    pub fn store(&self) -> crate::services::s3::S3Store {
        crate::services::s3::S3Store::new(
            &self.endpoint,
            &self.bucket,
            &self.access_key,
            &self.secret_key,
        )
    }
}

/// A parameter that may be one string or a list of strings.
#[derive(Debug, Clone, Deserialize)]
#[serde(untagged)]
pub enum StringOrList {
    One(String),
    Many(Vec<String>),
}

impl StringOrList {
    pub fn items(&self) -> Vec<String> {
        match self {
            StringOrList::One(s) => vec![s.clone()],
            StringOrList::Many(v) => v.clone(),
        }
    }

    /// Expands every item as a glob pattern; an item without a match is kept
    /// as-is when it exists as a path.
    pub fn paths(&self) -> Result<Vec<PathBuf>> {
        let mut paths = Vec::new();
        for item in self.items() {
            let matches = glob_paths(&item)?;
            if matches.is_empty() {
                if Path::new(&item).exists() {
                    paths.push(PathBuf::from(item));
                }
            } else {
                paths.extend(matches.into_iter().map(PathBuf::from));
            }
        }
        Ok(paths)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn flags_and_lists() {
        assert!(Flag::Bool(true).value());
        assert!(!Flag::Text("false".into()).value());
        assert!(Flag::Text("true".into()).value());
        assert_eq!(StringOrList::One("a".into()).items(), vec!["a"]);
        assert_eq!(
            StringOrList::Many(vec!["a".into(), "b".into()]).items(),
            vec!["a", "b"]
        );
    }

    #[test]
    fn missing_with_is_empty_mapping() {
        #[derive(Deserialize)]
        struct P {
            #[serde(default)]
            x: String,
        }
        let p: P = params(&serde_yaml::Value::Null).unwrap();
        assert_eq!(p.x, "");
    }
}
