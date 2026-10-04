//! The GitHub expression subset used by Ship workflows. Unsupported grammar
//! and functions are errors, including in branches that are not evaluated.

use anyhow::{bail, Context, Result};
use serde_json::{json, Value};

#[derive(Debug, Clone, Copy)]
pub struct Status {
    pub success: bool,
    pub failure: bool,
    pub cancelled: bool,
}
impl Default for Status {
    fn default() -> Self {
        Self {
            success: true,
            failure: false,
            cancelled: false,
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
enum Token {
    Name(String),
    Literal(Value),
    Symbol(&'static str),
    End,
}

fn tokens(source: &str) -> Result<Vec<Token>> {
    let mut rest = source;
    let mut out = Vec::new();
    while !rest.is_empty() {
        rest = rest.trim_start();
        if rest.is_empty() {
            break;
        }
        if rest.starts_with('\'') {
            rest = &rest[1..];
            let mut value = String::new();
            loop {
                let end = rest.find('\'').context("unterminated expression string")?;
                value.push_str(&rest[..end]);
                rest = &rest[end + 1..];
                if rest.starts_with('\'') {
                    value.push('\'');
                    rest = &rest[1..];
                } else {
                    break;
                }
            }
            out.push(Token::Literal(Value::String(value)));
            continue;
        }
        if let Some(symbol) = [
            "&&", "||", "==", "!=", "<=", ">=", "!", "<", ">", "(", ")", "[", "]", ".", ",",
        ]
        .into_iter()
        .find(|symbol| rest.starts_with(symbol))
        {
            out.push(Token::Symbol(symbol));
            rest = &rest[symbol.len()..];
            continue;
        }
        let first = rest.as_bytes()[0];
        if first.is_ascii_digit() || first == b'-' {
            let end = rest
                .find(|c: char| !(c.is_ascii_alphanumeric() || matches!(c, '.' | '+' | '-')))
                .unwrap_or(rest.len());
            let raw = &rest[..end];
            let (sign, digits) = raw
                .strip_prefix('-')
                .map(|v| (-1.0, v))
                .unwrap_or((1.0, raw));
            let value = if let Some(hex) = digits
                .strip_prefix("0x")
                .or_else(|| digits.strip_prefix("0X"))
            {
                json!(
                    sign * u64::from_str_radix(hex, 16).context("invalid hexadecimal literal")?
                        as f64
                )
            } else {
                serde_json::from_str::<Value>(raw).context("invalid numeric literal")?
            };
            out.push(Token::Literal(value));
            rest = &rest[end..];
            continue;
        }
        if first.is_ascii_alphabetic() || first == b'_' {
            let end = rest
                .find(|c: char| !(c.is_ascii_alphanumeric() || matches!(c, '_' | '-')))
                .unwrap_or(rest.len());
            let name = &rest[..end];
            out.push(match name.to_ascii_lowercase().as_str() {
                "true" => Token::Literal(json!(true)),
                "false" => Token::Literal(json!(false)),
                "null" => Token::Literal(Value::Null),
                _ => Token::Name(name.into()),
            });
            rest = &rest[end..];
            continue;
        }
        bail!("unsupported expression syntax near `{rest}`");
    }
    out.push(Token::End);
    Ok(out)
}

#[derive(Debug)]
pub enum Expr {
    Literal(Value),
    Name(String),
    Get(Box<Expr>, Box<Expr>),
    Not(Box<Expr>),
    Binary(&'static str, Box<Expr>, Box<Expr>),
    Call(String, Vec<Expr>),
}

struct Parser {
    tokens: Vec<Token>,
    at: usize,
}
impl Parser {
    fn peek(&self) -> &Token {
        &self.tokens[self.at]
    }
    fn take(&mut self) -> Token {
        let token = self.peek().clone();
        self.at += 1;
        token
    }
    fn eat(&mut self, symbol: &str) -> bool {
        if matches!(self.peek(), Token::Symbol(value) if *value == symbol) {
            self.at += 1;
            true
        } else {
            false
        }
    }
    fn expect(&mut self, symbol: &str) -> Result<()> {
        if !self.eat(symbol) {
            bail!("expected '{symbol}' in expression, found {:?}", self.peek());
        }
        Ok(())
    }
    fn expression(&mut self, minimum: u8) -> Result<Expr> {
        let mut left = if self.eat("!") {
            Expr::Not(Box::new(self.expression(5)?))
        } else {
            self.primary()?
        };
        loop {
            let (operator, priority) = match self.peek() {
                Token::Symbol("||") => ("||", 1),
                Token::Symbol("&&") => ("&&", 2),
                Token::Symbol("==") => ("==", 3),
                Token::Symbol("!=") => ("!=", 3),
                Token::Symbol("<") => ("<", 4),
                Token::Symbol("<=") => ("<=", 4),
                Token::Symbol(">") => (">", 4),
                Token::Symbol(">=") => (">=", 4),
                _ => break,
            };
            if priority < minimum {
                break;
            }
            self.at += 1;
            left = Expr::Binary(
                operator,
                Box::new(left),
                Box::new(self.expression(priority + 1)?),
            );
        }
        Ok(left)
    }
    fn primary(&mut self) -> Result<Expr> {
        let mut value = if self.eat("(") {
            let value = self.expression(1)?;
            self.expect(")")?;
            value
        } else {
            match self.take() {
                Token::Literal(value) => Expr::Literal(value),
                Token::Name(name) => Expr::Name(name),
                other => bail!("expected an expression value, found {other:?}"),
            }
        };
        loop {
            if self.eat(".") {
                let Token::Name(key) = self.take() else {
                    bail!("expected a property name after '.'");
                };
                value = Expr::Get(Box::new(value), Box::new(Expr::Literal(json!(key))));
            } else if self.eat("[") {
                let key = self.expression(1)?;
                self.expect("]")?;
                value = Expr::Get(Box::new(value), Box::new(key));
            } else if self.eat("(") {
                let name = match &value {
                    Expr::Name(name) => name.clone(),
                    Expr::Get(base, key) => match (&**base, &**key) {
                        (Expr::Name(base), Expr::Literal(Value::String(key)))
                            if base.eq_ignore_ascii_case("ship") =>
                        {
                            format!("ship.{key}")
                        }
                        _ => bail!("unsupported expression function"),
                    },
                    _ => bail!("unsupported expression function"),
                };
                let mut args = Vec::new();
                if !self.eat(")") {
                    loop {
                        args.push(self.expression(1)?);
                        if self.eat(")") {
                            break;
                        }
                        self.expect(",")?;
                    }
                }
                validate_call(&name, args.len())?;
                value = Expr::Call(name, args);
            } else {
                break;
            }
        }
        Ok(value)
    }
}

fn validate_call(name: &str, count: usize) -> Result<()> {
    let valid = match name.to_ascii_lowercase().as_str() {
        "success" | "failure" | "cancelled" | "always" => count == 0,
        "fromjson" | "tojson" | "ship.secret" | "ship.exists" | "ship.glob" | "ship.sha256"
        | "ship.lower" | "ship.msys" => count == 1,
        "contains" | "startswith" | "endswith" => count == 2,
        "join" => (1..=2).contains(&count),
        "format" => count >= 1,
        "ship.today" => count <= 1,
        "ship.replace" => count == 3,
        "ship.slice" => (2..=3).contains(&count),
        _ => bail!("unsupported expression function '{name}'"),
    };
    if !valid {
        bail!("wrong number of arguments for '{name}'");
    }
    Ok(())
}

pub fn parse(source: &str) -> Result<Expr> {
    let mut parser = Parser {
        tokens: tokens(source)?,
        at: 0,
    };
    let expression = parser.expression(1)?;
    if parser.peek() != &Token::End {
        bail!("unexpected expression token {:?}", parser.peek());
    }
    Ok(expression)
}

impl Expr {
    fn path(&self) -> Option<Vec<String>> {
        match self {
            Self::Name(name) => Some(vec![name.clone()]),
            Self::Get(base, key) => match &**key {
                Self::Literal(Value::String(key)) => {
                    let mut path = base.path()?;
                    path.push(key.clone());
                    Some(path)
                }
                _ => None,
            },
            _ => None,
        }
    }
    pub fn has_status_check(&self) -> bool {
        match self {
            Self::Call(name, args) => {
                matches!(
                    name.to_ascii_lowercase().as_str(),
                    "success" | "failure" | "always" | "cancelled"
                ) || args.iter().any(Self::has_status_check)
            }
            Self::Get(a, b) | Self::Binary(_, a, b) => a.has_status_check() || b.has_status_check(),
            Self::Not(a) => a.has_status_check(),
            _ => false,
        }
    }
    pub fn references(&self, root: &str) -> bool {
        match self {
            Self::Name(name) => name.eq_ignore_ascii_case(root),
            Self::Call(_, args) => args.iter().any(|a| a.references(root)),
            Self::Get(a, b) | Self::Binary(_, a, b) => a.references(root) || b.references(root),
            Self::Not(a) => a.references(root),
            _ => false,
        }
    }
    pub fn eval(
        &self,
        root: &Value,
        status: Status,
        helper: &dyn Fn(&str, &[Value]) -> Result<Value>,
    ) -> Result<Value> {
        let eval = |value: &Expr| value.eval(root, status, helper);
        Ok(match self {
            Self::Literal(value) => value.clone(),
            Self::Name(name) => member(root, name)
                .cloned()
                .with_context(|| format!("unknown expression context '{name}'"))?,
            Self::Get(base, key) => {
                let key = eval(key)?;
                if matches!(&**base, Self::Name(name) if name.eq_ignore_ascii_case("vars")) {
                    if let Some(reason) = key
                        .as_str()
                        .and_then(|key| root["__ship_vars_errors"].get(key))
                        .and_then(Value::as_str)
                    {
                        bail!("computed variable {} is unavailable: {reason}", text(&key)?);
                    }
                }
                let base = eval(base)?;
                if let Some(path) = unknown_path(&base) {
                    return Ok(unknown(&format!("{path}.{}", text(&key)?)));
                }
                if root["__ship_preview"] == true {
                    if let Some(path) = self.path() {
                        if path.len() == 4
                            && path[0] == "steps"
                            && path[2] == "outputs"
                            && root["steps"].get(&path[1]).is_some()
                            && base.get(&path[3]).is_none()
                        {
                            return Ok(unknown(&path.join(".")));
                        }
                    }
                }
                match (&base, &key) {
                    (Value::Object(_), Value::String(key)) => {
                        member(&base, key).cloned().unwrap_or(Value::Null)
                    }
                    (Value::Array(values), _) => {
                        let n = number(&key);
                        if n >= 0.0 && n.fract() == 0.0 {
                            values.get(n as usize).cloned().unwrap_or(Value::Null)
                        } else {
                            Value::Null
                        }
                    }
                    _ => Value::Null,
                }
            }
            Self::Not(value) => json!(!truthy(&eval(value)?)),
            Self::Binary("&&", a, b) => {
                let a = eval(a)?;
                if truthy(&a) {
                    eval(b)?
                } else {
                    a
                }
            }
            Self::Binary("||", a, b) => {
                let a = eval(a)?;
                if truthy(&a) {
                    a
                } else {
                    eval(b)?
                }
            }
            Self::Binary(op, a, b) => {
                let a = eval(a)?;
                let b = eval(b)?;
                let result = match *op {
                    "==" => equal(&a, &b)?,
                    "!=" => !equal(&a, &b)?,
                    op => {
                        let ordering = match (&a, &b) {
                            (Value::String(a), Value::String(b)) => {
                                a.to_lowercase().partial_cmp(&b.to_lowercase())
                            }
                            _ => number(&a).partial_cmp(&number(&b)),
                        };
                        match op {
                            "<" => ordering.is_some_and(|o| o.is_lt()),
                            "<=" => ordering.is_some_and(|o| o.is_le()),
                            ">" => ordering.is_some_and(|o| o.is_gt()),
                            ">=" => ordering.is_some_and(|o| o.is_ge()),
                            _ => unreachable!(),
                        }
                    }
                };
                json!(result)
            }
            Self::Call(name, args) => {
                let args = args.iter().map(eval).collect::<Result<Vec<_>>>()?;
                match name.to_ascii_lowercase().as_str() {
                    "success" => json!(status.success),
                    "failure" => json!(status.failure),
                    "cancelled" => json!(status.cancelled),
                    "always" => json!(true),
                    "fromjson" => {
                        if unknown_path(&args[0]).is_some() {
                            args[0].clone()
                        } else {
                            let value = text(&args[0])?;
                            if root["__ship_preview"] == true
                                && value.starts_with('<')
                                && value.ends_with('>')
                            {
                                unknown(&value[1..value.len() - 1])
                            } else {
                                serde_json::from_str(&value).context("invalid fromJSON value")?
                            }
                        }
                    }
                    "tojson" => json!(serde_json::to_string_pretty(&args[0])?),
                    "format" => json!(format_values(&text(&args[0])?, &args[1..])?),
                    "contains" => json!(match &args[0] {
                        Value::Array(values) => values
                            .iter()
                            .map(|v| equal(v, &args[1]))
                            .collect::<Result<Vec<_>>>()?
                            .into_iter()
                            .any(|v| v),
                        _ => text(&args[0])?
                            .to_lowercase()
                            .contains(&text(&args[1])?.to_lowercase()),
                    }),
                    "startswith" => json!(text(&args[0])?
                        .to_lowercase()
                        .starts_with(&text(&args[1])?.to_lowercase())),
                    "endswith" => json!(text(&args[0])?
                        .to_lowercase()
                        .ends_with(&text(&args[1])?.to_lowercase())),
                    "join" => {
                        let separator = args
                            .get(1)
                            .map(text)
                            .transpose()?
                            .unwrap_or_else(|| ",".into());
                        json!(match &args[0] {
                            Value::Array(values) => values
                                .iter()
                                .map(text)
                                .collect::<Result<Vec<_>>>()?
                                .join(&separator),
                            value => text(value)?,
                        })
                    }
                    "ship.lower" => json!(text(&args[0])?.to_lowercase()),
                    "ship.replace" => {
                        json!(text(&args[0])?.replace(&text(&args[1])?, &text(&args[2])?))
                    }
                    "ship.slice" => {
                        let value = text(&args[0])?;
                        let start = args[1]
                            .as_u64()
                            .context("ship.slice start must be a nonnegative integer")?
                            as usize;
                        let end = args
                            .get(2)
                            .map(|v| {
                                v.as_u64()
                                    .context("ship.slice end must be a nonnegative integer")
                            })
                            .transpose()?
                            .map(|n| n as usize)
                            .unwrap_or(usize::MAX);
                        json!(value
                            .chars()
                            .skip(start)
                            .take(end.saturating_sub(start))
                            .collect::<String>())
                    }
                    _ => helper(name, &args)?,
                }
            }
        })
    }
}

fn member<'a>(value: &'a Value, key: &str) -> Option<&'a Value> {
    value
        .as_object()?
        .iter()
        .find(|(name, _)| name.eq_ignore_ascii_case(key))
        .map(|(_, value)| value)
}
pub fn truthy(value: &Value) -> bool {
    match value {
        Value::Null => false,
        Value::Bool(b) => *b,
        Value::Number(n) => n.as_f64().is_some_and(|n| n != 0.0),
        Value::String(s) => !s.is_empty(),
        _ => true,
    }
}
fn unknown(path: &str) -> Value {
    json!({"__ship_preview_value": path})
}
fn unknown_path(value: &Value) -> Option<&str> {
    value.get("__ship_preview_value").and_then(Value::as_str)
}
pub fn text(value: &Value) -> Result<String> {
    if let Some(path) = unknown_path(value) {
        return Ok(format!("<{path}>"));
    }
    Ok(match value {
        Value::Null => String::new(),
        Value::String(s) => s.clone(),
        Value::Bool(_) => value.to_string(),
        Value::Number(n) => n.as_f64().context("invalid number")?.to_string(),
        _ => bail!("arrays and objects require toJSON() when inserted into text"),
    })
}
fn number(value: &Value) -> f64 {
    match value {
        Value::Null => 0.0,
        Value::Bool(b) => {
            if *b {
                1.0
            } else {
                0.0
            }
        }
        Value::Number(n) => n.as_f64().unwrap_or(f64::NAN),
        Value::String(s) if s.trim().is_empty() => 0.0,
        Value::String(s) => serde_json::from_str::<Value>(s.trim())
            .ok()
            .and_then(|v| v.as_f64())
            .unwrap_or(f64::NAN),
        _ => f64::NAN,
    }
}
fn equal(a: &Value, b: &Value) -> Result<bool> {
    Ok(match (a, b) {
        (Value::String(a), Value::String(b)) => a.to_lowercase() == b.to_lowercase(),
        (Value::Array(_), Value::Array(_)) | (Value::Object(_), Value::Object(_)) => {
            bail!("collection identity comparisons are not supported")
        }
        _ => number(a) == number(b),
    })
}
fn format_values(template: &str, args: &[Value]) -> Result<String> {
    let mut out = String::new();
    let mut chars = template.chars().peekable();
    while let Some(c) = chars.next() {
        match c {
            '{' if chars.peek() == Some(&'{') => {
                chars.next();
                out.push('{');
            }
            '}' if chars.peek() == Some(&'}') => {
                chars.next();
                out.push('}');
            }
            '{' => {
                let mut index = String::new();
                loop {
                    match chars.next() {
                        Some('}') => break,
                        Some(c) if c.is_ascii_digit() => index.push(c),
                        _ => bail!("invalid format placeholder"),
                    }
                }
                let index = index.parse::<usize>().context("invalid format index")?;
                out.push_str(&text(args.get(index).context("format argument missing")?)?);
            }
            '}' => bail!("unescaped closing brace in format"),
            _ => out.push(c),
        }
    }
    Ok(out)
}

/// Find the closing delimiter outside single-quoted expression strings.
pub fn end(source: &str) -> Result<usize> {
    let mut quoted = false;
    let mut i = 0;
    let bytes = source.as_bytes();
    while i < bytes.len() {
        if bytes[i] == b'\'' {
            if quoted && bytes.get(i + 1) == Some(&b'\'') {
                i += 2;
                continue;
            }
            quoted = !quoted;
        }
        if !quoted && source[i..].starts_with("}}") {
            return Ok(i);
        }
        i += source[i..].chars().next().unwrap().len_utf8();
    }
    bail!("unterminated ${{{{ expression")
}

#[cfg(test)]
mod tests {
    use super::*;
    fn evaluate(s: &str) -> Value {
        parse(s)
            .unwrap()
            .eval(
                &json!({"steps":{"cache":{"outputs":{"flag":"false"}}},"inputs": {"flag":false}}),
                Status::default(),
                &|_, _| unreachable!(),
            )
            .unwrap()
    }
    #[test]
    fn coercion_short_circuit_and_string_rules() {
        for expression in [
            "'FALSE' == 'false'",
            "null == false",
            "'' == 0",
            "'1' == true",
            "0xff == 255",
            "'not a number' != 0",
            "!(null > 1)",
            "'false' && true",
            "false || true",
            "steps.cache.outputs.flag != false",
            "inputs.missing == null",
            "inputs.FLAG == false",
            "true || fromJSON('invalid')",
            "startsWith('QtBuild','qt')",
            "contains(fromJSON('[1,2]'), '2')",
        ] {
            assert_eq!(evaluate(expression), json!(true), "{expression}");
        }
        assert_eq!(evaluate("'It''s ready'"), json!("It's ready"));
        assert_eq!(evaluate("false && fromJSON('invalid')"), json!(false));
        assert_eq!(evaluate("fromJSON('[false, 2]')[1]"), json!(2));
        assert_eq!(evaluate("format('{{{0}}}', 'x')"), json!("{x}"));
        assert_eq!(
            evaluate("join(fromJSON('[1, true, null]'), ':')"),
            json!("1:true:")
        );
    }
    #[test]
    fn unsupported_syntax_is_rejected_even_in_unused_branches() {
        for expression in [
            "true || mystery()",
            "a | lower",
            "true and false",
            "\"quoted\"",
            "[1, 2]",
            "a.*.x",
            "fromJSON()",
            "true false",
            "a[1:]",
            "(",
            "!",
        ] {
            assert!(parse(expression).is_err(), "{expression}");
        }
    }
    #[test]
    fn detects_status_checks_and_matrix_references() {
        assert!(parse("!failure() && !cancelled()")
            .unwrap()
            .has_status_check());
        assert!(!parse("contains('always()', 'always')")
            .unwrap()
            .has_status_check());
        assert!(parse("matrix['configuration'] == 'Release'")
            .unwrap()
            .references("matrix"));
        assert!(!parse("'matrix.configuration'")
            .unwrap()
            .references("matrix"));
        assert_eq!(end(" format('}} {0}', 'x') }} rest").unwrap(), 23);
    }
}
