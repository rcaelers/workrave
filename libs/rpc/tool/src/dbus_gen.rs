//! Renders a DBus binding (introspection XML + QtDBus dispatch/marshalling
//! C++) for an `@rpc.dbus(interface="...")`-tagged interface, from the same
//! IR the gRPC backend (`cpp_gen.rs`/`proto_gen.rs`) uses — this is the
//! payoff of keeping that IR free of gRPC-specific concepts from the start.
//!
//! Targets **QtDBus only** (the only backend actually live in a Qt UI
//! build — GDBus needs Linux+GTK). Generated code plugs into the existing,
//! unmodified `libs/dbus` runtime (`IDBus`, `DBusBindingQt`, the generic
//! `DBusMarshall<T>` template and its built-in specializations for
//! scalars/`std::string`/`int64_t`/`uint64_t`/`std::list<V>`/`std::map<K,V>`)
//! — this module only needs to emit *new* `DBusMarshall<T>` specializations
//! for types that runtime header doesn't already cover (enums, structs,
//! `std::vector<V>`, chrono durations, `Flags<Enum>` bitmasks), plus the
//! per-interface dispatch stub itself. It never touches `libs/dbus`.
//!
//! Structurally this mirrors what `libs/dbus/bin/dbusgen.py`'s `qt-cc.jinja`
//! template already produces for the hand-maintained IDL
//! (`workrave-service.xml`) — same base classes, same dispatch-table shape,
//! same `DBusMarshall<T>` specialization shape — so the two are meant to be
//! directly comparable. One deliberate correctness fix along the way: map
//! fields use the DBus-spec-correct `a{kv}` signature, not the old
//! generator's non-conformant `e{kv}`.

use std::collections::HashSet;

use anyhow::{bail, Context, Result};

use crate::ir::{Direction, Interface, Method, ParamKind, ProtoType, Signal, StructDef, Unit};

pub struct RenderedDBusBinding {
    pub header: String,
    pub source: String,
}

/// The DBus type signature for `proto_type` — mirrors dbusgen.py's scheme
/// (notably: enums are wire-encoded as **strings**, matching this tree's
/// `@rpc.enum.value(name="...")` canonical names, not raw integers — DBus
/// has no native enum type), except for maps, which use the DBus-spec-legal
/// `a{kv}` form instead of dbusgen.py's `e{kv}`.
fn dbus_signature(proto_type: &ProtoType, unit: &Unit) -> Result<String> {
    Ok(match proto_type {
        ProtoType::Bool => "b".to_string(),
        ProtoType::Int32 => "i".to_string(),
        ProtoType::Int64 => "x".to_string(),
        ProtoType::UInt32 => "u".to_string(),
        ProtoType::UInt64 => "t".to_string(),
        ProtoType::Double | ProtoType::Float => "d".to_string(),
        ProtoType::String => "s".to_string(),
        ProtoType::Bytes => "ay".to_string(),
        ProtoType::Enum(_) => "s".to_string(),
        ProtoType::Message(name) => {
            let struct_def = unit
                .find_struct_by_proto_name(name)
                .with_context(|| format!("struct '{name}' not registered"))?;
            let mut sig = String::from("(");
            for f in &struct_def.fields {
                sig.push_str(&dbus_signature(&f.proto_type, unit)?);
            }
            sig.push(')');
            sig
        }
        ProtoType::Repeated(inner) => format!("a{}", dbus_signature(inner, unit)?),
        ProtoType::Map(key, value) => format!(
            "a{{{}{}}}",
            dbus_signature(key, unit)?,
            dbus_signature(value, unit)?
        ),
    })
}

