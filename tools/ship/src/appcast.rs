use anyhow::{Context, Result};
use minijinja::value::Value as JValue;
use minijinja::{context, Environment};
use regex::Regex;
use serde_json::Value;
use std::path::PathBuf;
use std::sync::OnceLock;

use crate::newsgen::{NewsGenerator, NewsParams};
use crate::s3::S3Store;
use crate::templates::{load_templates, register_common_filters};

pub struct AppcastGenerator {
    catalog: Value,
    news: Option<Value>,
    environment: String,
}

impl AppcastGenerator {
    pub fn new(catalog: Value, news: Option<Value>, environment: String) -> Self {
        Self {
            catalog,
            news,
            environment,
        }
    }

    pub fn tag_to_version(tag: &str, increment: &str) -> String {
        static UNDERSCORE_DIGIT: OnceLock<Regex> = OnceLock::new();
        static DASH_DIGITS: OnceLock<Regex> = OnceLock::new();
        let r1 = UNDERSCORE_DIGIT.get_or_init(|| Regex::new(r"_([0-9])").unwrap());
        let r2 = DASH_DIGITS.get_or_init(|| Regex::new(r"-[0-9]+").unwrap());

        let mut v = r1.replace_all(tag, ".$1").into_owned();
        v = r2.replace_all(&v, "").into_owned();
        v = v.replace('_', "-");
        if let Some(rest) = v.strip_prefix('v') {
            v = rest.to_string();
        }
        if !increment.is_empty() && increment != "0" {
            v.push('-');
            v.push_str(increment);
        }
        v
    }

    fn generate_notes(news: &Value, version: &str) -> String {
        let params = NewsParams {
            release: Some(version.to_string()),
            single: true,
            latest: false,
            template: "github".to_string(),
            series: String::new(),
            increment: 0,
        };
        let mut gen = NewsGenerator::new(news.clone(), params);
        gen.generate().unwrap_or_default()
    }

    fn fixup(&mut self) {
        let news = match &self.news {
            Some(n) => n.clone(),
            None => return,
        };
        let builds = match self
            .catalog
            .get_mut("builds")
            .and_then(|v| v.as_array_mut())
        {
            Some(b) => b,
            None => return,
        };
        for build in builds.iter_mut() {
            let tag = build
                .get("tag")
                .and_then(|v| v.as_str())
                .unwrap_or("")
                .to_string();
            let increment = match build.get("increment") {
                Some(Value::String(s)) => s.clone(),
                Some(Value::Number(n)) => n.to_string(),
                _ => String::new(),
            };
            let version = Self::tag_to_version(&tag, &increment);
            let notes = Self::generate_notes(&news, &version);
            if !notes.is_empty() {
                if let Value::Object(map) = build {
                    map.insert("notes".to_string(), Value::String(notes));
                }
            }
        }
    }

    pub fn generate(mut self) -> Result<String> {
        self.fixup();

        let mut env = Environment::new();
        load_templates(&mut env)?;
        register_common_filters(&mut env);
        env.add_filter("version", |item: JValue| -> String {
            let tag = item
                .get_attr("tag")
                .ok()
                .and_then(|v| v.as_str().map(|s| s.to_string()))
                .unwrap_or_default();
            let increment = item
                .get_attr("increment")
                .ok()
                .map(|v| {
                    if let Some(s) = v.as_str() {
                        s.to_string()
                    } else {
                        v.to_string()
                    }
                })
                .unwrap_or_default();
            AppcastGenerator::tag_to_version(&tag, &increment)
        });
        env.add_filter("channel", |item: JValue| -> String {
            let increment = item
                .get_attr("increment")
                .ok()
                .map(|v| {
                    if let Some(s) = v.as_str() {
                        s.to_string()
                    } else {
                        v.to_string()
                    }
                })
                .unwrap_or_default();
            if !increment.is_empty() && increment != "0" {
                return "dev".to_string();
            }
            let tag = item
                .get_attr("tag")
                .ok()
                .and_then(|v| v.as_str().map(|s| s.to_string()))
                .unwrap_or_default();
            let version = AppcastGenerator::tag_to_version(&tag, &increment);
            match semver::Version::parse(&version) {
                Ok(v) => v.pre.split('.').next().unwrap_or("").to_string(),
                Err(_) => String::new(),
            }
        });

        let builds_jvalue: JValue = serde_json::from_str(&serde_json::to_string(
            self.catalog.get("builds").unwrap_or(&Value::Array(vec![])),
        )?)?;
        let tmpl = env.get_template("appcast.tmpl")?;
        let out = tmpl.render(context! {
            builds => builds_jvalue,
            environment => self.environment,
        })?;
        Ok(out)
    }
}

