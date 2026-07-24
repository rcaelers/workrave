// Render Markdown to plain text, mirroring the TypeScript release tool renderer.
// Implements paragraph wrapping, list indentation, and headings only — the
// templates only feed simple "changes" snippets through this filter.

use pulldown_cmark::{Event, HeadingLevel, Options, Parser, Tag, TagEnd};

const DEFAULT_INDENT: usize = 2;

pub fn markdown_to_text(input: &str, width: usize) -> String {
    let normalized = normalize_nested_list_indentation(input);
    let events: Vec<Event<'_>> = Parser::new_ext(&normalized, Options::empty()).collect();
    let list_lengths = list_lengths(&events);
    let mut renderer = Renderer::new(width, list_lengths);
    for ev in events {
        renderer.event(ev);
    }
    let mut out = renderer.finish();
    // Strip trailing newlines (matches `replace(/\n+$/g, '')` in TS caller).
    while out.ends_with('\n') {
        out.pop();
    }
    out
}

fn normalize_nested_list_indentation(input: &str) -> String {
    let mut changed = false;
    let mut out = Vec::new();

    for line in input.lines() {
        if is_two_space_nested_list_marker(line) {
            changed = true;
            out.push(format!("  {line}"));
        } else {
            out.push(line.to_string());
        }
    }

    if changed {
        let mut result = out.join("\n");
        if input.ends_with('\n') {
            result.push('\n');
        }
        result
    } else {
        input.to_string()
    }
}

fn is_two_space_nested_list_marker(line: &str) -> bool {
    let Some(rest) = line.strip_prefix("  ") else {
        return false;
    };
    if rest.starts_with("  ") {
        return false;
    }
    if rest.starts_with("- ") || rest.starts_with("* ") || rest.starts_with("+ ") {
        return true;
    }
    let Some((number, after_number)) = rest.split_once(". ") else {
        return false;
    };
    !number.is_empty() && number.chars().all(|c| c.is_ascii_digit()) && !after_number.is_empty()
}

#[derive(Debug)]
enum BlockKind {
    Paragraph,
    Heading,
    Code,
    BlockQuote,
    List {
        ordered_start: Option<u64>,
        index: u64,
        indent: usize,
    },
    ListItem,
    Inline, // emphasis/strong/link etc — collected as text
}

struct Frame {
    kind: BlockKind,
    buf: String,
}

struct Renderer {
    width: usize,
    level: usize,
    out: String,
    stack: Vec<Frame>,
    list_lengths: Vec<usize>,
    next_list: usize,
}

impl Renderer {
    fn new(width: usize, list_lengths: Vec<usize>) -> Self {
        Self {
            width,
            level: 0,
            out: String::new(),
            stack: Vec::new(),
            list_lengths,
            next_list: 0,
        }
    }

    fn finish(mut self) -> String {
        if !self.out.ends_with('\n') {
            self.out.push('\n');
        }
        self.out
    }

    fn push_text(&mut self, s: &str) {
        if let Some(top) = self.stack.last_mut() {
            top.buf.push_str(s);
        } else {
            self.out.push_str(s);
        }
    }

    fn emit(&mut self, s: &str) {
        if let Some(top) = self.stack.last_mut() {
            top.buf.push_str(s);
        } else {
            self.out.push_str(s);
        }
    }

    fn event(&mut self, ev: Event<'_>) {
        match ev {
            Event::Start(tag) => self.start(tag),
            Event::End(tag) => self.end(tag),
            Event::Text(t) => {
                let in_code = matches!(self.stack.last().map(|f| &f.kind), Some(BlockKind::Code));
                if in_code {
                    self.push_text(&t);
                } else {
                    // Collapse internal whitespace, matching TS `text` handler.
                    let collapsed: String = collapse_ws(&t);
                    self.push_text(&collapsed);
                }
            }
            Event::Code(c) => self.push_text(&c),
            Event::Html(h) | Event::InlineHtml(h) => self.push_text(&h),
            Event::SoftBreak => self.push_text(" "),
            Event::HardBreak => self.push_text("\n"),
            Event::Rule => {
                let line = "-".repeat(self.width.saturating_sub(self.level));
                self.emit(&line);
                self.emit("\n");
            }
            Event::TaskListMarker(_)
            | Event::FootnoteReference(_)
            | Event::InlineMath(_)
            | Event::DisplayMath(_) => {}
        }
    }