/// Renders the `<interface>...</interface>` introspection XML fragment for
/// one interface — the same fragment shape `qt-cc.jinja` bakes into its
/// generated `interface_introspect` string constant (2-space indented,
/// `<method>`/`<signal>` with `<arg type=... name=... [direction=...] />`).
/// Deliberately doesn't try to match the old generator's exact per-method
/// arg *order* (verified inconsistent even within `workrave-service.xml`
/// itself — `GetString`'s `found`/`value` args appear in the opposite order
/// from `GetInt`'s) — comparisons against it should be structural (a set of
/// (type, direction) per method/signal), not a strict sequence.
pub fn render_introspect_xml(iface: &Interface, dbus_interface: &str, unit: &Unit) -> Result<String> {
    let mut xml = String::new();
    xml.push_str(&format!("  <interface name=\"{dbus_interface}\">\n"));

    for m in &iface.methods {
        xml.push_str(&format!("    <method name=\"{}\">\n", m.rpc_name));
        for p in &m.params {
            if p.proto_field.is_empty() {
                continue;
            }
            let dir = match p.direction {
                Direction::In => "in",
                Direction::Out => "out",
                Direction::InOut => bail!(
                    "{}: parameter '{}' is `inout`, which DBus has no native representation for \
                     (only in/out args) — not supported by the DBus backend",
                    m.rpc_name,
                    p.cxx_name
                ),
            };
            let sig = dbus_signature(&p.proto_type, unit)
                .with_context(|| format!("{}: parameter '{}'", m.rpc_name, p.cxx_name))?;
            xml.push_str(&format!(
                "      <arg type=\"{sig}\" name=\"{}\" direction=\"{dir}\" />\n",
                p.proto_field
            ));
        }
        if let Some(rv) = &m.return_value {
            let sig = dbus_signature(&rv.proto_type, unit)
                .with_context(|| format!("{}: return type", m.rpc_name))?;
            xml.push_str(&format!(
                "      <arg type=\"{sig}\" name=\"{}\" direction=\"out\" />\n",
                rv.proto_field
            ));
        }
        xml.push_str("    </method>\n");
    }

    for s in &iface.signals {
        xml.push_str(&format!("    <signal name=\"{}\">\n", s.rpc_name));
        for f in &s.fields {
            let sig = dbus_signature(&f.proto_type, unit)
                .with_context(|| format!("{}: field '{}'", s.rpc_name, f.proto_field))?;
            xml.push_str(&format!("      <arg type=\"{sig}\" name=\"{}\" />\n", f.proto_field));
        }
        xml.push_str("    </signal>\n");
    }

    xml.push_str("  </interface>\n");
    Ok(xml)
}

/// `"org.workrave.CoreInterface"` -> `"org_workrave_CoreInterface"` — the
/// same dots-to-underscores C++ identifier scheme `dbusgen.py` uses, so
/// generated class names read the same way as the existing ones.
fn dbus_cxx_ident(interface: &str) -> String {
    interface.replace('.', "_")
}

/// Collects every "custom" type (enum, struct, `Flags<Enum>` bitmask,
/// chrono duration, or `std::vector<V>` sequence) referenced anywhere in
/// `iface`, recursively — each needs its own generated `DBusMarshall<T>`
/// specialization, since only scalars/`std::string`/`int64_t`/`uint64_t`/
/// `std::list<V>`/`std::map<K,V>` are covered generically by the existing
/// `libs/dbus` runtime header. Returned as (cxx_type_spelling, TypeShape) in
/// first-encounter order, deduplicated by cxx_type_spelling.
#[derive(Debug, Clone)]
enum CustomType {
    Enum { proto_name: String },
    Struct { proto_name: String },
    /// `std::vector<V>` specifically — `std::list<V>` is already covered by
    /// the runtime header, but `std::vector` isn't.
    Vector { element_cxx_type: String },
    Duration,
    Bitmask { enum_cxx_type: String },
}

