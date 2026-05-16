use anyhow::{Context, Result};
use chrono::Local;
use git2::Repository;
use serde_json::{json, Map, Value};
use std::path::PathBuf;
use tracing::{info, warn};

use crate::s3::S3Store;

pub struct Catalog {
    storage: S3Store,
    dry: bool,
    regenerate: bool,
    catalog: Value,
    catalog_filename: String,
}

impl Catalog {
    pub fn new(storage: S3Store, branch: &str, dry: bool, regenerate: bool) -> Self {
        Self {
            storage,
            dry,
            regenerate,
            catalog: Value::Null,
            catalog_filename: format!("{}/catalog.json", branch.trim_end_matches('/')),
        }
    }

    pub async fn load(&mut self) -> Result<()> {
        if !self.regenerate && self.storage.file_exists(&self.catalog_filename).await? {
            self.catalog = self.storage.read_json(&self.catalog_filename).await?;
        } else {
            self.catalog = json!({});
        }
        if !self
            .catalog
            .get("builds")
            .map(|v| v.is_array())
            .unwrap_or(false)
        {
            self.catalog
                .as_object_mut()
                .unwrap()
                .insert("builds".to_string(), Value::Array(vec![]));
        }
        Ok(())
    }

    pub async fn save(&self) -> Result<()> {
        self.save_as(&self.catalog_filename).await
    }

    pub async fn save_as(&self, filename: &str) -> Result<()> {
        if self.dry {
            info!("Dry run: would save {filename}");
            println!("{}", serde_json::to_string_pretty(&self.catalog)?);
            return Ok(());
        }
        info!("Saving: {filename}");
        self.storage.write_json(filename, &self.catalog).await
    }

    pub fn builds(&self) -> &[Value] {
        self.catalog
            .get("builds")
            .and_then(|v| v.as_array())
            .map(|v| v.as_slice())
            .unwrap_or(&[])
    }

    pub fn builds_mut(&mut self) -> &mut Vec<Value> {
        self.catalog
            .as_object_mut()
            .unwrap()
            .get_mut("builds")
            .unwrap()
            .as_array_mut()
            .unwrap()
    }

    pub fn set_builds(&mut self, builds: Vec<Value>) {
        self.catalog
            .as_object_mut()
            .unwrap()
            .insert("builds".to_string(), Value::Array(builds));
    }
}

pub struct Builder {
    storage: S3Store,
    catalog: Catalog,
    git_root: PathBuf,
    branch: String,
    dry: bool,
    regenerate: bool,
}

impl Builder {
    pub fn new(
        storage: S3Store,
        catalog: Catalog,
        git_root: PathBuf,
        branch: String,
        dry: bool,
        regenerate: bool,
    ) -> Self {
        Self {
            storage,
            catalog,
            git_root,
            branch,
            dry,
            regenerate,
        }
    }

    pub async fn load(&mut self) -> Result<()> {
        self.catalog.load().await
    }

    pub async fn save(&self) -> Result<()> {
        self.catalog.save().await
    }

    pub async fn process(&mut self) -> Result<()> {
        self.merge_catalogs().await?;
        self.fixups();
        self.deduplicate_builds();
        self.sort_catalog();
        self.remove_orphans().await?;
        self.update_git_logs()?;
        Ok(())
    }

    fn merge_build(&mut self, part: Value) {
        let part_id = part.get("id").cloned();
        let builds = self.catalog.builds_mut();
        let pos = builds.iter().position(|b| b.get("id") == part_id.as_ref());
        match pos {
            Some(idx) => {
                let merged = merge_value(builds[idx].clone(), part);
                builds[idx] = merged;
            }
            None => builds.push(part),
        }
    }

    fn is_catalog(&self, filename: &str) -> bool {
        if !filename.ends_with(".json") {
            return false;
        }
        if filename.starts_with("job-catalog-") {
            return true;
        }
        if self.regenerate && filename.starts_with(".job-catalog-") {
            return true;
        }
        false
    }