    fn start(&mut self, tag: Tag<'_>) {
        match tag {
            Tag::Paragraph => self.stack.push(Frame {
                kind: BlockKind::Paragraph,
                buf: String::new(),
            }),
            Tag::Heading { level: _, .. } => {
                self.stack.push(Frame {
                    kind: BlockKind::Heading,
                    buf: String::new(),
                });
            }
            Tag::CodeBlock(_) => self.stack.push(Frame {
                kind: BlockKind::Code,
                buf: String::new(),
            }),
            Tag::BlockQuote(_) => {
                self.level += DEFAULT_INDENT;
                self.stack.push(Frame {
                    kind: BlockKind::BlockQuote,
                    buf: String::new(),
                });
            }
            Tag::List(start) => {
                let length = self.list_lengths.get(self.next_list).copied().unwrap_or(0);
                self.next_list += 1;
                let indent = match start {
                    Some(s) => {
                        let max_number = s + length.saturating_sub(1) as u64;
                        let digits = max_number.max(1).to_string().len();
                        digits + 2
                    }
                    None => DEFAULT_INDENT,
                };
                self.level += indent;
                self.stack.push(Frame {
                    kind: BlockKind::List {
                        ordered_start: start,
                        index: 0,
                        indent,
                    },
                    buf: String::new(),
                });
            }
            Tag::Item => self.stack.push(Frame {
                kind: BlockKind::ListItem,
                buf: String::new(),
            }),
            Tag::Emphasis
            | Tag::Strong
            | Tag::Strikethrough
            | Tag::Link { .. }
            | Tag::Image { .. } => {
                self.stack.push(Frame {
                    kind: BlockKind::Inline,
                    buf: String::new(),
                });
            }
            _ => self.stack.push(Frame {
                kind: BlockKind::Inline,
                buf: String::new(),
            }),
        }
    }

    fn end(&mut self, tag: TagEnd) {
        let frame = match self.stack.pop() {
            Some(f) => f,
            None => return,
        };
        match tag {
            TagEnd::Paragraph => {
                let wrapped = wrap_text(&frame.buf, self.width.saturating_sub(self.level));
                self.emit(&wrapped);
                self.emit("\n");
            }
            TagEnd::Heading(_) => {
                self.emit(&frame.buf);
                self.emit("\n");
            }
            TagEnd::CodeBlock => {
                self.emit("\n");
                let body = frame.buf.trim_end_matches('\n').to_string();
                let indented = indentify(&body, DEFAULT_INDENT, None);
                self.emit(&indented);
                self.emit("\n\n");
            }
            TagEnd::BlockQuote(_) => {
                self.level -= DEFAULT_INDENT;
                let indented = indentify(&frame.buf, DEFAULT_INDENT, None);
                self.emit(&indented);
                self.emit("\n");
            }
            TagEnd::List(_) => {
                if let BlockKind::List { indent, .. } = frame.kind {
                    self.level -= indent;
                }
                if let Some(top) = self.stack.last() {
                    if !top.buf.is_empty() && !top.buf.ends_with('\n') {
                        self.emit("\n");
                    }
                }
                self.emit(&frame.buf);
                self.emit("\n");
            }
            TagEnd::Item => {
                let parent = self.stack.last_mut();
                let (start, index, indent) = match parent.as_ref().map(|p| &p.kind) {
                    Some(BlockKind::List {
                        ordered_start,
                        index,
                        indent,
                    }) => (*ordered_start, *index, *indent),
                    _ => (None, 0, DEFAULT_INDENT),
                };
                let prefix = match start {
                    Some(s) => format!("{}. ", s + index),
                    None => "- ".to_string(),
                };
                let trimmed = frame.buf.trim_matches('\n').to_string();
                let wrapped = if trimmed.contains('\n') {
                    let (first, rest) = trimmed.split_once('\n').unwrap_or((&trimmed, ""));
                    let first = wrap_text(first, self.width.saturating_sub(self.level));
                    if rest.is_empty() {
                        first
                    } else {
                        format!("{first}\n{rest}")
                    }
                } else {
                    wrap_text(&trimmed, self.width.saturating_sub(self.level))
                };
                let item = indentify(&wrapped, indent, Some(&prefix));
                if let Some(p) = parent {
                    if !p.buf.is_empty() {
                        p.buf.push('\n');
                    }
                    p.buf.push_str(&item);
                    if let BlockKind::List { index, .. } = &mut p.kind {
                        *index += 1;
                    }
                }
            }
            TagEnd::Emphasis
            | TagEnd::Strong
            | TagEnd::Strikethrough
            | TagEnd::Link
            | TagEnd::Image => {
                if let Some(top) = self.stack.last_mut() {
                    top.buf.push_str(&frame.buf);
                } else {
                    self.out.push_str(&frame.buf);
                }
            }
            _ => {
                if let Some(top) = self.stack.last_mut() {
                    top.buf.push_str(&frame.buf);
                } else {
                    self.out.push_str(&frame.buf);
                }
            }
        }
    }
}