fn collect_custom_types(iface: &Interface, unit: &Unit, out: &mut Vec<(String, CustomType)>, seen: &mut HashSet<String>) -> Result<()> {
    fn visit(
        cxx_type: &str,
        proto_type: &ProtoType,
        kind: &ParamKind,
        unit: &Unit,
        out: &mut Vec<(String, CustomType)>,
        seen: &mut HashSet<String>,
    ) -> Result<()> {
        match kind {
            ParamKind::Duration => {
                if seen.insert(cxx_type.to_string()) {
                    out.push((cxx_type.to_string(), CustomType::Duration));
                }
            }
            ParamKind::Bitmask { enum_cxx_type } => {
                // The bitmask's own element enum also needs its own
                // specialization (Flags<Enum> is marshalled as a sequence
                // of the enum's string names).
                if let ProtoType::Repeated(inner) = proto_type {
                    if let ProtoType::Enum(enum_proto_name) = inner.as_ref() {
                        if seen.insert(enum_cxx_type.clone()) {
                            out.push((
                                enum_cxx_type.clone(),
                                CustomType::Enum {
                                    proto_name: enum_proto_name.clone(),
                                },
                            ));
                        }
                    }
                }
                if seen.insert(cxx_type.to_string()) {
                    out.push((
                        cxx_type.to_string(),
                        CustomType::Bitmask {
                            enum_cxx_type: enum_cxx_type.clone(),
                        },
                    ));
                }
            }
            ParamKind::Message { struct_proto_name } => {
                if seen.insert(cxx_type.to_string()) {
                    let struct_def = unit
                        .find_struct_by_proto_name(struct_proto_name)
                        .with_context(|| format!("struct '{struct_proto_name}' not registered"))?;
                    for f in &struct_def.fields {
                        visit(&f.cxx_type.spelling, &f.proto_type, &f.kind, unit, out, seen)?;
                    }
                    out.push((
                        cxx_type.to_string(),
                        CustomType::Struct {
                            proto_name: struct_proto_name.clone(),
                        },
                    ));
                }
            }
            ParamKind::Sequence(elem) => {
                visit(&elem.cxx_type, &elem.proto_type, &elem.kind, unit, out, seen)?;
                if cxx_type.starts_with("std::vector<") && seen.insert(cxx_type.to_string()) {
                    out.push((
                        cxx_type.to_string(),
                        CustomType::Vector {
                            element_cxx_type: elem.cxx_type.clone(),
                        },
                    ));
                }
            }
            ParamKind::Map { key: _, value } => {
                visit(&value.cxx_type, &value.proto_type, &value.kind, unit, out, seen)?;
            }
            ParamKind::Value => {
                if let ProtoType::Enum(proto_name) = proto_type {
                    if seen.insert(cxx_type.to_string()) {
                        out.push((
                            cxx_type.to_string(),
                            CustomType::Enum {
                                proto_name: proto_name.clone(),
                            },
                        ));
                    }
                }
            }
            ParamKind::CString | ParamKind::Bytes { .. } => {}
        }
        Ok(())
    }

    for m in &iface.methods {
        for p in &m.params {
            if p.proto_field.is_empty() {
                continue;
            }
            visit(&p.cxx_type.spelling, &p.proto_type, &p.kind, unit, out, seen)?;
        }
        if let Some(rv) = &m.return_value {
            visit(&rv.cxx_type.spelling, &rv.proto_type, &rv.kind, unit, out, seen)?;
        }
    }
    for s in &iface.signals {
        for f in &s.fields {
            visit(&f.cxx_type.spelling, &f.proto_type, &f.kind, unit, out, seen)?;
        }
    }
    Ok(())
}

