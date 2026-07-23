//! Renders the `<Service>ServiceImpl` adapter (.hh + .cc): a `grpc::Service`
//! subclass whose method bodies unpack the protobuf request, call straight
//! into the real, unmodified, annotated C++ method, and pack the response.
//!
//! The real `<Service>::Service` base class and `<Rpc>Request`/`<Rpc>Response`
//! message types come from running the `.proto` this tool also emits through
//! actual `protoc` + `grpc_cpp_plugin` — this module only emits the glue that
//! calls into real C++, it never reimplements protobuf wire marshalling.

use std::collections::HashMap;

use anyhow::Result;
use askama::Template;

use crate::ir::{Direction, Interface, Method, ParamKind, ProtoType, Signal, Unit};

struct MethodView {
    rpc_name: String,
    body_lines: Vec<String>,
}

struct SignalView {
    rpc_name: String,
    /// The real accessor to connect to, e.g. "signal_operation_mode_changed".
    cxx_symbol: String,
    /// Set for keyed interfaces: resolves `impl_` from the registry before
    /// connecting, same as MethodView's keyed prefix.
    resolve_line: Option<String>,
    /// The connected lambda's parameter list, e.g. "workrave::OperationMode value".
    lambda_params: String,
    /// Statements building the `Event` message from the lambda's arguments.
    event_set_stmts: Vec<String>,
}

struct KeyedByView {
    /// The real C++ type to resolve against, e.g. "workrave::BreakId".
    cxx_type: String,
}

struct ServiceView {
    service_name: String,
    proto_cpp_ns: String,
    impl_class_name: String,
    cxx_impl_type: String,
    header_include: String,
    proto_basename: String,
    /// The literal file name written to `--out-adapter-hh` (may differ from
    /// `impl_class_name` — CMake names outputs after its own NAME parameter,
    /// not after the C++ service name). The generated .cc's own #include
    /// must reference this, not a name derived from the service.
    adapter_header_filename: String,
    /// Set when the interface has multiple live instances (Interface::keyed_by)
    /// — changes the constructor/member from a fixed `T &impl_` to an
    /// `rpc::InstanceRegistry<Key, T> &registry_` resolved per-call from an
    /// `id` request field. The gRPC analog of DBus's per-object-path routing.
    keyed_by: Option<KeyedByView>,
    /// Whether any method has a `ParamKind::Duration` parameter — gates the
    /// `#include "rpc/Duration.hh"` line, since most services don't need it.
    has_duration: bool,
    methods: Vec<MethodView>,
    signals: Vec<SignalView>,
}

#[derive(Template)]
#[template(path = "service_impl.hh.askama", escape = "none")]
struct HeaderTemplate<'a> {
    view: &'a ServiceView,
}

#[derive(Template)]
#[template(path = "service_impl.cc.askama", escape = "none")]
struct SourceTemplate<'a> {
    view: &'a ServiceView,
}

pub struct RenderedAdapter {
    pub header: String,
    pub source: String,
}

pub fn render_adapter(
    iface: &Interface,
    unit: &Unit,
    package: &str,
    header_include: &str,
    proto_basename: &str,
    adapter_header_filename: &str,
) -> Result<RenderedAdapter> {
    let proto_cpp_ns = package.replace('.', "::");
    let view = ServiceView {
        service_name: iface.service_name.clone(),
        proto_cpp_ns: proto_cpp_ns.clone(),
        impl_class_name: format!("{}ServiceImpl", iface.service_name),
        cxx_impl_type: iface.cxx_qualified_class(),
        header_include: header_include.to_string(),
        proto_basename: proto_basename.to_string(),
        adapter_header_filename: adapter_header_filename.to_string(),
        keyed_by: iface.keyed_by.as_ref().map(|k| KeyedByView {
            cxx_type: k.cxx_type.clone(),
        }),
        has_duration: iface.methods.iter().any(|m| {
            m.params
                .iter()
                .any(|p| matches!(p.kind, ParamKind::Duration))
        }),
        methods: iface
            .methods
            .iter()
            .map(|m| method_view(m, unit, &proto_cpp_ns, iface.keyed_by.as_ref()))
            .collect(),
        signals: iface
            .signals
            .iter()
            .map(|s| signal_view(s, unit, &proto_cpp_ns, iface.keyed_by.as_ref()))
            .collect(),
    };

    let header = HeaderTemplate { view: &view }.render()?;
    let source = SourceTemplate { view: &view }.render()?;
    Ok(RenderedAdapter { header, source })
}