fn list_lengths(events: &[Event<'_>]) -> Vec<usize> {
    let mut lengths = Vec::new();
    let mut stack: Vec<(usize, usize)> = Vec::new();

    for event in events {
        match event {
            Event::Start(Tag::List(_)) => {
                let id = lengths.len();
                lengths.push(0);
                stack.push((id, 0));
            }
            Event::Start(Tag::Item) => {
                if let Some((_, count)) = stack.last_mut() {
                    *count += 1;
                }
            }
            Event::End(TagEnd::List(_)) => {
                if let Some((id, count)) = stack.pop() {
                    lengths[id] = count;
                }
            }
            _ => {}
        }
    }

    lengths
}

fn collapse_ws(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    let mut prev_space = false;
    for c in s.chars() {
        let is_space = c == '\n' || c == ' ';
        if is_space {
            if !prev_space {
                out.push(' ');
            }
            prev_space = true;
        } else {
            out.push(c);
            prev_space = false;
        }
    }
    out
}

fn wrap_text(text: &str, width: usize) -> String {
    if width == 0 {
        return text.trim().to_string();
    }
    // Match the JS `word-wrap` package: break only on ASCII spaces, never split words.
    let opts = textwrap::Options::new(width)
        .break_words(false)
        .word_separator(textwrap::WordSeparator::AsciiSpace)
        .word_splitter(textwrap::WordSplitter::NoHyphenation);
    textwrap::fill(text.trim(), opts)
}

fn indentify(text: &str, indent: usize, start: Option<&str>) -> String {
    if text.is_empty() {
        return text.to_string();
    }
    let pad = " ".repeat(indent);
    let prefix = match start {
        Some(s) if s.len() <= indent => {
            let mut p = String::from(s);
            p.push_str(&" ".repeat(indent - s.len()));
            p
        }
        Some(s) => s.to_string(),
        None => pad.clone(),
    };
    let mut out = String::with_capacity(text.len() + indent + 8);
    out.push_str(&prefix);
    for (i, line) in text.split('\n').enumerate() {
        if i > 0 {
            out.push('\n');
            out.push_str(&pad);
        }
        out.push_str(line);
    }
    out
}

#[allow(dead_code)]
fn level_for_heading(level: HeadingLevel) -> usize {
    match level {
        HeadingLevel::H1 => 1,
        HeadingLevel::H2 => 2,
        HeadingLevel::H3 => 3,
        HeadingLevel::H4 => 4,
        HeadingLevel::H5 => 5,
        HeadingLevel::H6 => 6,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn renders_paragraph_wrapping_like_typescript() {
        assert_eq!(
            markdown_to_text(
                "This paragraph has enough words to wrap onto multiple lines while preserving words and trimming surrounding whitespace.",
                40,
            ),
            "This paragraph has enough words to wrap\nonto multiple lines while preserving\nwords and trimming surrounding\nwhitespace."
        );
    }

    #[test]
    fn renders_nested_lists_like_typescript() {
        assert_eq!(
            markdown_to_text(
                "- First item with enough words to wrap under the bullet marker nicely\n  - Nested item that also wraps after a few words\n- Second item",
                40,
            ),
            "- First item with enough words to wrap\n  under the bullet marker nicely\n  - Nested item that also wraps after a\n    few words\n- Second item"
        );
    }

    #[test]
    fn renders_ordered_list_indent_like_typescript() {
        assert_eq!(
            markdown_to_text(
                "9. Nine has text that wraps onto another line for indentation\n10. Ten has text too",
                40,
            ),
            "9.  Nine has text that wraps onto\n    another line for indentation\n10. Ten has text too"
        );
    }

    #[test]
    fn renders_blockquote_code_and_rule_like_typescript() {
        assert_eq!(
            markdown_to_text(
                "> Quoted paragraph with several words that should wrap.\n\n```\ncode line\nnext line\n```\n\n---",
                40,
            ),
            "  Quoted paragraph with several words\n  that should wrap.\n  \n\n  code line\n  next line\n\n----------------------------------------"
        );
    }

    #[test]
    fn renders_inline_markup_like_typescript() {
        assert_eq!(
            markdown_to_text(
                "A **strong** and *emphasized* [link](https://example.com) plus `code` and  multiple   spaces.",
                40,
            ),
            "A strong and emphasized link plus code\nand multiple spaces."
        );
    }
}