fn render_enum_marshall(cxx_type: &str, enum_def: &crate::ir::EnumDef) -> Result<String> {
    let mut convert_from_variant = String::new();
    let mut marshall_to_arg = String::new();
    let mut convert_to_variant = String::new();

    for v in &enum_def.values {
        let name = v.canonical_name.as_deref().with_context(|| {
            format!(
                "enum '{}' value '{}' has no @rpc.enum.value(name=\"...\") canonical name — \
                 required for DBus generation, since DBus wire-encodes enums as strings and this \
                 tool won't silently invent a name that might not match an existing wire contract",
                enum_def.cxx_symbol, v.cxx_symbol
            )
        })?;
        convert_from_variant.push_str(&format!(
            "    if (\"{name}\" == arg) {{ value = {}; }}\n",
            v.cxx_symbol
        ));
        marshall_to_arg.push_str(&format!(
            "      case {}: str = \"{name}\"; break;\n",
            v.cxx_symbol
        ));
        convert_to_variant.push_str(&format!(
            "      case {}: str = \"{name}\"; break;\n",
            v.cxx_symbol
        ));
    }

    Ok(format!(
        r#"namespace workrave::dbus
{{
template<>
struct DBusMarshall<{cxx_type}>
{{
  static {cxx_type} convert(const QVariant &variant)
  {{
    QString arg = variant.value<QString>();
    {cxx_type} value{{}};
{convert_from_variant}    return value;
  }}
  static void marshall(QDBusArgument &arg, const {cxx_type} &value)
  {{
    std::string str;
    switch (value)
      {{
{marshall_to_arg}      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }}
    arg << QString::fromStdString(str);
  }}
  static QVariant convert(const {cxx_type} &value)
  {{
    std::string str;
    switch (value)
      {{
{convert_to_variant}      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }}
    return QVariant::fromValue(QString::fromStdString(str));
  }}
}};
}} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via {cxx_type}'s
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<{cxx_type}>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const {cxx_type} &data)
{{
  workrave::dbus::DBusMarshall<{cxx_type}>::marshall(arg, data);
  return arg;
}}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, {cxx_type} &data)
{{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<{cxx_type}>::convert(QVariant::fromValue(value));
  return arg;
}}
"#
    ))
}

fn render_struct_marshall(cxx_type: &str, struct_def: &StructDef) -> String {
    let mut convert_fields = String::new();
    let mut marshall_fields = String::new();
    for f in &struct_def.fields {
        convert_fields.push_str(&format!(
            "    result.{} = DBusMarshall<{}>::convert(arg.asVariant());\n",
            f.cxx_name, f.cxx_type.spelling
        ));
        marshall_fields.push_str(&format!(
            "    DBusMarshall<{}>::marshall(arg, value.{});\n",
            f.cxx_type.spelling, f.cxx_name
        ));
    }

    format!(
        r#"namespace workrave::dbus
{{
template<>
struct DBusMarshall<{cxx_type}>
{{
  static {cxx_type} convert(const QVariant &variant)
  {{
    const auto arg = variant.value<QDBusArgument>();
    {cxx_type} result{{}};
    arg.beginStructure();
{convert_fields}    arg.endStructure();
    return result;
  }}
  static void marshall(QDBusArgument &arg, const {cxx_type} &value)
  {{
    arg.beginStructure();
{marshall_fields}    arg.endStructure();
  }}
  static QVariant convert(const {cxx_type} &value)
  {{
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }}
}};
}} // namespace workrave::dbus

// See the enum case above for why these are at global scope, not nested in
// workrave::dbus: ADL needs to find them from {cxx_type}'s own associated
// namespace, not this library's.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const {cxx_type} &data)
{{
  workrave::dbus::DBusMarshall<{cxx_type}>::marshall(arg, data);
  return arg;
}}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, {cxx_type} &data)
{{
  data = workrave::dbus::DBusMarshall<{cxx_type}>::convert(QVariant::fromValue(arg));
  return arg;
}}
"#
    )
}

fn render_vector_marshall(cxx_type: &str, element_cxx_type: &str) -> String {
    format!(
        r#"namespace workrave::dbus
{{
template<>
struct DBusMarshall<{cxx_type}>
{{
  static {cxx_type} convert(const QVariant &variant)
  {{
    const auto arg = variant.value<QDBusArgument>();
    {cxx_type} result;
    arg.beginArray();
    while (!arg.atEnd())
      {{
        result.push_back(DBusMarshall<{element_cxx_type}>::convert(arg.asVariant()));
      }}
    arg.endArray();
    return result;
  }}
  static void marshall(QDBusArgument &arg, const {cxx_type} &value)
  {{
    arg.beginArray(qMetaTypeId<QVariant>());
    for (const auto &item : value)
      {{
        DBusMarshall<{element_cxx_type}>::marshall(arg, item);
      }}
    arg.endArray();
  }}
  static QVariant convert(const {cxx_type} &value)
  {{
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }}
}};
}} // namespace workrave::dbus
"#
    )
}

