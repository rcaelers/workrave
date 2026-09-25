//! Client for the workrave signing service.
//!
//! The service signs artifacts (cosign/sigstore bundles, ed25519 signatures
//! for the update catalog, Windows authenticode) and hands out the secrets the
//! release needs (GitHub token, S3 key, ...), so that no key material lives on
//! the build machines.

use std::path::{Path, PathBuf};
use std::time::Duration;

use anyhow::{anyhow, bail, Context, Result};
use reqwest::multipart::{Form, Part};
use reqwest::{Client, Response};
use serde::Deserialize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, clap::ValueEnum)]
pub enum SignKind {
    /// Sigstore bundle written next to the file as `<file>.sigstore`.
    Cosign,
    /// ed25519 signature for the update catalog, printed as base64.
    Ed25519,
    /// Windows authenticode; the file is replaced by its signed version.
    Authenticode,
}

impl SignKind {
    fn endpoint(self) -> &'static str {
        match self {
            SignKind::Cosign => "cosign",
            SignKind::Ed25519 => "ed25519",
            SignKind::Authenticode => "authenticode",
        }
    }
}

#[derive(Clone)]
pub struct SigningService {
    url: String,
    client: Client,
}

impl SigningService {
    /// `url` is the service base URL, e.g. `https://host:50051`.
    pub fn new(url: &str) -> Result<Self> {
        let client = Client::builder()
            // The service uses a self-signed certificate on the local network.
            .danger_accept_invalid_certs(true)
            .connect_timeout(Duration::from_secs(10))
            .build()
            .context("creating HTTP client")?;
        Ok(Self {
            url: url.trim_end_matches('/').to_string(),
            client,
        })
    }

    async fn send(&self, request: reqwest::RequestBuilder) -> Result<Response> {
        request.send().await.map_err(|e| {
            anyhow!(
                "cannot reach the signing service at {}: {}\nIs the signing service running, and reachable from this machine?",
                self.url,
                without_url(&e)
            )
        })
    }

    async fn check_status(&self, what: &str, response: Response) -> Result<Response> {
        let status = response.status();
        if status.is_success() {
            return Ok(response);
        }
        let body = response.text().await.unwrap_or_default();
        let detail = error_message(&body);
        bail!(
            "signing service at {} returned HTTP {} for {what}{}",
            self.url,
            status.as_u16(),
            if detail.is_empty() {
                String::new()
            } else {
                format!(": {detail}")
            }
        );
    }

    /// Fetches a named secret, e.g. `secrets.tokens.github_pat`.
    pub async fn secret(&self, name: &str) -> Result<String> {
        #[derive(Deserialize)]
        struct Secret {
            value: Option<String>,
        }

        let response = self
            .send(self.client.get(format!("{}/secrets/{}", self.url, name)))
            .await?;
        let response = self
            .check_status(&format!("secret {name}"), response)
            .await?;
        let secret: Secret = response
            .json()
            .await
            .with_context(|| format!("parsing the response for secret {name}"))?;
        match secret.value {
            Some(v) if !v.is_empty() => Ok(v),
            _ => bail!(
                "signing service at {} returned no value for secret {name}",
                self.url
            ),
        }
    }

    /// Sends `file` to `/sign/<kind>` and returns the raw response body.
    pub async fn sign(&self, kind: SignKind, file: &Path) -> Result<Vec<u8>> {
        let data = tokio::fs::read(file)
            .await
            .with_context(|| format!("reading {}", file.display()))?;
        let name = file
            .file_name()
            .map(|n| n.to_string_lossy().into_owned())
            .unwrap_or_else(|| "file".to_string());
        let form = Form::new().part("file", Part::bytes(data).file_name(name));

        tracing::info!(
            "Signing {} ({}) via {}",
            file.display(),
            kind.endpoint(),
            self.url
        );
        let response = self
            .send(
                self.client
                    .post(format!("{}/sign/{}", self.url, kind.endpoint()))
                    .multipart(form),
            )
            .await?;
        let response = self
            .check_status(
                &format!("{} signing of {}", kind.endpoint(), file.display()),
                response,
            )
            .await?;
        let body = response.bytes().await.context("reading the signature")?;
        if body.is_empty() {
            bail!(
                "signing service at {} returned an empty signature for {}",
                self.url,
                file.display()
            );
        }
        Ok(body.to_vec())
    }

    /// Creates `<file>.sigstore` and returns its path.
    pub async fn cosign(&self, file: &Path) -> Result<PathBuf> {
        let bundle = self.sign(SignKind::Cosign, file).await?;
        let output = sigstore_path(file);
        tokio::fs::write(&output, bundle)
            .await
            .with_context(|| format!("writing {}", output.display()))?;
        tracing::info!("Created {}", output.display());
        Ok(output)
    }

    /// Returns the base64 ed25519 signature of `file`.
    pub async fn ed25519(&self, file: &Path) -> Result<String> {
        #[derive(Deserialize)]
        struct Signed {
            signature: Option<String>,
        }
        let body = self.sign(SignKind::Ed25519, file).await?;
        let signed: Signed = serde_json::from_slice(&body)
            .with_context(|| format!("parsing the ed25519 response for {}", file.display()))?;
        match signed.signature {
            Some(s) if !s.is_empty() => Ok(s),
            _ => bail!(
                "signing service at {} returned no ed25519 signature for {}",
                self.url,
                file.display()
            ),
        }
    }

