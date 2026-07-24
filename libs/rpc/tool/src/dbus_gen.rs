//! Renders a DBus binding (introspection XML + QtDBus dispatch/marshalling
//! C++) for an `@rpc.dbus(interface="...")`-tagged interface, from the same
//! IR the gRPC backend (`cpp_gen.rs`/`proto_gen.rs`) uses — this is the
//! payoff of keeping that IR free of gRPC-specific concepts from the start.
//!
//! Follows `cpp_gen.rs`'s split: this module only computes plain-data "view"
//! structs from the IR (support/data-prep logic — type mapping, custom-type
//! discovery, parameter classification), and askama templates
//! (`src/templates/dbus_*.askama`) own the actual generated-code structure
//! and boilerplate. Nothing here hand-builds C++/XML text via `format!`.
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
use askama::Template;

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

struct XmlMethodArgView {
    sig: String,
    name: String,
    direction: &'static str,
}

struct XmlSignalArgView {
    sig: String,
    name: String,
}

struct XmlMethodView {
    name: String,
    args: Vec<XmlMethodArgView>,
}

struct XmlSignalView {
    name: String,
    args: Vec<XmlSignalArgView>,
}

struct IntrospectView {
    dbus_interface: String,
    methods: Vec<XmlMethodView>,
    signals: Vec<XmlSignalView>,
}

#[derive(Template)]
#[template(path = "dbus_introspect.xml.askama", escape = "none")]
struct IntrospectTemplate<'a> {
    view: &'a IntrospectView,
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
    let mut methods = Vec::new();
    for m in &iface.methods {
        let mut args = Vec::new();
        for p in &m.params {
            if p.proto_field.is_empty() {
                continue;
            }
            let direction = match p.direction {
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
            args.push(XmlMethodArgView {
                sig,
                name: p.proto_field.clone(),
                direction,
            });
        }
        if let Some(rv) = &m.return_value {
            let sig = dbus_signature(&rv.proto_type, unit).with_context(|| format!("{}: return type", m.rpc_name))?;
            args.push(XmlMethodArgView {
                sig,
                name: rv.proto_field.clone(),
                direction: "out",
            });
        }
        methods.push(XmlMethodView {
            name: m.rpc_name.clone(),
            args,
        });
    }

    let mut signals = Vec::new();
    for s in &iface.signals {
        let mut args = Vec::new();
        for f in &s.fields {
            let sig = dbus_signature(&f.proto_type, unit).with_context(|| format!("{}: field '{}'", s.rpc_name, f.proto_field))?;
            args.push(XmlSignalArgView {
                sig,
                name: f.proto_field.clone(),
            });
        }
        signals.push(XmlSignalView {
            name: s.rpc_name.clone(),
            args,
        });
    }

    let view = IntrospectView {
        dbus_interface: dbus_interface.to_string(),
        methods,
        signals,
    };
    Ok(IntrospectTemplate { view: &view }.render()?)
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

struct EnumValueView {
    cxx_symbol: String,
    name: String,
}

struct EnumMarshallView<'a> {
    cxx_type: &'a str,
    values: Vec<EnumValueView>,
}

#[derive(Template)]
#[template(path = "dbus_marshall_enum.cc.askama", escape = "none")]
struct EnumMarshallTemplate<'a> {
    view: &'a EnumMarshallView<'a>,
}

fn render_enum_marshall(cxx_type: &str, enum_def: &crate::ir::EnumDef) -> Result<String> {
    let mut values = Vec::new();
    for v in &enum_def.values {
        let name = v.canonical_name.as_deref().with_context(|| {
            format!(
                "enum '{}' value '{}' has no @rpc.enum.value(name=\"...\") canonical name — \
                 required for DBus generation, since DBus wire-encodes enums as strings and this \
                 tool won't silently invent a name that might not match an existing wire contract",
                enum_def.cxx_symbol, v.cxx_symbol
            )
        })?;
        values.push(EnumValueView {
            cxx_symbol: v.cxx_symbol.clone(),
            name: name.to_string(),
        });
    }
    let view = EnumMarshallView { cxx_type, values };
    Ok(EnumMarshallTemplate { view: &view }.render()?)
}

struct StructFieldView {
    cxx_name: String,
    cxx_type: String,
}

struct StructMarshallView<'a> {
    cxx_type: &'a str,
    fields: Vec<StructFieldView>,
}

#[derive(Template)]
#[template(path = "dbus_marshall_struct.cc.askama", escape = "none")]
struct StructMarshallTemplate<'a> {
    view: &'a StructMarshallView<'a>,
}