    async fn merge_catalogs(&mut self) -> Result<()> {
        let timestamp = Local::now().format("%Y%m%d-%H%M%S");
        let backup_filename = format!(
            "{}/{}-catalog.json",
            self.branch.trim_end_matches('/'),
            timestamp
        );
        if self.dry {
            info!("Dry run: writeJson({backup_filename})");
        } else {
            self.catalog.save_as(&backup_filename).await?;
        }

        let files = self.storage.list(&self.branch).await?;
        for file in &files {
            let key = match file.key() {
                Some(k) => k.to_string(),
                None => continue,
            };
            let filename = key.rsplit('/').next().unwrap_or(&key).to_string();
            let directory = match key.rsplit_once('/') {
                Some((dir, _)) => dir.to_string(),
                None => String::new(),
            };
            info!("Processing {filename}");
            if !self.is_catalog(&filename) {
                continue;
            }
            let part = self.storage.read_json(&key).await?;
            let first_build = part
                .get("builds")
                .and_then(|v| v.as_array())
                .and_then(|a| a.first())
                .cloned();
            if let Some(b) = first_build {
                self.merge_build(b);
            }
            let backup_filename = if directory.is_empty() {
                format!(".{filename}")
            } else {
                format!("{directory}/.{filename}")
            };
            if self.dry || self.regenerate {
                info!("Dry run: not creating backup of {key}");
            } else {
                self.storage.write_json(&backup_filename, &part).await?;
                self.storage.delete_object(&key).await?;
            }
        }
        Ok(())
    }

    fn update_git_logs(&mut self) -> Result<()> {
        if self.catalog.builds().len() <= 1 {
            return Ok(());
        }
        let head_hash = match self.catalog.builds()[0]
            .get("hash")
            .and_then(|v| v.as_str())
        {
            Some(h) => h.to_string(),
            None => return Ok(()),
        };

        let repo = Repository::open(&self.git_root)
            .with_context(|| format!("opening git repo at {}", self.git_root.display()))?;
        let head_oid = git2::Oid::from_str(&head_hash)
            .with_context(|| format!("invalid head hash {head_hash}"))?;

        let mut walk = repo.revwalk()?;
        walk.push(head_oid)?;
        walk.set_sorting(git2::Sort::TIME)?;

        let mut build_index = 0usize;
        let builds = self.catalog.builds_mut();
        builds[build_index]
            .as_object_mut()
            .unwrap()
            .insert("commits".to_string(), Value::Array(vec![]));

        for oid_res in walk {
            if build_index >= builds.len() - 1 {
                break;
            }
            let oid = oid_res?;
            let oid_str = oid.to_string();
            let next_hash = builds[build_index + 1]
                .get("hash")
                .and_then(|v| v.as_str())
                .unwrap_or("");
            info!("Comparing {} with {}", oid_str, next_hash);
            if !next_hash.is_empty() && oid_str.starts_with(next_hash) {
                build_index += 1;
                builds[build_index]
                    .as_object_mut()
                    .unwrap()
                    .insert("commits".to_string(), Value::Array(vec![]));
                continue;
            }
            let commit = repo.find_commit(oid)?;
            let commit_value = commit_to_value(&commit, &oid_str);
            if let Some(arr) = builds[build_index]
                .as_object_mut()
                .unwrap()
                .get_mut("commits")
                .and_then(|v| v.as_array_mut())
            {
                arr.push(commit_value);
            }
        }
        Ok(())
    }

    async fn remove_orphans(&mut self) -> Result<()> {
        let mut new_builds: Vec<Value> = Vec::new();
        let builds = self.catalog.builds().to_vec();
        for build in builds {
            let mut kept_artifacts: Vec<Value> = Vec::new();
            let artifacts = build
                .get("artifacts")
                .and_then(|v| v.as_array())
                .cloned()
                .unwrap_or_default();
            for artifact in artifacts {
                let url = artifact.get("url").and_then(|v| v.as_str()).unwrap_or("");
                let filename = url.replacen("snapshots/", "", 1);
                info!("Checking for artifact {filename}");
                let exists = match self.storage.file_exists(&filename).await {
                    Ok(b) => b,
                    Err(e) => {
                        warn!("Exception while checking for artifact {filename}: {e}");
                        false
                    }
                };
                if exists {
                    kept_artifacts.push(artifact);
                } else {
                    info!("Artifact {filename} disappeared");
                }
            }
            if kept_artifacts.is_empty() {
                let id = build.get("id").and_then(|v| v.as_str()).unwrap_or("?");
                info!("Build {id} does not have any artifacts");
                continue;
            }
            let mut updated = build;
            if let Some(obj) = updated.as_object_mut() {
                obj.insert("artifacts".to_string(), Value::Array(kept_artifacts));
            }
            new_builds.push(updated);
        }
        self.catalog.set_builds(new_builds);
        Ok(())
    }

