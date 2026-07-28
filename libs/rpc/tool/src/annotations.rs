//! Parses the `@rpc(...)` / `@rpc.param(...)` tag vocabulary out of a raw
//! doc-comment string. Tags are found by regex anywhere in the comment text,
//! so they work whether the house `//!` style, `///`, or a `/** */` block is
//! used above the declaration — the tool doesn't care about comment
//! delimiters, only about the tags inside them.

use anyhow::{bail, Result};
use regex::Regex;

use crate::ir::Direction;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ParamKindTag {
    CString,
    Bytes,
}

#[derive(Debug, Clone)]
pub struct ParamTag {
    pub name: String,
    pub direction: Direction,
    pub kind: Option<ParamKindTag>,
    pub size: Option<String>,
    pub dbus_type: Option<DbusTypeTag>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DbusTypeTag {
    Byte,
    Boolean,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Double,
    String,
    ObjectPath,
    Signature,
    UnixFd,
    Variant,
}

fn parse_dbus_type(value: &str) -> Result<DbusTypeTag> {
    match value {
        "byte" => Ok(DbusTypeTag::Byte),
        "boolean" | "bool" => Ok(DbusTypeTag::Boolean),
        "int16" => Ok(DbusTypeTag::Int16),
        "uint16" => Ok(DbusTypeTag::UInt16),
        "int32" => Ok(DbusTypeTag::Int32),
        "uint32" => Ok(DbusTypeTag::UInt32),
        "int64" => Ok(DbusTypeTag::Int64),
        "uint64" => Ok(DbusTypeTag::UInt64),
        "double" => Ok(DbusTypeTag::Double),
        "string" => Ok(DbusTypeTag::String),
        "object_path" => Ok(DbusTypeTag::ObjectPath),
        "signature" => Ok(DbusTypeTag::Signature),
        "unix_fd" => Ok(DbusTypeTag::UnixFd),
        "variant" => Ok(DbusTypeTag::Variant),
        other => bail!(
            "unsupported DBus type '{other}'; supported types: byte, boolean, int16, uint16, \
             int32, uint32, int64, uint64, double, string, object_path, signature, unix_fd, variant"
        ),
    }
}

#[derive(Debug, Clone)]
pub struct ServiceTag {
    /// The protobuf package component of the public wire service name.
    pub proto_package: String,
    /// The unqualified protobuf service identifier.
    pub name: String,
    /// From `keyed_by="..."`: the class has multiple live C++ instances,
    /// distinguished by a key of this type (a recognized primitive name
    /// like "int32"/"string", or a fully-qualified enum type name resolved
    /// against the whole translation unit). Every generated RPC on this
    /// service gets an extra `id` request field of that type, and the
    /// adapter resolves the target instance from an `rpc::InstanceRegistry`
    /// instead of holding a single fixed reference — the gRPC analog of
    /// DBus's per-object-path routing (`IDBus::connect(path, iface, obj)`).
    pub keyed_by: Option<String>,
}

/// `@rpc(service="package.Name"[, keyed_by="Type"])` on a class — marks it
/// as an RPC-exposed interface, optionally with multiple live instances keyed
/// by `Type`. The fully-qualified service value is the source of truth for
/// the gRPC wire name: `package.Name` becomes `package Name` in the generated
/// `.proto` schema.
pub fn parse_service_tag(comment: &str) -> Result<Option<ServiceTag>> {
    let outer = Regex::new(r#"@rpc\(([^)]*)\)"#).expect("valid regex");
    let attr = Regex::new(r#"([A-Za-z_]\w*)\s*=\s*"([^"]*)""#).expect("valid regex");

    for caps in outer.captures_iter(comment) {
        let inner = &caps[1];
        let mut name = None;
        let mut keyed_by = None;
        for a in attr.captures_iter(inner) {
            match &a[1] {
                "service" => name = Some(a[2].to_string()),
                "keyed_by" => keyed_by = Some(a[2].to_string()),
                _ => {}
            }
        }
        if let Some(name) = name {
            let (proto_package, name) = name.rsplit_once('.').ok_or_else(|| {
                anyhow::anyhow!(
                    "@rpc(service=\"...\") must be a fully-qualified protobuf service name, \
                     e.g. @rpc(service=\"workrave.CoreService\")"
                )
            })?;
            if proto_package.is_empty()
                || name.is_empty()
                || !proto_package.split('.').all(is_proto_identifier)
                || !is_proto_identifier(name)
            {
                bail!(
                    "invalid protobuf service name '{proto_package}.{name}'; expected \
                     dot-separated protobuf identifiers, e.g. workrave.CoreService"
                );
            }
            return Ok(Some(ServiceTag {
                proto_package: proto_package.to_string(),
                name: name.to_string(),
                keyed_by,
            }));
        }
    }
    Ok(None)
}

fn is_proto_identifier(value: &str) -> bool {
    let mut chars = value.chars();
    matches!(chars.next(), Some(c) if c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_')
}

/// `@rpc.dbus(interface="org.workrave.CoreInterface")` on a class — opts
/// the same annotated interface into DBus generation too, alongside gRPC.
/// Orthogonal to `@rpc(service=...)`: method/signal DBus names reuse the
/// existing `@rpc(name=...)`/`@rpc.signal(name=...)` tags unchanged (DBus's
/// PascalCase convention already matches), this only supplies the one
/// DBus-specific thing gRPC has no equivalent of — the dotted interface
/// name.
pub fn parse_dbus_tag(comment: &str) -> Option<String> {
    let re = Regex::new(r#"@rpc\.dbus\(\s*interface\s*=\s*"([^"]+)"\s*\)"#).expect("valid regex");
    re.captures(comment).map(|c| c[1].to_string())
}

/// `@rpc.dbus(return_type="...")` changes only a method's DBus scalar wire
/// type; its native C++ and gRPC types stay unchanged.
pub fn parse_dbus_return_type(comment: &str) -> Result<Option<DbusTypeTag>> {
    let re = Regex::new(r#"@rpc\.dbus\(\s*return_type\s*=\s*"([^"]+)"\s*\)"#).expect("valid regex");
    let Some(caps) = re.captures(comment) else {
        return Ok(None);
    };
    parse_dbus_type(&caps[1]).map(Some)
}

/// `@rpc(name="Name")` on a method (or a specific overload) — marks it as an RPC.
pub fn parse_method_tag(comment: &str) -> Option<String> {
    let re = Regex::new(r#"@rpc\(\s*name\s*=\s*"([^"]+)"\s*\)"#).expect("valid regex");
    re.captures(comment).map(|c| c[1].to_string())
}

/// `@rpc.enum(name="dbus_style_name")` on an enum type declaration — pins an
/// explicit, backend-agnostic canonical name for the enum itself, e.g.
/// matching `workrave-service.xml`'s `<enum name="operation_mode">` for
/// `workrave::OperationMode`. Doesn't change what the gRPC backend emits
/// (still the auto-derived protobuf enum name); carried through the IR for
/// a future backend (e.g. DBus) that needs to reproduce an exact existing
/// wire name.
pub fn parse_enum_tag(comment: &str) -> Option<String> {
    let re = Regex::new(r#"@rpc\.enum\(\s*name\s*=\s*"([^"]+)"\s*\)"#).expect("valid regex");
    re.captures(comment).map(|c| c[1].to_string())
}

/// `@rpc.enum.value(name="dbus_style_name")` on one enumerator — the
/// per-value analog of `@rpc.enum(...)` above, e.g. matching
/// `workrave-service.xml`'s `<value name="normal" csymbol="...Normal"/>`
/// for `workrave::OperationMode::Normal`.
pub fn parse_enum_value_tag(comment: &str) -> Option<String> {
    let re = Regex::new(r#"@rpc\.enum\.value\(\s*name\s*=\s*"([^"]+)"\s*\)"#).expect("valid regex");
    re.captures(comment).map(|c| c[1].to_string())
}

/// `@rpc.bitmask` on a class template — opts a `Flags<Enum>`-shaped type
/// (see `libs/utils/include/utils/Enum.hh`) into being recognized wherever
/// it's used as an `@rpc` parameter/return type, mapped to `repeated Enum`.
/// A presence marker, not a `name=...`-style tag: it says once, on the
/// template itself, that the whole pattern is RPC-representable, rather than
/// requiring every call site to say so.
pub fn has_bitmask_tag(comment: &str) -> bool {
    let re = Regex::new(r#"@rpc\.bitmask\b"#).expect("valid regex");
    re.is_match(comment)
}

#[derive(Debug, Clone)]
pub struct SignalTag {
    pub name: String,
    /// From `fields="a,b,..."`: names for the signal's arguments, in order.
    /// Required when the signal has more than one argument (there's no
    /// source of parameter names to infer from — `boost::signals2::signal`'s
    /// template argument is a bare function *type*, `void(Foo)`, with no
    /// parameter identifiers). Omitted for the common single-argument case,
    /// which defaults to a field named "value".
    pub fields: Option<Vec<String>>,
    pub dbus_types: Option<Vec<DbusTypeTag>>,
}

/// `@rpc.signal(name="Name"[, fields="a,b,..."])` on a
/// `boost::signals2::signal<void(Args...)> &` accessor — marks it as a
/// push event source (a server-streaming RPC), the gRPC analog of a DBus
/// signal.
pub fn parse_signal_tag(comment: &str) -> Result<Option<SignalTag>> {
    let re = Regex::new(
        r#"@rpc\.signal\(\s*name\s*=\s*"([^"]+)"\s*(?:,\s*fields\s*=\s*"([^"]*)"\s*)?(?:,\s*dbus_types\s*=\s*"([^"]*)"\s*)?\)"#,
    )
    .expect("valid regex");
    let Some(caps) = re.captures(comment) else {
        return Ok(None);
    };
    let name = caps[1].to_string();
    let fields = caps.get(2).map(|m| {
        m.as_str()
            .split(',')
            .map(|s| s.trim().to_string())
            .collect()
    });
    let dbus_types = caps
        .get(3)
        .map(|m| {
            m.as_str()
                .split(',')
                .map(|value| parse_dbus_type(value.trim()))
                .collect::<Result<Vec<_>>>()
        })
        .transpose()?;
    Ok(Some(SignalTag {
        name,
        fields,
        dbus_types,
    }))
}

/// `@rpc.param(name, dir=in|out|inout[, kind=cstring|bytes][, size=other])`,
/// zero or more per method comment.
pub fn parse_param_tags(comment: &str) -> Result<Vec<ParamTag>> {
    let re = Regex::new(
        r#"@rpc\.param\(\s*([A-Za-z_]\w*)\s*,\s*dir\s*=\s*(in|out|inout)(?:\s*,\s*kind\s*=\s*(cstring|bytes))?(?:\s*,\s*size\s*=\s*([A-Za-z_]\w*))?(?:\s*,\s*dbus_type\s*=\s*"?([A-Za-z0-9_]+)"?)?\s*\)"#,
    )
    .expect("valid regex");

    let mut tags = Vec::new();
    for caps in re.captures_iter(comment) {
        let name = caps[1].to_string();
        let direction = match &caps[2] {
            "in" => Direction::In,
            "out" => Direction::Out,
            "inout" => Direction::InOut,
            other => unreachable!("regex only matches in|out|inout, got {other}"),
        };
        let kind = caps.get(3).map(|m| match m.as_str() {
            "cstring" => ParamKindTag::CString,
            "bytes" => ParamKindTag::Bytes,
            other => unreachable!("regex only matches cstring|bytes, got {other}"),
        });
        let size = caps.get(4).map(|m| m.as_str().to_string());
        let dbus_type = caps
            .get(5)
            .map(|m| parse_dbus_type(m.as_str()))
            .transpose()?;

        if kind == Some(ParamKindTag::Bytes) && size.is_none() {
            bail!("@rpc.param({name}, kind=bytes, ...) requires a size=<param-name> entry");
        }

        tags.push(ParamTag {
            name,
            direction,
            kind,
            size,
            dbus_type,
        });
    }
    Ok(tags)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn service_tag() {
        let c = "//! Main backend implementation.\n//! @rpc(service=\"workrave.CoreService\")";
        let tag = parse_service_tag(c).unwrap().unwrap();
        assert_eq!(tag.proto_package, "workrave");
        assert_eq!(tag.name, "CoreService");
        assert_eq!(tag.keyed_by, None);
        assert_eq!(parse_method_tag(c), None);
    }

    #[test]
    fn service_tag_keyed_by() {
        let c = "//! @rpc(service=\"workrave.BreakService\", keyed_by=\"workrave::BreakId\")";
        let tag = parse_service_tag(c).unwrap().unwrap();
        assert_eq!(tag.name, "BreakService");
        assert_eq!(tag.keyed_by.as_deref(), Some("workrave::BreakId"));
    }

    #[test]
    fn service_tag_requires_a_fully_qualified_wire_name() {
        let err = parse_service_tag("//! @rpc(service=\"CoreService\")").unwrap_err();
        assert!(format!("{err:#}").contains("fully-qualified protobuf service name"));
    }

    #[test]
    fn method_tag() {
        let c = "//! Sets the operation mode.\n//! @rpc(name=\"SetOperationMode\")";
        assert_eq!(parse_method_tag(c), Some("SetOperationMode".to_string()));
    }

    #[test]
    fn param_tags_plain() {
        let c = "//! @rpc(name=\"GetMode\")\n//! @rpc.param(mode, dir=out)";
        let tags = parse_param_tags(c).unwrap();
        assert_eq!(tags.len(), 1);
        assert_eq!(tags[0].name, "mode");
        assert_eq!(tags[0].direction, Direction::Out);
        assert_eq!(tags[0].kind, None);
    }

    #[test]
    fn param_tags_bytes() {
        let c = "//! @rpc(name=\"ReadChunk\")\n//! @rpc.param(data, dir=out, kind=bytes, size=len)";
        let tags = parse_param_tags(c).unwrap();
        assert_eq!(tags.len(), 1);
        assert_eq!(tags[0].kind, Some(ParamKindTag::Bytes));
        assert_eq!(tags[0].size.as_deref(), Some("len"));
    }

    #[test]
    fn param_tags_bytes_without_size_errors() {
        let c = "//! @rpc.param(data, dir=out, kind=bytes)";
        assert!(parse_param_tags(c).is_err());
    }

    #[test]
    fn dbus_tag() {
        let c = "// @rpc.dbus(interface=\"org.workrave.CoreInterface\")";
        assert_eq!(
            parse_dbus_tag(c),
            Some("org.workrave.CoreInterface".to_string())
        );
        assert_eq!(parse_dbus_tag("// no tag here"), None);
    }

    #[test]
    fn dbus_return_type_tag() {
        assert_eq!(
            parse_dbus_return_type("// @rpc.dbus(return_type=\"int32\")").unwrap(),
            Some(DbusTypeTag::Int32)
        );
        assert_eq!(
            parse_dbus_return_type("// @rpc.dbus(return_type=\"byte\")").unwrap(),
            Some(DbusTypeTag::Byte)
        );
    }

    #[test]
    fn enum_tag() {
        let c = "// @rpc.enum(name=\"operation_mode\")";
        assert_eq!(parse_enum_tag(c), Some("operation_mode".to_string()));
        assert_eq!(parse_enum_tag("// no tag here"), None);
    }

    #[test]
    fn enum_value_tag() {
        let c = "// @rpc.enum.value(name=\"normal\")";
        assert_eq!(parse_enum_value_tag(c), Some("normal".to_string()));
        assert_eq!(parse_enum_value_tag("// no tag here"), None);
    }

    #[test]
    fn bitmask_tag_present() {
        let c = "// @rpc.bitmask\n// Any Flags<Enum> is RPC-representable.";
        assert!(has_bitmask_tag(c));
    }

    #[test]
    fn bitmask_tag_absent() {
        let c = "// A plain doc comment with no tags at all.";
        assert!(!has_bitmask_tag(c));
    }

    #[test]
    fn signal_tag_plain() {
        let c = "//! @rpc.signal(name=\"OperationModeChanged\")";
        let tag = parse_signal_tag(c).unwrap().unwrap();
        assert_eq!(tag.name, "OperationModeChanged");
        assert_eq!(tag.fields, None);
    }

    #[test]
    fn signal_tag_with_fields() {
        let c = "//! @rpc.signal(name=\"Progress\", fields=\"stage, percent\")";
        let tag = parse_signal_tag(c).unwrap().unwrap();
        assert_eq!(tag.name, "Progress");
        assert_eq!(
            tag.fields,
            Some(vec!["stage".to_string(), "percent".to_string()])
        );
    }

    #[test]
    fn all_dbus_type_overrides() {
        let cases = [
            ("byte", DbusTypeTag::Byte),
            ("boolean", DbusTypeTag::Boolean),
            ("bool", DbusTypeTag::Boolean),
            ("int16", DbusTypeTag::Int16),
            ("uint16", DbusTypeTag::UInt16),
            ("int32", DbusTypeTag::Int32),
            ("uint32", DbusTypeTag::UInt32),
            ("int64", DbusTypeTag::Int64),
            ("uint64", DbusTypeTag::UInt64),
            ("double", DbusTypeTag::Double),
            ("string", DbusTypeTag::String),
            ("object_path", DbusTypeTag::ObjectPath),
            ("signature", DbusTypeTag::Signature),
            ("unix_fd", DbusTypeTag::UnixFd),
            ("variant", DbusTypeTag::Variant),
        ];
        for (name, expected) in cases {
            let params = parse_param_tags(&format!(
                "// @rpc.param(value, dir=in, dbus_type=\"{name}\")"
            ))
            .unwrap();
            assert_eq!(params[0].dbus_type, Some(expected));
            assert_eq!(
                parse_dbus_return_type(&format!("// @rpc.dbus(return_type=\"{name}\")")).unwrap(),
                Some(expected)
            );
        }

        let signal = parse_signal_tag(
            "// @rpc.signal(name=\"Changed\", fields=\"a,b,c\", dbus_types=\"byte,object_path,unix_fd\")",
        )
        .unwrap()
        .unwrap();
        assert_eq!(
            signal.dbus_types,
            Some(vec![
                DbusTypeTag::Byte,
                DbusTypeTag::ObjectPath,
                DbusTypeTag::UnixFd
            ])
        );
    }
}