fn method_view(
    m: &Method,
    unit: &Unit,
    proto_cpp_ns: &str,
    keyed_by: Option<&crate::ir::KeyType>,
) -> MethodView {
    // Maps a "bytes" pointer field's paired size parameter name to the
    // primary field name it should read `.size()` from.
    let size_owner: HashMap<&str, &str> = m
        .params
        .iter()
        .filter_map(|p| match &p.kind {
            ParamKind::Bytes { size_param } => Some((size_param.as_str(), p.proto_field.as_str())),
            _ => None,
        })
        .collect();

    let mut pre_call_decls = Vec::new();
    let mut post_call_stmts = Vec::new();
    let mut call_args = Vec::new();

    for p in &m.params {
        if p.proto_field.is_empty() {
            // Absorbed size parameter of a `bytes` pair.
            let owner = size_owner
                .get(p.cxx_name.as_str())
                .expect("absorbed param must have a bytes owner");
            call_args.push(format!(
                "static_cast<{}>(request->{owner}().size())",
                p.cxx_type.spelling
            ));
            continue;
        }

        match (p.direction, &p.kind) {
            (Direction::In, ParamKind::Value) => {
                call_args.push(request_read_expr(
                    p.proto_field.as_str(),
                    &p.proto_type,
                    &p.cxx_type.base_spelling,
                ));
            }
            (Direction::In, ParamKind::CString) => {
                call_args.push(format!("request->{}().c_str()", p.proto_field));
            }
            (Direction::In, ParamKind::Bytes { .. }) => {
                call_args.push(format!(
                    "reinterpret_cast<{}>(request->{}().data())",
                    p.cxx_type.spelling, p.proto_field
                ));
            }
            (Direction::In, ParamKind::Duration) => {
                call_args.push(format!(
                    "std::chrono::duration_cast<{}>(rpc::parse_duration(request->{}()))",
                    p.cxx_type.spelling, p.proto_field
                ));
            }
            (Direction::In, ParamKind::Bitmask { enum_cxx_type }) => {
                let local = format!("local_{}", p.cxx_name);
                pre_call_decls.push(format!("{} {local};", p.cxx_type.spelling));
                pre_call_decls.push(format!(
                    "for (int i = 0; i < request->{field}_size(); ++i) {{ {local} |= static_cast<{enum_cxx_type}>(request->{field}(i)); }}",
                    field = p.proto_field,
                ));
                call_args.push(local);
            }
            (Direction::In, ParamKind::Message { .. })
            | (Direction::In, ParamKind::Sequence(_))
            | (Direction::In, ParamKind::Map { .. }) => {
                let local = format!("local_{}", p.cxx_name);
                pre_call_decls.push(format!("{} {local}{{}};", p.cxx_type.base_spelling));
                pre_call_decls.extend(assign_from_wire(
                    &local,
                    &format!("request->{}()", p.proto_field),
                    &p.cxx_type.base_spelling,
                    &p.proto_type,
                    &p.kind,
                    unit,
                    0,
                ));
                call_args.push(if p.cxx_type.is_pointer {
                    format!("&{local}")
                } else {
                    local.clone()
                });
            }
            (Direction::Out, ParamKind::Value) | (Direction::InOut, ParamKind::Value) => {
                let local = format!("local_{}", p.cxx_name);
                if p.direction == Direction::InOut {
                    let init = request_read_expr(
                        p.proto_field.as_str(),
                        &p.proto_type,
                        &p.cxx_type.base_spelling,
                    );
                    pre_call_decls.push(format!("{} {local} = {init};", p.cxx_type.base_spelling));
                } else {
                    pre_call_decls.push(format!("{} {local}{{}};", p.cxx_type.base_spelling));
                }
                call_args.push(if p.cxx_type.is_pointer {
                    format!("&{local}")
                } else {
                    local.clone()
                });
                post_call_stmts.extend(write_wire(
                    "response->",
                    &p.proto_field,
                    &local,
                    &p.proto_type,
                    &p.kind,
                    proto_cpp_ns,
                    unit,
                    0,
                ));
            }
            (Direction::Out, ParamKind::Message { .. })
            | (Direction::InOut, ParamKind::Message { .. })
            | (Direction::Out, ParamKind::Sequence(_))
            | (Direction::InOut, ParamKind::Sequence(_))
            | (Direction::Out, ParamKind::Map { .. })
            | (Direction::InOut, ParamKind::Map { .. }) => {
                let local = format!("local_{}", p.cxx_name);
                pre_call_decls.push(format!("{} {local}{{}};", p.cxx_type.base_spelling));
                if p.direction == Direction::InOut {
                    pre_call_decls.extend(assign_from_wire(
                        &local,
                        &format!("request->{}()", p.proto_field),
                        &p.cxx_type.base_spelling,
                        &p.proto_type,
                        &p.kind,
                        unit,
                        0,
                    ));
                }
                call_args.push(if p.cxx_type.is_pointer {
                    format!("&{local}")
                } else {
                    local.clone()
                });
                post_call_stmts.extend(write_wire(
                    "response->",
                    &p.proto_field,
                    &local,
                    &p.proto_type,
                    &p.kind,
                    proto_cpp_ns,
                    unit,
                    0,
                ));
            }
            (_, ParamKind::CString)
            | (_, ParamKind::Bytes { .. })
            | (_, ParamKind::Duration)
            | (_, ParamKind::Bitmask { .. }) => {
                // Guarded against at parse time: v1 only supports
                // cstring/bytes/duration/bitmask as `in` parameters.
                unreachable!(
                    "out/inout cstring, bytes, duration or bitmask parameter should have been \
                     rejected during parsing"
                );
            }
        }
    }

    let call = format!("impl_.{}({})", m.cxx_symbol, call_args.join(", "));

    let mut body_lines = Vec::new();
    if let Some(key) = keyed_by {
        body_lines.push(format!(
            "auto &impl_ = registry_.resolve(static_cast<{}>(request->id()));",
            key.cxx_type
        ));
    }
    body_lines.extend(pre_call_decls);
    if let Some(rv) = &m.return_value {
        body_lines.push(format!("auto rpc_result = {call};"));
        body_lines.extend(write_wire(
            "response->",
            &rv.proto_field,
            "rpc_result",
            &rv.proto_type,
            &rv.kind,
            proto_cpp_ns,
            unit,
            0,
        ));
    } else {
        body_lines.push(format!("{call};"));
    }
    body_lines.extend(post_call_stmts);

    MethodView {
        rpc_name: m.rpc_name.clone(),
        body_lines,
    }
}