    fn fixups(&mut self) {
        for build in self.catalog.builds_mut() {
            if build.get("channel").and_then(|v| v.as_str()) == Some("nightly") {
                build
                    .as_object_mut()
                    .unwrap()
                    .insert("channel".to_string(), Value::String("dev".to_string()));
            }
            if let Some(artifacts) = build.get_mut("artifacts").and_then(|v| v.as_array_mut()) {
                for artifact in artifacts {
                    let map = match artifact.as_object_mut() {
                        Some(m) => m,
                        None => continue,
                    };
                    fixup_path_field(map, "url");
                    fixup_path_field(map, "filename");
                    if let Some(Value::String(p)) = map.get_mut("platform") {
                        *p = p.replace("win32", "windows");
                    }
                    if let Some(Value::String(a)) = map.get_mut("arch") {
                        *a = a.replace("ia32", "x86");
                    }
                }
            }
        }
    }

    fn deduplicate_builds(&mut self) {
        let builds = self.catalog.builds().to_vec();
        let original_len = builds.len();
        let mut deduped = deduplicate_build_values(builds);
        if deduped.len() < original_len {
            info!(
                "Deduplicated builds: removed {} older duplicate(s)",
                original_len - deduped.len()
            );
        }

        for build in deduped.iter_mut() {
            if let Some((original_artifact_len, artifacts)) = deduplicate_artifacts_for_build(build)
            {
                if artifacts.len() < original_artifact_len {
                    let id = build
                        .get("id")
                        .and_then(|v| v.as_str())
                        .unwrap_or("?")
                        .to_string();
                    info!(
                        "Deduplicated artifacts in build {id}: removed {} duplicate(s)",
                        original_artifact_len - artifacts.len()
                    );
                    build
                        .as_object_mut()
                        .unwrap()
                        .insert("artifacts".to_string(), Value::Array(artifacts));
                }
            }
        }

        self.catalog.set_builds(deduped);
    }

    fn sort_catalog(&mut self) {
        let mut builds = self.catalog.builds().to_vec();
        builds.sort_by(|a, b| {
            let a_date = a.get("date").and_then(|v| v.as_str()).unwrap_or("");
            let b_date = b.get("date").and_then(|v| v.as_str()).unwrap_or("");
            b_date.cmp(a_date)
        });
        self.catalog.set_builds(builds);
    }
}

// Mirrors mergician({appendArrays: true, beforeEach: keep target.notes if src.notes is empty}).
fn merge_value(target: Value, src: Value) -> Value {
    match (target, src) {
        (Value::Object(mut t), Value::Object(s)) => {
            for (k, v) in s {
                if k == "notes" {
                    let src_empty = match &v {
                        Value::Null => true,
                        Value::String(s) => s.is_empty(),
                        Value::Array(a) => a.is_empty(),
                        _ => false,
                    };
                    if src_empty {
                        continue;
                    }
                    t.insert(k, v);
                    continue;
                }
                let existing = t.remove(&k);
                let merged = match existing {
                    Some(e) => merge_value(e, v),
                    None => v,
                };
                t.insert(k, merged);
            }
            Value::Object(t)
        }
        (Value::Array(mut t), Value::Array(s)) => {
            t.extend(s);
            Value::Array(t)
        }
        (_, src) => src,
    }
}

fn fixup_path_field(map: &mut Map<String, Value>, key: &str) {
    if let Some(Value::String(s)) = map.get_mut(key) {
        *s = s.replace("/workspace/source/_deploy", "");
        *s = s.replace("//", "/");
    }
}

fn deduplicate_build_values(builds: Vec<Value>) -> Vec<Value> {
    use std::collections::HashMap;

    let mut newest_by_key: HashMap<String, usize> = HashMap::new();
    for (i, build) in builds.iter().enumerate() {
        let Some(key) = build_dedup_key(build) else {
            continue;
        };
        let new_date = build.get("date").and_then(|v| v.as_str()).unwrap_or("");
        match newest_by_key.get(&key) {
            Some(&prev_i) => {
                let prev_date = builds[prev_i]
                    .get("date")
                    .and_then(|v| v.as_str())
                    .unwrap_or("");
                if date_is_newer(new_date, prev_date) {
                    newest_by_key.insert(key, i);
                }
            }
            None => {
                newest_by_key.insert(key, i);
            }
        }
    }

    builds
        .into_iter()
        .enumerate()
        .filter(|(i, build)| match build_dedup_key(build) {
            Some(key) => newest_by_key.get(&key) == Some(i),
            None => true,
        })
        .map(|(_, build)| build)
        .collect()
}