fn render_duration_marshall(cxx_type: &str) -> String {
    format!(
        r#"namespace workrave::dbus
{{
template<>
struct DBusMarshall<{cxx_type}>
{{
  static {cxx_type} convert(const QVariant &variant)
  {{
    std::string text = DBusMarshall<std::string>::convert(variant);
    return std::chrono::duration_cast<{cxx_type}>(rpc::parse_duration(text));
  }}
}};
}} // namespace workrave::dbus
"#
    )
}

fn render_bitmask_marshall(cxx_type: &str, enum_cxx_type: &str) -> String {
    format!(
        r#"namespace workrave::dbus
{{
template<>
struct DBusMarshall<{cxx_type}>
{{
  static {cxx_type} convert(const QVariant &variant)
  {{
    std::list<{enum_cxx_type}> items = DBusMarshall<std::list<{enum_cxx_type}>>::convert(variant);
    {cxx_type} result;
    for (const auto &item : items)
      {{
        result |= item;
      }}
    return result;
  }}
  static void marshall(QDBusArgument &arg, const {cxx_type} &value)
  {{
    std::list<{enum_cxx_type}> items;
    for (unsigned int b = 0; b < workrave::utils::enum_traits<{enum_cxx_type}>::bits; ++b)
      {{
        auto e = static_cast<{enum_cxx_type}>(1 << b);
        if (value.is_set(e))
          {{
            items.push_back(e);
          }}
      }}
    DBusMarshall<std::list<{enum_cxx_type}>>::marshall(arg, items);
  }}
  static QVariant convert(const {cxx_type} &value)
  {{
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }}
}};
}} // namespace workrave::dbus
"#
    )
}

fn render_method_body(m: &Method, dbus_interface: &str, cxx_impl_type: &str) -> Vec<String> {
    let num_in = m
        .params
        .iter()
        .filter(|p| !p.proto_field.is_empty() && matches!(p.direction, Direction::In))
        .count();

    let mut lines = Vec::new();
    lines.push(format!("auto *dbus_object = static_cast<{cxx_impl_type} *>(object);"));
    lines.push(String::new());

    // Declare a local for every real parameter (in and out both need one —
    // "in" so DBusMarshall::convert has somewhere to assign into before the
    // call, "out" so the real method has an lvalue to write through) and
    // for the return value, if any (the call's result is captured into it
    // before being marshalled into the reply).
    for p in &m.params {
        if p.proto_field.is_empty() {
            continue;
        }
        lines.push(format!("{} p_{}{{}};", p.cxx_type.base_spelling, p.cxx_name));
    }
    if let Some(rv) = &m.return_value {
        lines.push(format!("{} p_{}{{}};", rv.cxx_type.base_spelling, rv.proto_field));
    }
    lines.push(String::new());

    lines.push("auto num_in_args = message.arguments().size();".to_string());
    lines.push(format!("if (num_in_args != {num_in})"));
    lines.push("  {".to_string());
    lines.push("    throw workrave::dbus::DBusRemoteException()".to_string());
    lines.push("      << workrave::dbus::message_info(\"Incorrect number of in-parameters\")".to_string());
    lines.push("      << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)".to_string());
    lines.push(format!(
        "      << workrave::dbus::method_info(\"{}\")",
        m.rpc_name
    ));
    lines.push(format!(
        "      << workrave::dbus::interface_info(\"{dbus_interface}\");"
    ));
    lines.push("  }".to_string());
    lines.push(String::new());

    let mut arg_index = 0usize;
    for p in &m.params {
        if p.proto_field.is_empty() || !matches!(p.direction, Direction::In) {
            continue;
        }
        lines.push(format!(
            "p_{} = DBusMarshall<{}>::convert(message.arguments().at({arg_index}));",
            p.cxx_name, p.cxx_type.base_spelling
        ));
        arg_index += 1;
    }
    lines.push(String::new());

    let call_args: Vec<String> = m
        .params
        .iter()
        .map(|p| {
            if p.proto_field.is_empty() {
                // Absorbed size param — not expected in real @rpc.dbus
                // interfaces today (cstring/bytes aren't DBus-mapped), but
                // keep the shape consistent if it ever occurs.
                format!("p_{}", p.cxx_name)
            } else if p.cxx_type.is_pointer {
                format!("&p_{}", p.cxx_name)
            } else {
                format!("p_{}", p.cxx_name)
            }
        })
        .collect();
    let call = format!("dbus_object->{}({})", m.cxx_symbol, call_args.join(", "));

    if let Some(rv) = &m.return_value {
        lines.push(format!("p_{} = {call};", rv.proto_field));
    } else {
        lines.push(format!("{call};"));
    }
    lines.push(String::new());

    lines.push("QDBusMessage reply = message.createReply();".to_string());
    if let Some(rv) = &m.return_value {
        lines.push(format!(
            "reply << DBusMarshall<{}>::convert(p_{});",
            rv.cxx_type.base_spelling, rv.proto_field
        ));
    }
    for p in &m.params {
        if p.proto_field.is_empty() || !matches!(p.direction, Direction::Out) {
            continue;
        }
        lines.push(format!(
            "reply << DBusMarshall<{}>::convert(p_{});",
            p.cxx_type.base_spelling, p.cxx_name
        ));
    }
    lines.push(String::new());

    lines.push("bool rc = connection.send(reply);".to_string());
    lines.push("if (!rc)".to_string());
    lines.push("  {".to_string());
    lines.push("    throw workrave::dbus::DBusRemoteException()".to_string());
    lines.push("      << workrave::dbus::message_info(\"Failed to send reply\")".to_string());
    lines.push("      << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)".to_string());
    lines.push(format!(
        "      << workrave::dbus::method_info(\"{}\")",
        m.rpc_name
    ));
    lines.push(format!(
        "      << workrave::dbus::interface_info(\"{dbus_interface}\");"
    ));
    lines.push("  }".to_string());

    lines
}