fn render_struct_marshall(cxx_type: &str, struct_def: &StructDef) -> Result<String> {
    let fields = struct_def
        .fields
        .iter()
        .map(|f| StructFieldView {
            cxx_name: f.cxx_name.clone(),
            cxx_type: f.cxx_type.spelling.clone(),
        })
        .collect();
    let view = StructMarshallView { cxx_type, fields };
    Ok(StructMarshallTemplate { view: &view }.render()?)
}

struct VectorMarshallView<'a> {
    cxx_type: &'a str,
    element_cxx_type: &'a str,
}

#[derive(Template)]
#[template(path = "dbus_marshall_vector.cc.askama", escape = "none")]
struct VectorMarshallTemplate<'a> {
    view: &'a VectorMarshallView<'a>,
}

fn render_vector_marshall(cxx_type: &str, element_cxx_type: &str) -> Result<String> {
    let view = VectorMarshallView { cxx_type, element_cxx_type };
    Ok(VectorMarshallTemplate { view: &view }.render()?)
}

struct DurationMarshallView<'a> {
    cxx_type: &'a str,
}

#[derive(Template)]
#[template(path = "dbus_marshall_duration.cc.askama", escape = "none")]
struct DurationMarshallTemplate<'a> {
    view: &'a DurationMarshallView<'a>,
}

fn render_duration_marshall(cxx_type: &str) -> Result<String> {
    let view = DurationMarshallView { cxx_type };
    Ok(DurationMarshallTemplate { view: &view }.render()?)
}

struct BitmaskMarshallView<'a> {
    cxx_type: &'a str,
    enum_cxx_type: &'a str,
}

#[derive(Template)]
#[template(path = "dbus_marshall_bitmask.cc.askama", escape = "none")]
struct BitmaskMarshallTemplate<'a> {
    view: &'a BitmaskMarshallView<'a>,
}

fn render_bitmask_marshall(cxx_type: &str, enum_cxx_type: &str) -> Result<String> {
    let view = BitmaskMarshallView { cxx_type, enum_cxx_type };
    Ok(BitmaskMarshallTemplate { view: &view }.render()?)
}

struct DBusParamView {
    cxx_name: String,
    base_spelling: String,
    is_out: bool,
    /// Position among the method's `in` (or `inout`, once supported)
    /// arguments in `message.arguments()` — `None` for an `out`-only param,
    /// which has no wire representation to read from.
    arg_index: Option<usize>,
}

struct DBusMethodView {
    rpc_name: String,
    cxx_symbol: String,
    num_in: usize,
    params: Vec<DBusParamView>,
    /// Pre-joined real-call argument list, e.g. "p_key, &p_out" — a
    /// mechanical `&`-if-pointer/`p_`-prefix transform, not a structural
    /// decision, so joining it here (rather than in the template) is just
    /// data prep.
    call_args: String,
    has_return_value: bool,
    return_base_spelling: String,
    return_proto_field: String,
}

fn method_view(m: &Method) -> DBusMethodView {
    let mut params = Vec::new();
    let mut call_args = Vec::new();
    let mut arg_index = 0usize;
    let mut num_in = 0usize;

    for p in &m.params {
        if p.proto_field.is_empty() {
            // Absorbed size param — not expected in real @rpc.dbus
            // interfaces today (cstring/bytes aren't DBus-mapped), but
            // keep the shape consistent if it ever occurs.
            call_args.push(format!("p_{}", p.cxx_name));
            continue;
        }

        let is_in = matches!(p.direction, Direction::In);
        let is_out = matches!(p.direction, Direction::Out);
        let idx = if is_in {
            let i = arg_index;
            arg_index += 1;
            num_in += 1;
            Some(i)
        } else {
            None
        };

        params.push(DBusParamView {
            cxx_name: p.cxx_name.clone(),
            base_spelling: p.cxx_type.base_spelling.clone(),
            is_out,
            arg_index: idx,
        });
        call_args.push(if p.cxx_type.is_pointer {
            format!("&p_{}", p.cxx_name)
        } else {
            format!("p_{}", p.cxx_name)
        });
    }

    let (has_return_value, return_base_spelling, return_proto_field) = match &m.return_value {
        Some(rv) => (true, rv.cxx_type.base_spelling.clone(), rv.proto_field.clone()),
        None => (false, String::new(), String::new()),
    };

    DBusMethodView {
        rpc_name: m.rpc_name.clone(),
        cxx_symbol: m.cxx_symbol.clone(),
        num_in,
        params,
        call_args: call_args.join(", "),
        has_return_value,
        return_base_spelling,
        return_proto_field,
    }
}

struct DBusSignalFieldView {
    cxx_type: String,
    cxx_name: String,
}

