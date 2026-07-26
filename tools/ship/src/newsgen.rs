use anyhow::{Context, Result};
use minijinja::value::Value as JValue;
use minijinja::{context, Environment};
use serde_json::Value;
use std::path::Path;

use crate::templates::{channel_from_version_filter, load_templates, register_common_filters};

#[derive(Debug, Clone)]
pub struct NewsParams {
    pub release: Option<String>,
    pub single: bool,
    pub latest: bool,
    pub template: String,
    pub series: String,
    pub increment: i64,
}

pub struct NewsGenerator {
    news: Value,
    params: NewsParams,
}

impl NewsGenerator {
    pub fn new(news: Value, params: NewsParams) -> Self {
        Self { news, params }
    }

    pub fn generate(&mut self) -> Result<String> {
        let mut env = Environment::new();
        load_templates(&mut env)?;
        register_common_filters(&mut env);
        // newsgen's `channel` filter takes a version string.
        env.add_filter("channel", channel_from_version_filter);

        // Snapshot the previous version before filter_releases mutates the array.
        let previous_version = self.compute_previous_version();
        self.filter_releases();
        let releases_value = serde_json::to_string(&self.news)?;
        let releases_jvalue: JValue = serde_json::from_str(&releases_value)?;

        let tmpl = env.get_template(&format!("{}.tmpl", self.params.template))?;
        let ctx = context! {
            previous_version => previous_version,
            series => self.params.series,
            increment => self.params.increment,
            releases => releases_jvalue,
        };
        let out = tmpl.render(ctx)?;
        Ok(out)
    }

    fn version_index(&self) -> Option<usize> {
        let releases = self.news.get("releases")?.as_array()?;
        match &self.params.release {
            Some(target) => releases
                .iter()
                .position(|r| r.get("version").and_then(|v| v.as_str()) == Some(target)),
            None => Some(0),
        }
    }

    fn compute_previous_version(&self) -> Option<String> {
        if !(self.params.release.is_some() || self.params.latest) {
            return None;
        }
        let idx = self.version_index()?;
        if !self.params.single {
            return None;
        }
        let releases = self.news.get("releases")?.as_array()?;
        if idx + 1 >= releases.len() {
            return None;
        }
        releases[idx + 1]
            .get("version")?
            .as_str()
            .map(|s| s.to_string())
    }

    fn filter_releases(&mut self) {
        if !(self.params.release.is_some() || self.params.latest) {
            return;
        }
        let idx = match self.version_index() {
            Some(i) => i,
            None => return,
        };
        let single = self.params.single;
        if let Some(releases) = self.news.get_mut("releases").and_then(|v| v.as_array_mut()) {
            let kept: Vec<Value> = releases
                .iter()
                .enumerate()
                .filter(|(i, _)| if single { *i == idx } else { idx >= *i })
                .map(|(_, v)| v.clone())
                .collect();
            *releases = kept;
        }
    }
}

pub async fn run_newsgen(
    input: &Path,
    output: &Path,
    template: String,
    release: Option<String>,
    single: bool,
    latest: bool,
    series: String,
    increment: i64,
) -> Result<()> {
    let raw = tokio::fs::read_to_string(input)
        .await
        .with_context(|| format!("reading {}", input.display()))?;
    let news: Value = serde_yaml::from_str(&raw).context("parsing input YAML")?;
    let mut gen = NewsGenerator::new(
        news,
        NewsParams {
            release,
            single,
            latest,
            template,
            series,
            increment,
        },
    );
    let content = gen.generate()?;
    tokio::fs::write(output, content)
        .await
        .with_context(|| format!("writing {}", output.display()))?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    fn fixture_news() -> Value {
        json!({
            "releases": [
                {
                    "version": "1.2.3",
                    "date": "2026-01-15T12:34:56Z",
                    "short": "Short intro.",
                    "more": "More details.",
                    "changes": [
                        "Fix #123 and improve wrapping behavior with enough words for text output.",
                        "Second change"
                    ]
                },
                {
                    "version": "1.2.2",
                    "date": "2025-12-01T08:00:00Z",
                    "changes": ["Older change"]
                }
            ]
        })
    }

    fn render_template(template: &str) -> String {
        let mut generator = NewsGenerator::new(
            fixture_news(),
            NewsParams {
                release: Some("1.2.3".to_string()),
                single: true,
                latest: false,
                template: template.to_string(),
                series: "jammy".to_string(),
                increment: 2,
            },
        );
        generator.generate().unwrap()
    }

    #[test]
    fn renders_github_template_like_typescript_fixture() {
        assert_eq!(
            render_template("github"),
            "\nShort intro.\n\nMore details.\n- Fix #123 and improve wrapping behavior with enough words for text output.\n- Second change\n\n\n"
        );
    }

    #[test]
    fn renders_news_template_like_typescript_fixture() {
        let output = render_template("news");
        assert!(output
            .starts_with("Workrave NEWS -- history of user-visible changes. 15 January 2026\n"));
        assert!(output.contains("* Workrave 1.2.3\n\n"));
        assert!(output.contains(
            "** Fix #123 and improve wrapping behavior with enough words for text output.\n** Second change\n"
        ));
    }

    #[test]
    fn renders_blog_template_like_typescript_fixture() {
        let output = render_template("blog");
        assert!(output.contains("slug: workrave-1-2-3-released\n"));
        assert!(output.contains("title: Workrave 1.2.3 Released\n"));
        assert!(output.contains("Short intro.\n\n<!--more-->\n\nMore details.\n\n"));
        assert!(output.contains(
            "- Fix [#123](https://github.com/rcaelers/workrave/issues/123) and improve wrapping behavior with enough words for text output.\n- Second change\n"
        ));
    }

    #[test]
    fn renders_debian_changelog_template_like_typescript_fixture() {
        let output = render_template("debian-changelog");
        assert!(output.starts_with("workrave (1.2.3-ppa2~jammy) jammy; urgency=medium\n"));
        assert!(output.contains(
            "  * Fix #123 and improve wrapping behavior with enough words for text output.\n  * Second change\n"
        ));
        assert!(output.contains(" -- Rob Caelers <robc@krandor.org>  "));
    }
}