fn request_read_expr(field: &str, proto_type: &ProtoType, cxx_base_type: &str) -> String {
    match proto_type {
        ProtoType::Enum(_) => format!("static_cast<{cxx_base_type}>(request->{field}())"),
        _ => format!("request->{field}()"),
    }
}

/// Emits statement(s) that assign into the already-existing lvalue
/// `dest_expr` (a variable name, or a field-access expression like
/// "local.field") by reading `wire_expr` (a proto accessor call — a nested
/// message reference for `Message`, a repeated-field range for `Sequence`).
/// The recursive workhorse behind all "read from the wire into real C++"
/// marshalling: used directly for a top-level struct/sequence parameter,
/// and recursively for its fields/elements, however deeply nested.
///
/// `depth` only feeds unique loop-variable names (`rpc_item_0`, `rpc_item_1`,
/// ...) so a sequence-of-sequence doesn't shadow its own outer loop variable.
#[allow(clippy::too_many_arguments)]
fn assign_from_wire(
    dest_expr: &str,
    wire_expr: &str,
    cxx_type_spelling: &str,
    proto_type: &ProtoType,
    kind: &ParamKind,
    unit: &Unit,
    depth: usize,
) -> Vec<String> {
    match kind {
        ParamKind::Message { struct_proto_name } => {
            let struct_def = unit
                .find_struct_by_proto_name(struct_proto_name)
                .expect("Message kind always names a registered struct");
            let mut stmts = Vec::new();
            for f in &struct_def.fields {
                let field_wire = format!("{wire_expr}.{}()", f.proto_field);
                let field_dest = format!("{dest_expr}.{}", f.cxx_name);
                stmts.extend(assign_from_wire(
                    &field_dest,
                    &field_wire,
                    &f.cxx_type.spelling,
                    &f.proto_type,
                    &f.kind,
                    unit,
                    depth,
                ));
            }
            stmts
        }
        ParamKind::Sequence(elem) => {
            let wire_item = format!("rpc_wire_{depth}");
            let item = format!("rpc_item_{depth}");
            let assign_stmts = assign_from_wire(
                &item,
                &wire_item,
                &elem.cxx_type,
                &elem.proto_type,
                &elem.kind,
                unit,
                depth + 1,
            );
            vec![format!(
                "for (const auto &{wire_item} : {wire_expr}) {{ {elem_ty} {item}{{}}; {assign} {dest_expr}.push_back({item}); }}",
                elem_ty = elem.cxx_type,
                assign = assign_stmts.join(" "),
            )]
        }
        ParamKind::Map { key: _, value } => {
            // A protobuf map field's C++ getter returns a
            // `const google::protobuf::Map<K, V>&`, whose iterator value
            // type is `std::pair<const K, V>` — `.first`/`.second` work the
            // same as a real std::map, so the key never needs its own
            // marshalling (protobuf-legal keys always match their C++ type
            // exactly); only the value recurses through assign_from_wire,
            // same as a sequence element.
            let kv = format!("rpc_kv_{depth}");
            let val = format!("rpc_val_{depth}");
            let assign_stmts = assign_from_wire(
                &val,
                &format!("{kv}.second"),
                &value.cxx_type,
                &value.proto_type,
                &value.kind,
                unit,
                depth + 1,
            );
            vec![format!(
                "for (const auto &{kv} : {wire_expr}) {{ {val_ty} {val}{{}}; {assign} {dest_expr}.emplace({kv}.first, {val}); }}",
                val_ty = value.cxx_type,
                assign = assign_stmts.join(" "),
            )]
        }
        _ => match proto_type {
            ProtoType::Enum(_) => vec![format!("{dest_expr} = static_cast<{cxx_type_spelling}>({wire_expr});")],
            _ => vec![format!("{dest_expr} = {wire_expr};")],
        },
    }
}

