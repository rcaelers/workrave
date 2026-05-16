use std::sync::OnceLock;

use chrono::{DateTime, FixedOffset, Local, TimeZone, Utc};
use minijinja::value::Value;
use minijinja::{Environment, Error, ErrorKind, UndefinedBehavior};
use regex::Regex;

use crate::markdown_text::markdown_to_text;

// Templates are baked into the binary so the tool can run from anywhere.
const APPCAST_TMPL: &str = include_str!("../templates/appcast.tmpl");
const BLOG_TMPL: &str = include_str!("../templates/blog.tmpl");
const DEBIAN_CHANGELOG_TMPL: &str = include_str!("../templates/debian-changelog.tmpl");
const GITHUB_TMPL: &str = include_str!("../templates/github.tmpl");
const NEWS_TMPL: &str = include_str!("../templates/news.tmpl");

pub fn load_templates(env: &mut Environment<'static>) -> Result<(), Error> {
    env.set_keep_trailing_newline(true);
    env.set_undefined_behavior(UndefinedBehavior::Chainable);
    env.add_template("appcast.tmpl", APPCAST_TMPL)?;
    env.add_template("blog.tmpl", BLOG_TMPL)?;
    env.add_template("debian-changelog.tmpl", DEBIAN_CHANGELOG_TMPL)?;
    env.add_template("github.tmpl", GITHUB_TMPL)?;
    env.add_template("news.tmpl", NEWS_TMPL)?;
    Ok(())
}

const DEFAULT_STRFTIME_FORMAT: &str = "%Y-%m-%dT%H:%M:%S%:z";

pub fn data_format_filter(value: Value, format: Option<String>) -> Result<String, Error> {
    let s = value
        .as_str()
        .ok_or_else(|| Error::new(ErrorKind::InvalidOperation, "data_format expects a string"))?;
    let dt = parse_datetime(s)?.with_timezone(&Local);
    let fmt = format.as_deref().unwrap_or(DEFAULT_STRFTIME_FORMAT);
    Ok(dt.format(fmt).to_string())
}

pub fn data_format_from_unix_filter(value: Value, format: Option<String>) -> Result<String, Error> {
    let secs: i64 = if let Some(n) = value.as_i64() {
        n
    } else if let Some(s) = value.as_str() {
        s.parse().map_err(|e| {
            Error::new(
                ErrorKind::InvalidOperation,
                format!("not a unix timestamp: {e}"),
            )
        })?
    } else {
        return Err(Error::new(
            ErrorKind::InvalidOperation,
            "data_format_from_unix expects a number",
        ));
    };
    let dt = Utc
        .timestamp_opt(secs, 0)
        .single()
        .ok_or_else(|| Error::new(ErrorKind::InvalidOperation, "invalid unix timestamp"))?
        .with_timezone(&Local);
    let fmt = format.as_deref().unwrap_or(DEFAULT_STRFTIME_FORMAT);
    Ok(dt.format(fmt).to_string())
}

pub fn is_string_filter(value: Value) -> bool {
    value.as_str().is_some()
}

pub fn github_filter(value: String) -> String {
    static RE: OnceLock<Regex> = OnceLock::new();
    let re = RE.get_or_init(|| Regex::new(r"#([0-9]+)").unwrap());
    re.replace_all(
        &value,
        "[#$1](https://github.com/rcaelers/workrave/issues/$1)",
    )
    .into_owned()
}

pub fn wrap_filter(value: String, width: usize) -> String {
    let opts = textwrap::Options::new(width)
        .break_words(false)
        .word_separator(textwrap::WordSeparator::AsciiSpace)
        .word_splitter(textwrap::WordSplitter::NoHyphenation);
    textwrap::fill(&value, opts)
}

pub fn channel_from_version_filter(value: String) -> String {
    match semver::Version::parse(&value) {
        Ok(v) => v.pre.split('.').next().unwrap_or("").to_string(),
        Err(_) => String::new(),
    }
}

pub fn text_filter(value: String) -> String {
    markdown_to_text(&value, 78)
}

fn parse_datetime(s: &str) -> Result<DateTime<FixedOffset>, Error> {
    if let Ok(dt) = DateTime::parse_from_rfc3339(s) {
        return Ok(dt);
    }
    if let Ok(dt) = DateTime::parse_from_rfc2822(s) {
        return Ok(dt);
    }
    // Fallback: assume RFC3339-ish without TZ -> treat as UTC.
    if let Ok(naive) = chrono::NaiveDateTime::parse_from_str(s, "%Y-%m-%dT%H:%M:%S") {
        return Ok(naive.and_utc().fixed_offset());
    }
    if let Ok(naive) = chrono::NaiveDate::parse_from_str(s, "%Y-%m-%d") {
        return Ok(naive.and_hms_opt(0, 0, 0).unwrap().and_utc().fixed_offset());
    }
    Err(Error::new(
        ErrorKind::InvalidOperation,
        format!("could not parse date: {s}"),
    ))
}

pub fn register_common_filters(env: &mut Environment<'static>) {
    env.add_filter("data_format", data_format_filter);
    env.add_filter("data_format_from_unix", data_format_from_unix_filter);
    env.add_filter("is_string", is_string_filter);
    env.add_filter("github", github_filter);
    env.add_filter("wrap", wrap_filter);
    env.add_filter("text", text_filter);
}