fn build_dedup_key(build: &Value) -> Option<String> {
    let tag = build.get("tag").and_then(|v| v.as_str())?;
    if tag.is_empty() {
        return None;
    }
    Some(format!("{tag}:{}", increment_key(build.get("increment"))))
}

fn increment_key(value: Option<&Value>) -> String {
    match value {
        Some(Value::Null) | None => String::new(),
        Some(Value::String(s)) => s.clone(),
        Some(Value::Number(n)) => n.to_string(),
        Some(Value::Bool(b)) => b.to_string(),
        Some(other) => other.to_string(),
    }
}

fn date_is_newer(new_date: &str, prev_date: &str) -> bool {
    match (
        chrono::DateTime::parse_from_rfc3339(new_date),
        chrono::DateTime::parse_from_rfc3339(prev_date),
    ) {
        (Ok(new_date), Ok(prev_date)) => new_date > prev_date,
        _ => false,
    }
}

fn deduplicate_artifacts_for_build(build: &Value) -> Option<(usize, Vec<Value>)> {
    let artifacts = build.get("artifacts").and_then(|v| v.as_array())?.clone();
    let original_len = artifacts.len();
    let mut latest: Vec<(String, Value)> = Vec::new();
    for artifact in artifacts {
        let key = artifact
            .get("url")
            .and_then(|v| v.as_str())
            .unwrap_or("")
            .to_string();
        let new_lastmod = artifact
            .get("lastmod")
            .and_then(|v| v.as_str())
            .unwrap_or("");
        match latest
            .iter_mut()
            .find(|(existing_key, _)| existing_key == &key)
        {
            Some((_, prev)) => {
                let prev_lastmod = prev.get("lastmod").and_then(|v| v.as_str()).unwrap_or("");
                if !new_lastmod.is_empty()
                    && (prev_lastmod.is_empty() || new_lastmod > prev_lastmod)
                {
                    *prev = artifact;
                }
            }
            None => {
                latest.push((key, artifact));
            }
        }
    }
    Some((
        original_len,
        latest.into_iter().map(|(_, artifact)| artifact).collect(),
    ))
}

fn commit_to_value(commit: &git2::Commit<'_>, oid: &str) -> Value {
    let mut commit_value = Map::new();
    commit_value.insert(
        "message".to_string(),
        Value::String(String::from_utf8_lossy(commit.message_bytes()).into_owned()),
    );
    commit_value.insert(
        "parent".to_string(),
        Value::Array(
            commit
                .parent_ids()
                .map(|parent| Value::String(parent.to_string()))
                .collect(),
        ),
    );
    commit_value.insert(
        "tree".to_string(),
        Value::String(commit.tree_id().to_string()),
    );
    commit_value.insert("author".to_string(), signature_to_value(&commit.author()));
    commit_value.insert(
        "committer".to_string(),
        signature_to_value(&commit.committer()),
    );
    if let Some(gpgsig) = gpgsig_from_raw_header(commit.raw_header().unwrap_or("")) {
        commit_value.insert("gpgsig".to_string(), Value::String(gpgsig));
    }

    let mut value = Map::new();
    value.insert("oid".to_string(), Value::String(oid.to_string()));
    value.insert("commit".to_string(), Value::Object(commit_value));
    value.insert("payload".to_string(), Value::String(commit_payload(commit)));
    Value::Object(value)
}

fn signature_to_value(signature: &git2::Signature<'_>) -> Value {
    json!({
        "name": signature.name().unwrap_or(""),
        "email": signature.email().unwrap_or(""),
        "timestamp": signature.when().seconds(),
        "timezoneOffset": -signature.when().offset_minutes(),
    })
}

fn commit_payload(commit: &git2::Commit<'_>) -> String {
    let header = String::from_utf8_lossy(commit.raw_header_bytes());
    let message = String::from_utf8_lossy(commit.message_bytes());
    commit_payload_from_parts(&header, &message)
}

fn commit_payload_from_parts(header: &str, message: &str) -> String {
    format!("{}\n\n{}", raw_header_without_gpgsig(header), message)
}

fn raw_header_without_gpgsig(header: &str) -> String {
    let mut lines = Vec::new();
    let mut in_signature = false;

    for line in header.lines() {
        if line.starts_with("gpgsig ") {
            in_signature = true;
            continue;
        }
        if in_signature {
            if line.starts_with(' ') {
                continue;
            }
            in_signature = false;
        }
        lines.push(line);
    }

    lines.join("\n")
}