/// Emits statement(s) that write the real C++ value `src_expr` onto the
/// proto field reached via `target_prefix` + `field` (`set_field`/
/// `mutable_field`/`add_field`, chosen by `kind`) — the write counterpart of
/// `assign_from_wire`. `target_prefix` is the C++ prefix to reach the
/// setter, e.g. "response->" for a top-level field or "event." for a signal
/// event; recursion passes a nested message pointer's `->` prefix instead.
#[allow(clippy::too_many_arguments)]
fn write_wire(
    target_prefix: &str,
    field: &str,
    src_expr: &str,
    proto_type: &ProtoType,
    kind: &ParamKind,
    proto_cpp_ns: &str,
    unit: &Unit,
    depth: usize,
) -> Vec<String> {
    match kind {
        ParamKind::Message { struct_proto_name } => {
            let struct_def = unit
                .find_struct_by_proto_name(struct_proto_name)
                .expect("Message kind always names a registered struct");
            let msg_var = format!("rpc_msg_{depth}");
            let mut stmts = vec![format!("auto *{msg_var} = {target_prefix}mutable_{field}();")];
            for f in &struct_def.fields {
                let field_src = format!("{src_expr}.{}", f.cxx_name);
                stmts.extend(write_wire(
                    &format!("{msg_var}->"),
                    &f.proto_field,
                    &field_src,
                    &f.proto_type,
                    &f.kind,
                    proto_cpp_ns,
                    unit,
                    depth + 1,
                ));
            }
            stmts
        }
        ParamKind::Sequence(elem) => {
            let item = format!("rpc_item_{depth}");
            match &*elem.kind {
                ParamKind::Message { struct_proto_name } => {
                    let struct_def = unit
                        .find_struct_by_proto_name(struct_proto_name)
                        .expect("Message kind always names a registered struct");
                    let elem_var = format!("rpc_elem_{depth}");
                    let mut field_stmts = Vec::new();
                    for f in &struct_def.fields {
                        let field_src = format!("{item}.{}", f.cxx_name);
                        field_stmts.extend(write_wire(
                            &format!("{elem_var}->"),
                            &f.proto_field,
                            &field_src,
                            &f.proto_type,
                            &f.kind,
                            proto_cpp_ns,
                            unit,
                            depth + 1,
                        ));
                    }
                    vec![format!(
                        "for (const auto &{item} : {src_expr}) {{ auto *{elem_var} = {target_prefix}add_{field}(); {stmts} }}",
                        stmts = field_stmts.join(" ")
                    )]
                }
                ParamKind::Sequence(_) => {
                    unreachable!("sequence-of-sequence is not supported")
                }
                ParamKind::Map { .. } => {
                    unreachable!("sequence-of-map is rejected at parse time (see resolve_value_type)")
                }
                _ => match &elem.proto_type {
                    ProtoType::Enum(name) => vec![format!(
                        "for (const auto &{item} : {src_expr}) {{ {target_prefix}add_{field}(static_cast<::{proto_cpp_ns}::{name}>({item})); }}"
                    )],
                    _ => vec![format!(
                        "for (const auto &{item} : {src_expr}) {{ {target_prefix}add_{field}({item}); }}"
                    )],
                },
            }
        }
        ParamKind::Map { key: _, value } => {
            // A protobuf map field's C++ mutator returns a
            // `google::protobuf::Map<K, V>*`; `(*mutable_field())[key]`
            // inserts-or-returns a reference to the value slot (never a
            // pointer, even for a message value type — unlike
            // mutable_field() elsewhere in this function).
            let kv = format!("rpc_kv_{depth}");
            match &*value.kind {
                ParamKind::Message { struct_proto_name } => {
                    let struct_def = unit
                        .find_struct_by_proto_name(struct_proto_name)
                        .expect("Message kind always names a registered struct");
                    let map_val_var = format!("rpc_map_val_{depth}");
                    let mut field_stmts = vec![format!(
                        "auto &{map_val_var} = (*{target_prefix}mutable_{field}())[{kv}.first];"
                    )];
                    for f in &struct_def.fields {
                        let field_src = format!("{kv}.second.{}", f.cxx_name);
                        field_stmts.extend(write_wire(
                            &format!("{map_val_var}."),
                            &f.proto_field,
                            &field_src,
                            &f.proto_type,
                            &f.kind,
                            proto_cpp_ns,
                            unit,
                            depth + 1,
                        ));
                    }
                    vec![format!(
                        "for (const auto &{kv} : {src_expr}) {{ {stmts} }}",
                        stmts = field_stmts.join(" ")
                    )]
                }
                ParamKind::Sequence(_) | ParamKind::Map { .. } => unreachable!(
                    "a map value can't be a sequence or another map (rejected at parse time)"
                ),
                _ => match &value.proto_type {
                    ProtoType::Enum(name) => vec![format!(
                        "for (const auto &{kv} : {src_expr}) {{ (*{target_prefix}mutable_{field}())[{kv}.first] = static_cast<::{proto_cpp_ns}::{name}>({kv}.second); }}"
                    )],
                    _ => vec![format!(
                        "for (const auto &{kv} : {src_expr}) {{ (*{target_prefix}mutable_{field}())[{kv}.first] = {kv}.second; }}"
                    )],
                },
            }
        }
        _ => match proto_type {
            ProtoType::Enum(name) => vec![format!(
                "{target_prefix}set_{field}(static_cast<::{proto_cpp_ns}::{name}>({src_expr}));"
            )],
            _ => vec![format!("{target_prefix}set_{field}({src_expr});")],
        },
    }
}

fn signal_view(
    s: &Signal,
    unit: &Unit,
    proto_cpp_ns: &str,
    keyed_by: Option<&crate::ir::KeyType>,
) -> SignalView {
    let resolve_line = keyed_by.map(|key| {
        format!(
            "auto &impl_ = registry_.resolve(static_cast<{}>(request->id()));",
            key.cxx_type
        )
    });

    let lambda_params = s
        .fields
        .iter()
        .map(|f| format!("{} {}", f.cxx_type.spelling, f.cxx_name))
        .collect::<Vec<_>>()
        .join(", ");

    let mut event_set_stmts = Vec::new();
    for f in &s.fields {
        event_set_stmts.extend(write_wire(
            "event.",
            &f.proto_field,
            &f.cxx_name,
            &f.proto_type,
            &f.kind,
            proto_cpp_ns,
            unit,
            0,
        ));
    }

    SignalView {
        rpc_name: s.rpc_name.clone(),
        cxx_symbol: s.cxx_symbol.clone(),
        resolve_line,
        lambda_params,
        event_set_stmts,
    }
}