pub fn render_dbus_binding(
    iface: &Interface,
    unit: &Unit,
    header_include: &str,
    dbus_header_filename: &str,
) -> Result<RenderedDBusBinding> {
    let dbus_interface = iface
        .dbus_interface
        .as_deref()
        .context("interface has no @rpc.dbus(interface=\"...\") tag")?;
    let ident = dbus_cxx_ident(dbus_interface);
    let stub_name = format!("{ident}_Stub");
    let cxx_impl_type = iface.cxx_qualified_class();

    // Header: pure-virtual signal-emit interface, mirroring dbusgen.py's
    // I<Interface> classes.
    let mut header = String::new();
    header.push_str("// GENERATED FILE - DO NOT EDIT.\n");
    header.push_str("// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.\n");
    header.push_str("#pragma once\n\n");
    header.push_str("#include <memory>\n#include <string>\n\n");
    header.push_str("#include \"dbus/IDBus.hh\"\n\n");
    header.push_str(&format!("#include \"{header_include}\"\n\n"));
    header.push_str(&format!("class {ident}\n{{\npublic:\n"));
    header.push_str(&format!("  virtual ~{ident}() = default;\n"));
    header.push_str(&format!(
        "  static {ident} *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);\n"
    ));
    for s in &iface.signals {
        let params: String = s
            .fields
            .iter()
            .map(|f| format!(", {} {}", f.cxx_type.spelling, f.cxx_name))
            .collect();
        header.push_str(&format!(
            "  virtual void {}(const std::string &path{params}) = 0;\n",
            s.rpc_name
        ));
    }
    header.push_str("};\n");

    // Source.
    let mut custom_types = Vec::new();
    let mut seen = HashSet::new();
    collect_custom_types(iface, unit, &mut custom_types, &mut seen)?;

    let mut source = String::new();
    source.push_str("// GENERATED FILE - DO NOT EDIT.\n");
    source.push_str("// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.\n");
    source.push_str("#ifdef HAVE_CONFIG_H\n#  include \"config.h\"\n#endif\n\n");
    source.push_str("#include <array>\n#include <chrono>\n#include <list>\n#include <map>\n#include <string>\n#include <string_view>\n#include <vector>\n\n");
    source.push_str("#include \"dbus/DBusBindingQt.hh\"\n#include \"dbus/DBusException.hh\"\n");
    if custom_types.iter().any(|(_, t)| matches!(t, CustomType::Duration)) {
        source.push_str("#include \"rpc/Duration.hh\"\n");
    }
    source.push_str(&format!("#include \"{dbus_header_filename}\"\n\n"));
    source.push_str("using namespace workrave::dbus;\n\n");

    // Each type's render_*_marshall function wraps its own DBusMarshall<T>
    // specialization in `namespace workrave::dbus { ... }` and emits any
    // free operator<</>> at global scope itself — see the ADL note in
    // render_enum_marshall. No shared outer namespace block here.
    for (cxx_type, shape) in &custom_types {
        match shape {
            CustomType::Enum { proto_name } => {
                let enum_def = unit
                    .enums
                    .iter()
                    .find(|e| &e.proto_name == proto_name)
                    .with_context(|| format!("enum '{proto_name}' not registered"))?;
                source.push_str(&render_enum_marshall(cxx_type, enum_def)?);
            }
            CustomType::Struct { proto_name } => {
                let struct_def = unit
                    .find_struct_by_proto_name(proto_name)
                    .with_context(|| format!("struct '{proto_name}' not registered"))?;
                source.push_str(&render_struct_marshall(cxx_type, struct_def));
            }
            CustomType::Vector { element_cxx_type } => {
                source.push_str(&render_vector_marshall(cxx_type, element_cxx_type));
            }
            CustomType::Duration => {
                source.push_str(&render_duration_marshall(cxx_type));
            }
            CustomType::Bitmask { enum_cxx_type } => {
                source.push_str(&render_bitmask_marshall(cxx_type, enum_cxx_type));
            }
        }
        source.push('\n');
    }

    let introspect_xml = render_introspect_xml(iface, dbus_interface, unit)?;

    source.push_str(&format!(
        "class {stub_name} : public DBusBindingQt, public {ident}\n{{\nprivate:\n"
    ));
    source.push_str(&format!(
        "  using DBusMethodPointer = void ({stub_name}::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);\n\n"
    ));
    source.push_str("  struct DBusMethod\n  {\n    std::string_view name;\n    DBusMethodPointer fn;\n  };\n\n");
    source.push_str("  bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection) override;\n\n");
    source.push_str("  std::string_view get_interface_introspect() override\n  {\n    return interface_introspect;\n  }\n\n");
    source.push_str("public:\n");
    source.push_str(&format!(
        "  explicit {stub_name}(std::shared_ptr<IDBus> dbus) : DBusBindingQt(std::move(dbus)) {{}}\n"
    ));
    source.push_str(&format!("  ~{stub_name}() override = default;\n\n"));
    for s in &iface.signals {
        let params: String = s
            .fields
            .iter()
            .map(|f| format!(", {} {}", f.cxx_type.spelling, f.cxx_name))
            .collect();
        source.push_str(&format!(
            "  void {}(const std::string &path{params}) override;\n",
            s.rpc_name
        ));
    }
    source.push_str("\nprivate:\n");
    for m in &iface.methods {
        source.push_str(&format!(
            "  void {}(void *object, const QDBusMessage &message, const QDBusConnection &connection);\n",
            m.rpc_name
        ));
    }
    source.push('\n');
    source.push_str(&format!(
        "  static constexpr std::array<DBusMethod, {}> method_table =\n  {{ {{\n",
        iface.methods.len()
    ));
    for m in &iface.methods {
        source.push_str(&format!(
            "      {{.name = \"{}\", .fn = &{stub_name}::{}}},\n",
            m.rpc_name, m.rpc_name
        ));
    }
    source.push_str("  } };\n\n");
    source.push_str("  static constexpr std::string_view interface_introspect =\n");
    for line in introspect_xml.lines() {
        source.push_str(&format!("  \"{}\\n\"\n", escape_cxx_string(line)));
    }
    source.push_str(";\n");
    source.push_str("};\n\n");

    source.push_str(&format!(
        "{ident} *{ident}::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)\n{{\n"
    ));
    source.push_str(&format!("  {stub_name} *iface = nullptr;\n"));
    source.push_str(&format!(
        "  DBusBinding *binding = dbus->find_binding(\"{dbus_interface}\");\n"
    ));
    source.push_str("  if (binding != nullptr)\n    {\n");
    source.push_str(&format!("      iface = dynamic_cast<{stub_name} *>(binding);\n"));
    source.push_str("    }\n  return iface;\n}\n\n");

    source.push_str(&format!(
        "bool\n{stub_name}::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)\n{{\n"
    ));
    source.push_str("  std::string method_name = message.member().toStdString();\n");
    source.push_str(&format!(
        "  constexpr std::array<DBusMethod, {}> table = method_table;\n",
        iface.methods.len()
    ));
    source.push_str("  for (const auto &method : table)\n    {\n");
    source.push_str("      if (method_name == method.name)\n        {\n");
    source.push_str("          DBusMethodPointer ptr = method.fn;\n");
    source.push_str("          if (ptr != nullptr)\n            {\n");
    source.push_str("              (this->*ptr)(object, message, connection);\n");
    source.push_str("            }\n          return true;\n        }\n    }\n");
    source.push_str("  throw DBusRemoteException()\n");
    source.push_str("    << message_info(\"Unknown method\")\n");
    source.push_str("    << error_code_info(DBUS_ERROR_UNKNOWN_METHOD)\n");
    source.push_str("    << method_info(method_name)\n");
    source.push_str(&format!("    << interface_info(\"{dbus_interface}\");\n}}\n\n"));

    for m in &iface.methods {
        let body = render_method_body(m, dbus_interface, &cxx_impl_type);
        source.push_str(&format!(
            "void\n{stub_name}::{}(void *object, const QDBusMessage &message, const QDBusConnection &connection)\n{{\n  try\n    {{\n",
            m.rpc_name
        ));
        for line in &body {
            if line.is_empty() {
                source.push('\n');
            } else {
                source.push_str("      ");
                source.push_str(line);
                source.push('\n');
            }
        }
        source.push_str("    }\n");
        source.push_str("  catch (const DBusRemoteException &e)\n    {\n");
        source.push_str(&format!(
            "      e << method_info(\"{}\") << interface_info(\"{dbus_interface}\");\n",
            m.rpc_name
        ));
        source.push_str("      throw;\n    }\n}\n\n");
    }

    for s in &iface.signals {
        source.push_str(&render_signal_emit(s, &stub_name, dbus_interface));
    }

    source.push_str(&format!(
        "void init_{}(std::shared_ptr<IDBus> dbus)\n{{\n",
        ident
    ));
    source.push_str(&format!(
        "  dbus->register_binding(\"{dbus_interface}\", new {stub_name}(dbus));\n\n"
    ));
    for (cxx_type, shape) in &custom_types {
        if matches!(shape, CustomType::Enum { .. }) {
            source.push_str(&format!("  qDBusRegisterMetaType<{cxx_type}>();\n"));
        }
    }
    source.push_str("}\n");

    Ok(RenderedDBusBinding { header, source })
}

fn render_signal_emit(s: &Signal, stub_name: &str, dbus_interface: &str) -> String {
    let params: String = s
        .fields
        .iter()
        .map(|f| format!(", {} {}", f.cxx_type.spelling, f.cxx_name))
        .collect();
    let mut out = format!(
        "void\n{stub_name}::{}(const std::string &path{params})\n{{\n",
        s.rpc_name
    );
    out.push_str(&format!(
        "  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), \"{dbus_interface}\", \"{}\");\n",
        s.rpc_name
    ));
    for f in &s.fields {
        out.push_str(&format!(
            "  sig << DBusMarshall<{}>::convert({});\n",
            f.cxx_type.spelling, f.cxx_name
        ));
    }
    out.push_str("  IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<IDBusPrivateQt>(dbus);\n");
    out.push_str("  priv->get_connection().send(sig);\n}\n\n");
    out
}

fn escape_cxx_string(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"")
}