    /// Replaces `file` by its authenticode-signed version.
    pub async fn authenticode(&self, file: &Path) -> Result<()> {
        let signed = self.sign(SignKind::Authenticode, file).await?;
        let temp = file.with_extension(format!(
            "{}.signing",
            file.extension()
                .map(|e| e.to_string_lossy().into_owned())
                .unwrap_or_default()
        ));
        tokio::fs::write(&temp, signed)
            .await
            .with_context(|| format!("writing {}", temp.display()))?;
        tokio::fs::rename(&temp, file)
            .await
            .with_context(|| format!("replacing {}", file.display()))?;
        tracing::info!("Signed {}", file.display());
        Ok(())
    }

    /// Adds an ed25519 signature for every artifact listed in the
    /// `job-catalog*.json` files under `dir` (the `ed25519` field of each
    /// artifact entry).
    pub async fn sign_catalogs(&self, dir: &Path) -> Result<()> {
        for catalog_path in find_catalogs(dir)? {
            let raw = tokio::fs::read_to_string(&catalog_path)
                .await
                .with_context(|| format!("reading {}", catalog_path.display()))?;
            let mut catalog: serde_json::Value = serde_json::from_str(&raw)
                .with_context(|| format!("parsing {}", catalog_path.display()))?;
            let folder = catalog_path.parent().unwrap_or(dir);

            let builds = catalog
                .get_mut("builds")
                .and_then(|b| b.as_array_mut())
                .ok_or_else(|| anyhow!("{} has no builds", catalog_path.display()))?;
            for build in builds {
                let Some(artifacts) = build.get_mut("artifacts").and_then(|a| a.as_array_mut())
                else {
                    continue;
                };
                for artifact in artifacts {
                    let Some(filename) = artifact.get("filename").and_then(|f| f.as_str()) else {
                        continue;
                    };
                    let signature = self.ed25519(&folder.join(filename.trim())).await?;
                    artifact["ed25519"] = serde_json::Value::String(signature);
                }
            }

            let updated = serde_json::to_string_pretty(&catalog)?;
            tokio::fs::write(&catalog_path, updated)
                .await
                .with_context(|| format!("writing {}", catalog_path.display()))?;
            tracing::info!("Signed artifacts in {}", catalog_path.display());
        }
        Ok(())
    }
}

pub fn sigstore_path(file: &Path) -> PathBuf {
    let mut name = file.as_os_str().to_os_string();
    name.push(".sigstore");
    PathBuf::from(name)
}

fn find_catalogs(dir: &Path) -> Result<Vec<PathBuf>> {
    let mut found = Vec::new();
    let mut stack = vec![dir.to_path_buf()];
    while let Some(current) = stack.pop() {
        let entries = std::fs::read_dir(&current)
            .with_context(|| format!("listing {}", current.display()))?;
        for entry in entries {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                stack.push(path);
            } else if path
                .file_name()
                .map(|n| n.to_string_lossy().starts_with("job-catalog"))
                .unwrap_or(false)
            {
                found.push(path);
            }
        }
    }
    found.sort();
    Ok(found)
}

/// The service reports errors as `{"error": "..."}`; fall back to the raw body.
fn error_message(body: &str) -> String {
    if let Ok(value) = serde_json::from_str::<serde_json::Value>(body) {
        if let Some(error) = value.get("error").and_then(|e| e.as_str()) {
            return error.to_string();
        }
    }
    let trimmed = body.trim();
    if trimmed.len() > 500 {
        format!("{}...", &trimmed[..500])
    } else {
        trimmed.to_string()
    }
}

/// reqwest errors repeat the URL, which the caller already prints.
fn without_url(error: &reqwest::Error) -> String {
    let mut message = error.to_string();
    let mut source = std::error::Error::source(error);
    while let Some(inner) = source {
        message = format!("{message}: {inner}");
        source = inner.source();
    }
    message
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sigstore_path_appends_extension() {
        assert_eq!(
            sigstore_path(Path::new("/x/workrave-1.12.dmg")),
            PathBuf::from("/x/workrave-1.12.dmg.sigstore")
        );
    }

    #[test]
    fn error_message_prefers_json_error_field() {
        assert_eq!(error_message(r#"{"error": "no such key"}"#), "no such key");
        assert_eq!(error_message("plain text"), "plain text");
        assert_eq!(error_message(r#"{"other": 1}"#), r#"{"other": 1}"#);
    }

    #[tokio::test]
    async fn unreachable_service_reports_url() {
        let service = SigningService::new("https://127.0.0.1:1").unwrap();
        let err = service.secret("x").await.unwrap_err().to_string();
        assert!(
            err.contains("cannot reach the signing service at https://127.0.0.1:1"),
            "{err}"
        );
    }

    #[tokio::test]
    async fn http_errors_include_status_and_body() {
        use std::io::{Read, Write};
        let listener = std::net::TcpListener::bind("127.0.0.1:0").unwrap();
        let port = listener.local_addr().unwrap().port();
        std::thread::spawn(move || {
            let (mut stream, _) = listener.accept().unwrap();
            let mut buf = [0u8; 4096];
            let _ = stream.read(&mut buf);
            let body = r#"{"error":"no such secret"}"#;
            write!(
                stream,
                "HTTP/1.1 404 Not Found\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                body.len(),
                body
            )
            .unwrap();
        });
        let service = SigningService::new(&format!("http://127.0.0.1:{port}")).unwrap();
        let err = service
            .secret("secrets.tokens.x")
            .await
            .unwrap_err()
            .to_string();
        assert!(err.contains("HTTP 404"), "{err}");
        assert!(err.contains("no such secret"), "{err}");
    }
}