fn gpgsig_from_raw_header(header: &str) -> Option<String> {
    let mut signature = Vec::new();
    let mut in_signature = false;

    for line in header.lines() {
        if let Some(first_line) = line.strip_prefix("gpgsig ") {
            in_signature = true;
            signature.push(first_line.to_string());
            continue;
        }
        if in_signature {
            if let Some(continuation) = line.strip_prefix(' ') {
                signature.push(continuation.to_string());
            } else {
                break;
            }
        }
    }

    if signature.is_empty() {
        None
    } else {
        Some(signature.join("\n"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    fn ids(builds: &[Value]) -> Vec<&str> {
        builds
            .iter()
            .map(|build| build.get("id").and_then(|v| v.as_str()).unwrap_or(""))
            .collect()
    }

    #[test]
    fn deduplicate_builds_keeps_same_tag_with_different_increments() {
        let builds = vec![
            json!({
                "id": "release",
                "tag": "v1.11.0",
                "increment": "0",
                "date": "2026-01-01T00:00:00Z"
            }),
            json!({
                "id": "dev-old",
                "tag": "v1.11.0",
                "increment": "1",
                "date": "2026-01-02T00:00:00Z"
            }),
            json!({
                "id": "dev-new",
                "tag": "v1.11.0",
                "increment": "1",
                "date": "2026-01-03T00:00:00Z"
            }),
        ];

        let deduped = deduplicate_build_values(builds);

        assert_eq!(ids(&deduped), vec!["release", "dev-new"]);
    }

    #[test]
    fn deduplicate_builds_treats_missing_and_empty_increment_as_same_key() {
        let builds = vec![
            json!({
                "id": "old",
                "tag": "v1.11.0",
                "date": "2026-01-01T00:00:00Z"
            }),
            json!({
                "id": "new",
                "tag": "v1.11.0",
                "increment": "",
                "date": "2026-01-02T00:00:00Z"
            }),
        ];

        let deduped = deduplicate_build_values(builds);

        assert_eq!(ids(&deduped), vec!["new"]);
    }

    #[test]
    fn deduplicate_builds_matches_typescript_date_parse_failure_behavior() {
        let builds = vec![
            json!({
                "id": "first-invalid",
                "tag": "v1.11.0",
                "date": "not-a-date"
            }),
            json!({
                "id": "newer-valid",
                "tag": "v1.11.0",
                "date": "2026-01-02T00:00:00Z"
            }),
        ];

        let deduped = deduplicate_build_values(builds);

        assert_eq!(ids(&deduped), vec!["first-invalid"]);
    }

    #[test]
    fn deduplicate_builds_keeps_untagged_builds() {
        let builds = vec![
            json!({
                "id": "untagged-a",
                "date": "2026-01-01T00:00:00Z"
            }),
            json!({
                "id": "untagged-b",
                "tag": "",
                "date": "2026-01-02T00:00:00Z"
            }),
            json!({
                "id": "tagged",
                "tag": "v1.11.0",
                "date": "2026-01-03T00:00:00Z"
            }),
        ];

        let deduped = deduplicate_build_values(builds);

        assert_eq!(ids(&deduped), vec!["untagged-a", "untagged-b", "tagged"]);
    }

    #[test]
    fn deduplicate_artifacts_keeps_latest_by_url() {
        let build = json!({
            "id": "build",
            "artifacts": [
                {"url": "snapshots/a.exe", "lastmod": "2026-01-01T00:00:00Z", "size": 1},
                {"url": "snapshots/b.exe", "lastmod": "2026-01-01T00:00:00Z", "size": 3},
                {"url": "snapshots/a.exe", "lastmod": "2026-01-02T00:00:00Z", "size": 2}
            ]
        });

        let (original_len, artifacts) = deduplicate_artifacts_for_build(&build).unwrap();
        let sizes: Vec<i64> = artifacts
            .iter()
            .map(|artifact| artifact.get("size").and_then(|v| v.as_i64()).unwrap())
            .collect();

        assert_eq!(original_len, 3);
        assert_eq!(sizes, vec![2, 3]);
    }

    #[test]
    fn merge_value_preserves_existing_notes_when_new_notes_are_empty() {
        let target = json!({
            "id": "build",
            "notes": "existing notes",
            "artifacts": [{"url": "a"}]
        });
        let src = json!({
            "notes": "",
            "artifacts": [{"url": "b"}]
        });

        let merged = merge_value(target, src);

        assert_eq!(
            merged.get("notes").and_then(|v| v.as_str()),
            Some("existing notes")
        );
        assert_eq!(
            merged
                .get("artifacts")
                .and_then(|v| v.as_array())
                .unwrap()
                .len(),
            2
        );
    }

    #[test]
    fn commit_to_value_matches_isomorphic_git_log_shape() {
        let unique = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        let path =
            std::env::temp_dir().join(format!("ship-commit-test-{}-{unique}", std::process::id()));
        std::fs::create_dir_all(&path).unwrap();

        let repo = Repository::init(&path).unwrap();
        let author = git2::Signature::new(
            "Author Name",
            "author@example.com",
            &git2::Time::new(1_700_000_000, 120),
        )
        .unwrap();
        let committer = git2::Signature::new(
            "Committer Name",
            "committer@example.com",
            &git2::Time::new(1_700_000_100, 60),
        )
        .unwrap();
        let tree_id = {
            let mut index = repo.index().unwrap();
            index.write_tree().unwrap()
        };
        let tree = repo.find_tree(tree_id).unwrap();
        let oid = repo
            .commit(
                Some("HEAD"),
                &author,
                &committer,
                "commit subject\n\nbody line\n",
                &tree,
                &[],
            )
            .unwrap();
        let commit = repo.find_commit(oid).unwrap();

        let value = commit_to_value(&commit, &oid.to_string());
        let commit_value = value.get("commit").unwrap();

        assert_eq!(
            value.get("oid").and_then(|v| v.as_str()).unwrap(),
            oid.to_string()
        );
        assert_eq!(
            commit_value.get("message").and_then(|v| v.as_str()),
            Some("commit subject\n\nbody line\n")
        );
        assert_eq!(
            commit_value
                .get("parent")
                .and_then(|v| v.as_array())
                .unwrap()
                .len(),
            0
        );
        assert_eq!(
            commit_value.get("tree").and_then(|v| v.as_str()).unwrap(),
            tree_id.to_string()
        );
        assert_eq!(
            commit_value
                .get("author")
                .and_then(|v| v.get("timezoneOffset"))
                .and_then(|v| v.as_i64()),
            Some(-120)
        );
        assert_eq!(
            commit_value
                .get("committer")
                .and_then(|v| v.get("timezoneOffset"))
                .and_then(|v| v.as_i64()),
            Some(-60)
        );
        assert!(value
            .get("payload")
            .and_then(|v| v.as_str())
            .unwrap()
            .contains("author Author Name <author@example.com> 1700000000 +0200"));

        drop(commit);
        drop(tree);
        drop(repo);
        std::fs::remove_dir_all(&path).unwrap();
    }

    #[test]
    fn signed_commit_helpers_match_isomorphic_git_shape() {
        let header = r#"tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Rob Caelers <robc@example.com> 1700000000 +0100
committer Rob Caelers <robc@example.com> 1700000001 +0100
gpgsig -----BEGIN PGP SIGNATURE-----
 line one
 line two
 -----END PGP SIGNATURE-----"#;
        let message = "Signed commit\n";

        assert_eq!(
            gpgsig_from_raw_header(header).as_deref(),
            Some("-----BEGIN PGP SIGNATURE-----\nline one\nline two\n-----END PGP SIGNATURE-----")
        );
        assert_eq!(
            commit_payload_from_parts(header, message),
            "tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904\n\
author Rob Caelers <robc@example.com> 1700000000 +0100\n\
committer Rob Caelers <robc@example.com> 1700000001 +0100\n\n\
Signed commit\n"
        );
    }
}

pub struct CatalogOptions {
    pub branch: String,
    pub bucket: String,
    pub key: String,
    pub secret: String,
    pub workspace: PathBuf,
    pub endpoint: String,
    pub dry: bool,
    pub regenerate: bool,
}

pub async fn run_catalog(opts: CatalogOptions) -> Result<()> {
    let storage_for_catalog = S3Store::new(&opts.endpoint, &opts.bucket, &opts.key, &opts.secret);
    let storage_for_builder = S3Store::new(&opts.endpoint, &opts.bucket, &opts.key, &opts.secret);
    let catalog = Catalog::new(storage_for_catalog, &opts.branch, opts.dry, opts.regenerate);
    let mut builder = Builder::new(
        storage_for_builder,
        catalog,
        opts.workspace,
        opts.branch.clone(),
        opts.dry,
        opts.regenerate,
    );
    builder.load().await?;
    builder.process().await?;
    builder.save().await?;
    Ok(())
}
