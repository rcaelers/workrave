//! Support for `@rpc` tags that can't live in the header's own comments —
//! e.g. a third-party or generated header nothing here is allowed to touch.
//! An external annotations file maps a fully-qualified C++ name to a block
//! of tag lines, which are treated exactly like a doc comment attached to
//! that declaration: fed through the same `annotations::parse_*` functions,
//! merged with whatever real comment (if any) the declaration also has.
//!
//! File format (deliberately a small hand-rolled parser, not a new
//! dependency — this mirrors how the tag grammar itself is plain regexes
//! over text, not a structured schema):
//!
//! ```text
//! # Lines starting with # are comments.
//!
//! [Namespace::Class]
//! @rpc(service="ServiceName")
//!
//! [Namespace::Class::method_name(ParamType1,ParamType2)]
//! @rpc(name="MethodName")
//! @rpc.param(x, dir=out)
//!
//! [Namespace::Class::no_arg_method()]
//! @rpc(name="Other")
//! ```
//!
//! A method's key includes its parenthesized, comma-joined parameter types
//! (exactly as they'd be spelled at the declaration) to disambiguate
//! overloads — the same reason each overload gets its own comment block
//! in-source. Whitespace inside a key is insignificant: `Class::f(int,bool)`
//! and `Class::f( int , bool )` are the same key.

use std::collections::HashMap;
use std::fs;
use std::path::Path;

use anyhow::{bail, Context, Result};

#[derive(Debug, Clone, Default)]
pub struct ExternalAnnotations {
    /// Keyed by `normalize_key(...)` of the `[...]` header text.
    entries: HashMap<String, String>,
}

/// Strips all whitespace, so formatting differences between how a type is
/// spelled in the annotations file and how libclang spells it don't cause a
/// spurious lookup miss.
pub fn normalize_key(s: &str) -> String {
    s.chars().filter(|c| !c.is_whitespace()).collect()
}

impl ExternalAnnotations {
    pub fn load(path: &Path) -> Result<Self> {
        let text = fs::read_to_string(path)
            .with_context(|| format!("reading annotations file {}", path.display()))?;
        Self::parse(&text).with_context(|| format!("parsing annotations file {}", path.display()))
    }

    pub fn parse(text: &str) -> Result<Self> {
        let mut entries: HashMap<String, String> = HashMap::new();
        let mut current_key: Option<String> = None;
        let mut current_body = String::new();

        let flush = |entries: &mut HashMap<String, String>, key: &str, body: &str| {
            entries
                .entry(normalize_key(key))
                .or_default()
                .push_str(body);
        };

        for (lineno, raw_line) in text.lines().enumerate() {
            let line = raw_line.trim();
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            if let Some(header) = line.strip_prefix('[').and_then(|s| s.strip_suffix(']')) {
                if let Some(key) = current_key.take() {
                    flush(&mut entries, &key, &current_body);
                }
                current_key = Some(header.to_string());
                current_body = String::new();
                continue;
            }
            match &current_key {
                Some(_) => {
                    current_body.push_str(raw_line);
                    current_body.push('\n');
                }
                None => bail!(
                    "line {}: expected a '[Qualified::Name]' header before any tag lines, found '{line}'",
                    lineno + 1
                ),
            }
        }
        if let Some(key) = current_key {
            flush(&mut entries, &key, &current_body);
        }

        Ok(Self { entries })
    }

    pub fn lookup(&self, qualified_name: &str) -> Option<&str> {
        self.entries
            .get(&normalize_key(qualified_name))
            .map(String::as_str)
    }
}

/// Concatenates a declaration's real comment (if any) with its external
/// annotation text (if any) into one blob to run the existing tag parsers
/// over — order doesn't matter, they scan for `@rpc...` patterns anywhere.
pub fn effective_comment(own_comment: Option<String>, external: Option<&str>) -> Option<String> {
    match (own_comment, external) {
        (Some(mut a), Some(b)) => {
            a.push('\n');
            a.push_str(b);
            Some(a)
        }
        (Some(a), None) => Some(a),
        (None, Some(b)) => Some(b.to_string()),
        (None, None) => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_single_block() {
        let text = "[Foo::Bar]\n@rpc(service=\"BarService\")\n";
        let a = ExternalAnnotations::parse(text).unwrap();
        assert_eq!(a.lookup("Foo::Bar"), Some("@rpc(service=\"BarService\")\n"));
    }

    #[test]
    fn lookup_is_whitespace_insensitive() {
        let text = "[Foo::Bar::f( int , bool )]\n@rpc(name=\"F\")\n";
        let a = ExternalAnnotations::parse(text).unwrap();
        assert_eq!(a.lookup("Foo::Bar::f(int,bool)"), Some("@rpc(name=\"F\")\n"));
    }

    #[test]
    fn comments_and_blank_lines_are_ignored() {
        let text = "# a header comment\n\n[Foo::Bar]\n# a body comment\n@rpc(service=\"BarService\")\n\n";
        let a = ExternalAnnotations::parse(text).unwrap();
        assert_eq!(a.lookup("Foo::Bar"), Some("@rpc(service=\"BarService\")\n"));
    }

    #[test]
    fn multiple_blocks_for_the_same_key_are_merged() {
        let text = "[Foo::Bar]\n@rpc(service=\"BarService\")\n\n[Foo::Bar]\n@rpc.bitmask\n";
        let a = ExternalAnnotations::parse(text).unwrap();
        let body = a.lookup("Foo::Bar").unwrap();
        assert!(body.contains("@rpc(service=\"BarService\")"));
        assert!(body.contains("@rpc.bitmask"));
    }

    #[test]
    fn unknown_key_returns_none() {
        let a = ExternalAnnotations::parse("[Foo::Bar]\n@rpc.bitmask\n").unwrap();
        assert_eq!(a.lookup("Foo::Baz"), None);
    }

    #[test]
    fn tag_line_before_any_header_errors() {
        assert!(ExternalAnnotations::parse("@rpc.bitmask\n[Foo::Bar]\n").is_err());
    }

    #[test]
    fn effective_comment_merges_both_sources() {
        let merged = effective_comment(Some("// @rpc(name=\"A\")".to_string()), Some("@rpc.param(x, dir=out)"));
        let merged = merged.unwrap();
        assert!(merged.contains("@rpc(name=\"A\")"));
        assert!(merged.contains("@rpc.param(x, dir=out)"));
    }
}
