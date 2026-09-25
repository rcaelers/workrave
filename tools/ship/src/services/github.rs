//! GitHub releases through the REST API.

use std::path::Path;

use anyhow::{anyhow, bail, Context, Result};
use reqwest::header::{ACCEPT, AUTHORIZATION, CONTENT_TYPE, USER_AGENT};
use reqwest::{Client, Response};
use serde::Deserialize;

const API: &str = "https://api.github.com";
const UPLOADS: &str = "https://uploads.github.com";

#[derive(Debug, Clone, Deserialize)]
pub struct Release {
    pub id: u64,
    pub tag_name: String,
    pub html_url: String,
}

pub struct GitHub {
    client: Client,
    token: String,
    owner: String,
    repo: String,
}

impl GitHub {
    /// `repo_url` is the clone URL, e.g. `https://github.com/rcaelers/workrave.git`.
    pub fn new(repo_url: &str, token: &str) -> Result<GitHub> {
        let (owner, repo) = parse_repo(repo_url)?;
        Ok(GitHub {
            client: Client::builder().build()?,
            token: token.to_string(),
            owner,
            repo,
        })
    }

    fn request(&self, builder: reqwest::RequestBuilder) -> reqwest::RequestBuilder {
        builder
            .header(AUTHORIZATION, format!("Bearer {}", self.token))
            .header(ACCEPT, "application/vnd.github+json")
            .header("X-GitHub-Api-Version", "2022-11-28")
            .header(USER_AGENT, "workrave-ship")
    }

    async fn check(&self, what: &str, response: Response) -> Result<Response> {
        let status = response.status();
        if status.is_success() {
            return Ok(response);
        }
        let body = response.text().await.unwrap_or_default();
        bail!(
            "GitHub API: {what} failed with HTTP {}: {}",
            status.as_u16(),
            body.trim()
        );
    }

    /// Finds the release for `tag`, including drafts (which the by-tag
    /// endpoint does not return).
    pub async fn find_release(&self, tag: &str) -> Result<Option<Release>> {
        let mut page = 1;
        loop {
            let url = format!(
                "{API}/repos/{}/{}/releases?per_page=100&page={page}",
                self.owner, self.repo
            );
            let response = self.request(self.client.get(&url)).send().await?;
            let releases: Vec<Release> = self
                .check("listing releases", response)
                .await?
                .json()
                .await
                .context("parsing the release list")?;
            if let Some(release) = releases.iter().find(|r| r.tag_name == tag) {
                return Ok(Some(release.clone()));
            }
            if releases.len() < 100 {
                return Ok(None);
            }
            page += 1;
        }
    }

    pub async fn create_draft_release(
        &self,
        tag: &str,
        title: &str,
        notes: &str,
        prerelease: bool,
    ) -> Result<Release> {
        let url = format!("{API}/repos/{}/{}/releases", self.owner, self.repo);
        let body = serde_json::json!({
            "tag_name": tag,
            "name": title,
            "body": notes,
            "draft": true,
            "prerelease": prerelease,
        });
        let response = self
            .request(self.client.post(&url))
            .json(&body)
            .send()
            .await?;
        let release: Release = self
            .check(&format!("creating release {tag}"), response)
            .await?
            .json()
            .await
            .context("parsing the created release")?;
        tracing::info!("Created draft release {} ({})", tag, release.html_url);
        Ok(release)
    }

    pub async fn upload_asset(&self, release: &Release, file: &Path) -> Result<()> {
        let name = file
            .file_name()
            .ok_or_else(|| anyhow!("{} has no file name", file.display()))?
            .to_string_lossy()
            .into_owned();
        let data = tokio::fs::read(file)
            .await
            .with_context(|| format!("reading {}", file.display()))?;
        let url = format!(
            "{UPLOADS}/repos/{}/{}/releases/{}/assets?name={}",
            self.owner,
            self.repo,
            release.id,
            urlencode(&name)
        );
        tracing::info!(
            "Uploading {} to release {}",
            file.display(),
            release.tag_name
        );
        let response = self
            .request(self.client.post(&url))
            .header(CONTENT_TYPE, "application/octet-stream")
            .body(data)
            .send()
            .await?;
        self.check(&format!("uploading {name}"), response).await?;
        Ok(())
    }
}

/// `https://github.com/owner/repo.git` or `git@github.com:owner/repo.git` -> (owner, repo)
pub fn parse_repo(url: &str) -> Result<(String, String)> {
    let path = url
        .strip_prefix("https://github.com/")
        .or_else(|| url.strip_prefix("git@github.com:"))
        .ok_or_else(|| anyhow!("not a GitHub repository url: {url}"))?;
    let path = path.trim_end_matches('/').trim_end_matches(".git");
    match path.split_once('/') {
        Some((owner, repo)) if !owner.is_empty() && !repo.is_empty() && !repo.contains('/') => {
            Ok((owner.to_string(), repo.to_string()))
        }
        _ => bail!("not a GitHub repository url: {url}"),
    }
}

fn urlencode(s: &str) -> String {
    let mut out = String::new();
    for b in s.bytes() {
        match b {
            b'A'..=b'Z' | b'a'..=b'z' | b'0'..=b'9' | b'-' | b'_' | b'.' | b'~' => {
                out.push(b as char)
            }
            _ => out.push_str(&format!("%{b:02X}")),
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_repository_urls() {
        assert_eq!(
            parse_repo("https://github.com/rcaelers/workrave.git").unwrap(),
            ("rcaelers".to_string(), "workrave".to_string())
        );
        assert_eq!(
            parse_repo("git@github.com:rcaelers/workrave-appcast.git").unwrap(),
            ("rcaelers".to_string(), "workrave-appcast".to_string())
        );
        assert!(parse_repo("https://gitlab.com/x/y").is_err());
    }

    #[test]
    fn encodes_asset_names() {
        assert_eq!(
            urlencode("workrave-1.12.0-alpha.1.dmg"),
            "workrave-1.12.0-alpha.1.dmg"
        );
        assert_eq!(urlencode("a b+c"), "a%20b%2Bc");
    }
}