struct DBusSignalView {
    rpc_name: String,
    /// Pre-joined trailing parameter list, e.g. ", workrave::OperationMode
    /// value" — same mechanical-join rationale as DBusMethodView::call_args.
    params: String,
    fields: Vec<DBusSignalFieldView>,
}

fn signal_view(s: &Signal) -> DBusSignalView {
    let params = s
        .fields
        .iter()
        .map(|f| format!(", {} {}", f.cxx_type.spelling, f.cxx_name))
        .collect::<String>();
    let fields = s
        .fields
        .iter()
        .map(|f| DBusSignalFieldView {
            cxx_type: f.cxx_type.spelling.clone(),
            cxx_name: f.cxx_name.clone(),
        })
        .collect();
    DBusSignalView { rpc_name: s.rpc_name.clone(), params, fields }
}

struct DBusBindingView {
    ident: String,
    stub_name: String,
    dbus_interface: String,
    header_include: String,
    dbus_header_filename: String,
    cxx_impl_type: String,
    has_duration: bool,
    /// Each already-rendered via its own dbus_marshall_*.cc.askama template
    /// (self-contained `namespace workrave::dbus {...}` blocks with any free
    /// operators at global scope — see the ADL note on render_enum_marshall
    /// in the template itself) — spliced in verbatim, same as how
    /// HeaderTemplate/SourceTemplate compose in cpp_gen.rs.
    marshall_blocks: Vec<String>,
    /// Already C++-string-literal-escaped, one XML line each — the template
    /// only needs to wrap each in `"...\n"`.
    introspect_xml_lines: Vec<String>,
    methods: Vec<DBusMethodView>,
    signals: Vec<DBusSignalView>,
    enum_metatype_registrations: Vec<String>,
}

#[derive(Template)]
#[template(path = "dbus_binding.hh.askama", escape = "none")]
struct DBusHeaderTemplate<'a> {
    view: &'a DBusBindingView,
}

#[derive(Template)]
#[template(path = "dbus_binding.cc.askama", escape = "none")]
struct DBusSourceTemplate<'a> {
    view: &'a DBusBindingView,
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

    let mut custom_types = Vec::new();
    let mut seen = HashSet::new();
    collect_custom_types(iface, unit, &mut custom_types, &mut seen)?;

    let mut marshall_blocks = Vec::new();
    let mut enum_metatype_registrations = Vec::new();
    for (cxx_type, shape) in &custom_types {
        match shape {
            CustomType::Enum { proto_name } => {
                let enum_def = unit
                    .enums
                    .iter()
                    .find(|e| &e.proto_name == proto_name)
                    .with_context(|| format!("enum '{proto_name}' not registered"))?;
                marshall_blocks.push(render_enum_marshall(cxx_type, enum_def)?);
                enum_metatype_registrations.push(cxx_type.clone());
            }
            CustomType::Struct { proto_name } => {
                let struct_def = unit
                    .find_struct_by_proto_name(proto_name)
                    .with_context(|| format!("struct '{proto_name}' not registered"))?;
                marshall_blocks.push(render_struct_marshall(cxx_type, struct_def)?);
            }
            CustomType::Vector { element_cxx_type } => {
                marshall_blocks.push(render_vector_marshall(cxx_type, element_cxx_type)?);
            }
            CustomType::Duration => {
                marshall_blocks.push(render_duration_marshall(cxx_type)?);
            }
            CustomType::Bitmask { enum_cxx_type } => {
                marshall_blocks.push(render_bitmask_marshall(cxx_type, enum_cxx_type)?);
            }
        }
    }
    let has_duration = custom_types.iter().any(|(_, t)| matches!(t, CustomType::Duration));

    let introspect_xml = render_introspect_xml(iface, dbus_interface, unit)?;
    let introspect_xml_lines: Vec<String> = introspect_xml.lines().map(escape_cxx_string).collect();

    let view = DBusBindingView {
        ident,
        stub_name,
        dbus_interface: dbus_interface.to_string(),
        header_include: header_include.to_string(),
        dbus_header_filename: dbus_header_filename.to_string(),
        cxx_impl_type,
        has_duration,
        marshall_blocks,
        introspect_xml_lines,
        methods: iface.methods.iter().map(method_view).collect(),
        signals: iface.signals.iter().map(signal_view).collect(),
        enum_metatype_registrations,
    };

    let header = DBusHeaderTemplate { view: &view }.render()?;
    let source = DBusSourceTemplate { view: &view }.render()?;
    Ok(RenderedDBusBinding { header, source })
}

fn escape_cxx_string(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"")
}