pub struct AppcastOptions {
    pub branch: String,
    pub bucket: String,
    pub environment: String,
    pub key: String,
    pub secret: String,
    pub endpoint: String,
    pub name: String,
    pub file: bool,
    pub release: Option<String>,
    pub dry: bool,
    pub input: Option<PathBuf>,
}

pub async fn run_appcast(opts: AppcastOptions) -> Result<()> {
    let url = format!(
        "{}/{}/{}/catalog.json",
        opts.endpoint.trim_end_matches('/'),
        opts.bucket,
        opts.branch
    );
    let resp = reqwest::get(&url)
        .await
        .with_context(|| format!("fetching {url}"))?;
    let catalog: Value = resp.json().await.context("parsing catalog.json")?;

    let news: Option<Value> = match &opts.input {
        Some(path) => {
            let raw = tokio::fs::read_to_string(path)
                .await
                .with_context(|| format!("reading {}", path.display()))?;
            Some(serde_yaml::from_str(&raw).context("parsing input YAML")?)
        }
        None => None,
    };

    let _ = opts.release; // present for CLI parity; current generator ignores it.

    let generator = AppcastGenerator::new(catalog, news, opts.environment.clone());
    let content = generator.generate()?;

    if opts.dry {
        return Ok(());
    }
    if opts.file {
        tokio::fs::write(&opts.name, content)
            .await
            .with_context(|| format!("writing {}", opts.name))?;
    } else {
        let store = S3Store::new(&opts.endpoint, &opts.bucket, &opts.key, &opts.secret);
        let key = format!("{}/{}", opts.branch.trim_end_matches('/'), opts.name);
        store.write(&key, content.into_bytes(), "text/xml").await?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn renders_appcast_fixture_like_typescript_template() {
        let catalog = json!({
            "builds": [
                {
                    "tag": "v1_11_0-alpha-2",
                    "increment": "0",
                    "channel": "alpha",
                    "date": "2026-01-15T12:34:56Z",
                    "notes": "Alpha notes",
                    "commits": [],
                    "artifacts": [
                        {
                            "platform": "windows",
                            "kind": "installer",
                            "configuration": "release",
                            "url": "v1.11/a.exe",
                            "filename": "workrave-win.exe",
                            "ed25519": "sig-a",
                            "size": 123
                        },
                        {
                            "platform": "windows",
                            "kind": "portable",
                            "configuration": "release",
                            "url": "v1.11/a.zip",
                            "filename": "workrave.zip",
                            "ed25519": "sig-z",
                            "size": 456
                        }
                    ]
                },
                {
                    "tag": "v1_11_1",
                    "increment": "3",
                    "channel": "dev",
                    "date": "2026-01-16T12:34:56Z",
                    "notes": "Dev notes",
                    "commits": [
                        {
                            "oid": "abc123",
                            "commit": {
                                "author": {
                                    "name": "Dev User",
                                    "timestamp": 1700000000
                                },
                                "message": "Fix thing\n"
                            }
                        }
                    ],
                    "artifacts": [
                        {
                            "platform": "windows",
                            "kind": "installer",
                            "configuration": "release",
                            "url": "v1.11/dev.exe",
                            "filename": "workrave-v1_11-dev.exe",
                            "ed25519": "sig-d",
                            "size": 789
                        }
                    ]
                },
                {
                    "tag": "v1_11_2",
                    "increment": "0",
                    "channel": "stable",
                    "date": "2026-01-17T12:34:56Z",
                    "notes": "No installer",
                    "commits": [],
                    "artifacts": [
                        {
                            "platform": "macos",
                            "kind": "dmg",
                            "configuration": "release",
                            "url": "mac.dmg",
                            "filename": "mac.dmg",
                            "ed25519": "sig-m",
                            "size": 99
                        }
                    ]
                }
            ]
        });

        let output = AppcastGenerator::new(catalog, None, "staging".to_string())
            .generate()
            .unwrap();

        assert!(output.contains("<title>Workrave 1.11.0-alpha</title>"));
        assert!(output.contains("<title>Workrave 1.11.1-3</title>"));
        assert!(!output.contains("No installer"));
        assert_eq!(output.matches("<item>").count(), 2);
        assert!(output.contains("https://snapshots.workrave.org/v1.11/a.exe"));
        assert!(output.contains("https://snapshots.workrave.org/v1.11/dev.exe"));
        assert!(!output.contains("v1.11/a.zip"));
        assert!(output.contains("commit abc123\nAuthor: Dev User\nDate: "));
        assert!(output.contains("\n\nFix thing\n"));
    }
}
