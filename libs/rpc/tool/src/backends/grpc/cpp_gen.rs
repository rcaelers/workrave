//! Renders the `<Service>ServiceImpl` adapter (.hh + .cc): a `grpc::Service`
//! subclass whose method bodies unpack the protobuf request, call straight
//! into the real, unmodified, annotated C++ method, and pack the response.
//!
//! The real `<Service>::Service` base class and `<Rpc>Request`/`<Rpc>Response`
//! message types come from running the `.proto` this tool also emits through
//! actual `protoc` + `grpc_cpp_plugin` — this module only emits the glue that
//! calls into real C++, it never reimplements protobuf wire marshalling.
//!
//! Every individual C++ statement/expression is produced by rendering a
//! small `templates/cpp_*.askama` file — this module's Rust functions only
//! walk the IR (deciding *which* statement shape applies where, and
//! recursing into nested message/sequence/map fields) and hand each
//! template bare, unprocessed facts: a symbol's plain name (never pre-fixed
//! with "local_" — the templates own that naming convention, see
//! `cpp_local_decl.askama`/`cpp_bitmask_decl.askama`/`CallArgView::Local`),
//! a type spelling, an `is_pointer` bool (the `&`-or-not decision is the
//! template's `{% if %}`, not Rust's), an optional enum-cast target. No
//! function here builds C++ syntax itself via `format!` — not even a `&`
//! prefix or a naming-convention prefix.
//!
//! Askama can't express genuine recursion (`{% macro %}` self-calls are a
//! compile-time error — verified against askama_derive's source), so the
//! tree-walk over nested message/sequence/map fields (`assign_from_wire`/
//! `write_wire`) necessarily stays a real Rust function calling itself; deferred
//! open question is whether to invert that so the *template* calls into Rust
//! for recursion (askama does support calling Rust functions from `{{ }}`)
//! rather than Rust calling `.render()` on the way down. For now Rust still
//! drives that specific walk, one `.render()` per node, with the
//! already-rendered children spliced in as an `inner: &str` — but every
//! individual name/prefix/cast decision within each node's own text is the
//! template's, not Rust's.

use std::collections::HashMap;

use anyhow::Result;
use askama::Template;

use crate::ir::{Direction, Interface, Method, ParamKind, ProtoType, Signal, Unit};

struct MethodView {
    rpc_name: String,
    /// Statements declaring/initializing locals the call needs an lvalue for
    /// (recursively unpacking a message/sequence/map `in` parameter, or a
    /// bare declaration for an `out`/`inout` one) — from `assign_from_wire`,
    /// whose recursion must stay Rust: protobuf has no generic "assign this
    /// C++ value from a wire field" call the way DBus's
    /// `DBusMarshall<T>::convert` does, so unpacking a nested message's
    /// fields is an inherent tree-walk. Each statement's actual text still
    /// comes from a `cpp_*.askama` template render, never a Rust `format!`.
    pre_call_decls: Vec<String>,
    /// The real method to call, e.g. "impl_.set_operation_mode".
    cxx_symbol: String,
    /// One entry per call argument, in order — the template decides how to
    /// render each (a plain expression, or `&local_name`/`local_name` with
    /// the `&` and the `local_` prefix both written as template text) and
    /// joins them with ", " itself via `loop.first`.
    call_args: Vec<CallArgView>,
    has_return_value: bool,
    /// Statements marshalling the call's return value onto `response` (from
    /// `write_wire`, same tree-walk rationale as `pre_call_decls`) — empty
    /// when `has_return_value` is false.
    return_value_stmts: Vec<String>,
    /// Statements marshalling each `out`/`inout` parameter onto `response`,
    /// in parameter order — always run, independent of `has_return_value`.
    out_param_stmts: Vec<String>,
}

struct SignalFieldView {
    /// Bare type spelling and name — the template writes the "Type name"
    /// parameter-declaration syntax itself (and the ", "-joining, via
    /// `loop.first`), not Rust.
    cxx_type: String,
    cxx_name: String,
}

struct SignalView {
    rpc_name: String,
    /// The real accessor to connect to, e.g. "signal_operation_mode_changed".
    cxx_symbol: String,
    /// The connected lambda's parameters — see `SignalFieldView`.
    fields: Vec<SignalFieldView>,
    /// Statements building the `Event` message from the lambda's arguments.
    event_set_stmts: Vec<String>,
}

/// One call argument's shape — the template (not Rust) decides the actual
/// text: `Expr` splices an already-rendered expression in verbatim (from a
/// `cpp_expr_*.askama` template, for `in` scalar/cstring/bytes/duration/
/// absorbed-size cases, which have no "local" of their own); `Local` names a
/// declared local by its bare `cxx_name` and lets the template both add the
/// `local_` prefix and decide whether to take its address.
enum CallArgView {
    Expr(String),
    Local { cxx_name: String, is_pointer: bool },
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
    /// The template reads this directly (once, at the top of each method/
    /// signal body) rather than each MethodView/SignalView repeating an
    /// identical pre-rendered "resolve" line — the resolve statement's shape
    /// never varies per method, only whether it's present at all.
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
            .map(|m| method_view(m, unit, &proto_cpp_ns))
            .collect::<Result<Vec<_>>>()?,
        signals: iface
            .signals
            .iter()
            .map(|s| signal_view(s, unit, &proto_cpp_ns))
            .collect::<Result<Vec<_>>>()?,
    };

    let header = HeaderTemplate { view: &view }.render()?;
    let source = SourceTemplate { view: &view }.render()?;
    Ok(RenderedAdapter { header, source })
}

// ---- Small, single-statement/expression templates ----------------------
//
// Each of these mirrors exactly one C++ statement or expression shape. Rust
// only supplies the plain values (names, type spellings, an optional enum
// cast target); the template owns every keyword, operator, and brace.

struct LocalDeclView<'a> {
    cxx_type: &'a str,
    /// Bare name — the template writes the `local_` prefix itself.
    cxx_name: &'a str,
    init: Option<String>,
}

#[derive(Template)]
#[template(path = "cpp_local_decl.askama", escape = "none")]
struct LocalDeclTemplate<'a> {
    view: &'a LocalDeclView<'a>,
}

fn render_local_decl(cxx_type: &str, cxx_name: &str, init: Option<String>) -> Result<String> {
    let view = LocalDeclView { cxx_type, cxx_name, init };
    Ok(LocalDeclTemplate { view: &view }.render()?)
}

struct BitmaskDeclView<'a> {
    cxx_type: &'a str,
    /// Bare name — the template writes the `local_` prefix itself.
    cxx_name: &'a str,
    field: &'a str,
    enum_cxx_type: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_bitmask_decl.askama", escape = "none")]
struct BitmaskDeclTemplate<'a> {
    view: &'a BitmaskDeclView<'a>,
}

fn render_bitmask_decl(cxx_type: &str, cxx_name: &str, field: &str, enum_cxx_type: &str) -> Result<Vec<String>> {
    let view = BitmaskDeclView {
        cxx_type,
        cxx_name,
        field,
        enum_cxx_type,
    };
    Ok(lines_of(BitmaskDeclTemplate { view: &view }.render()?))
}

struct ReadExprView<'a> {
    field: &'a str,
    enum_cast: Option<&'a str>,
}

#[derive(Template)]
#[template(path = "cpp_expr_read.askama", escape = "none")]
struct ReadExprTemplate<'a> {
    view: &'a ReadExprView<'a>,
}

/// The expression reading an `in` `Value` parameter straight off the
/// request — `static_cast<Enum>(request->field())` for an enum, else just
/// `request->field()`.
fn request_read_expr(field: &str, proto_type: &ProtoType, cxx_base_type: &str) -> Result<String> {
    let enum_cast = match proto_type {
        ProtoType::Enum(_) => Some(cxx_base_type),
        _ => None,
    };
    let view = ReadExprView { field, enum_cast };
    Ok(ReadExprTemplate { view: &view }.render()?.trim_end().to_string())
}

/// Bare `request->field()`, no cast — the same template as
/// `request_read_expr` with no enum, used as the entry point into
/// `assign_from_wire` for a message/sequence/map `in` parameter (which is
/// never itself an enum, only its leaves can be).
fn render_wire_read(field: &str) -> Result<String> {
    let view = ReadExprView { field, enum_cast: None };
    Ok(ReadExprTemplate { view: &view }.render()?.trim_end().to_string())
}

/// The one place the `local_` naming convention is spelled out for Rust's
/// own bookkeeping (building `dest_expr`/`src_expr` strings threaded through
/// the recursive `assign_from_wire`/`write_wire` walk) — every place that
/// convention appears in *emitted* C++ text is independently written by a
/// template instead (`cpp_local_decl.askama`, `cpp_bitmask_decl.askama`, and
/// `CallArgView::Local` in `service_impl.cc.askama`), so this only exists to
/// avoid scattering the same `format!("local_{}", ...)` across call sites.
fn local_name(cxx_name: &str) -> String {
    format!("local_{cxx_name}")
}

struct CStringExprView<'a> {
    field: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_expr_cstring.askama", escape = "none")]
struct CStringExprTemplate<'a> {
    view: &'a CStringExprView<'a>,
}

struct BytesExprView<'a> {
    cxx_type: &'a str,
    field: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_expr_bytes.askama", escape = "none")]
struct BytesExprTemplate<'a> {
    view: &'a BytesExprView<'a>,
}

struct DurationExprView<'a> {
    cxx_type: &'a str,
    field: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_expr_duration.askama", escape = "none")]
struct DurationExprTemplate<'a> {
    view: &'a DurationExprView<'a>,
}

struct AbsorbedSizeExprView<'a> {
    cxx_type: &'a str,
    owner: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_expr_absorbed_size.askama", escape = "none")]
struct AbsorbedSizeExprTemplate<'a> {
    view: &'a AbsorbedSizeExprView<'a>,
}

struct AssignScalarView<'a> {
    dest: &'a str,
    wire: &'a str,
    enum_cast: Option<&'a str>,
}

#[derive(Template)]
#[template(path = "cpp_assign_scalar.askama", escape = "none")]
struct AssignScalarTemplate<'a> {
    view: &'a AssignScalarView<'a>,
}

struct AssignSequenceView<'a> {
    wire_item: &'a str,
    wire_expr: &'a str,
    elem_ty: &'a str,
    item: &'a str,
    inner: &'a str,
    dest_expr: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_assign_sequence.askama", escape = "none")]
struct AssignSequenceTemplate<'a> {
    view: &'a AssignSequenceView<'a>,
}

struct AssignMapView<'a> {
    kv: &'a str,
    wire_expr: &'a str,
    val_ty: &'a str,
    val: &'a str,
    inner: &'a str,
    dest_expr: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_assign_map.askama", escape = "none")]
struct AssignMapTemplate<'a> {
    view: &'a AssignMapView<'a>,
}

struct SetScalarView<'a> {
    target_prefix: &'a str,
    field: &'a str,
    src: &'a str,
    proto_ns: &'a str,
    enum_cast: Option<&'a str>,
}

#[derive(Template)]
#[template(path = "cpp_set_scalar.askama", escape = "none")]
struct SetScalarTemplate<'a> {
    view: &'a SetScalarView<'a>,
}

struct SetMessageView<'a> {
    msg_var: &'a str,
    target_prefix: &'a str,
    field: &'a str,
    inner: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_set_message.askama", escape = "none")]
struct SetMessageTemplate<'a> {
    view: &'a SetMessageView<'a>,
}

struct SetSequenceMessageView<'a> {
    item: &'a str,
    src_expr: &'a str,
    elem_var: &'a str,
    target_prefix: &'a str,
    field: &'a str,
    inner: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_set_sequence_message.askama", escape = "none")]
struct SetSequenceMessageTemplate<'a> {
    view: &'a SetSequenceMessageView<'a>,
}

struct SetSequenceScalarView<'a> {
    item: &'a str,
    src_expr: &'a str,
    target_prefix: &'a str,
    field: &'a str,
    proto_ns: &'a str,
    enum_cast: Option<&'a str>,
}

#[derive(Template)]
#[template(path = "cpp_set_sequence_scalar.askama", escape = "none")]
struct SetSequenceScalarTemplate<'a> {
    view: &'a SetSequenceScalarView<'a>,
}

struct SetMapMessageView<'a> {
    kv: &'a str,
    src_expr: &'a str,
    map_val_var: &'a str,
    target_prefix: &'a str,
    field: &'a str,
    inner: &'a str,
}

#[derive(Template)]
#[template(path = "cpp_set_map_message.askama", escape = "none")]
struct SetMapMessageTemplate<'a> {
    view: &'a SetMapMessageView<'a>,
}

struct SetMapScalarView<'a> {
    kv: &'a str,
    src_expr: &'a str,
    target_prefix: &'a str,
    field: &'a str,
    proto_ns: &'a str,
    enum_cast: Option<&'a str>,
}

#[derive(Template)]
#[template(path = "cpp_set_map_scalar.askama", escape = "none")]
struct SetMapScalarTemplate<'a> {
    view: &'a SetMapScalarView<'a>,
}

/// Splits a template's rendered output into one `Vec<String>` entry per
/// physical line — the shape every statement-list field on `MethodView`/
/// `SignalView` expects, so a multi-line template render (e.g.
/// `cpp_bitmask_decl.askama`, or `cpp_set_message.askama` with a non-trivial
/// `inner`) still gets indented correctly by the top-level per-line
/// `{% for line in ... %}` loop in `service_impl.cc.askama`.
fn lines_of(rendered: String) -> Vec<String> {
    rendered.lines().map(str::to_string).collect()
}

fn method_view(m: &Method, unit: &Unit, proto_cpp_ns: &str) -> Result<MethodView> {
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
    let mut out_param_stmts = Vec::new();
    let mut call_args = Vec::new();

    for p in &m.params {
        if p.proto_field.is_empty() {
            // Absorbed size parameter of a `bytes` pair.
            let owner = size_owner
                .get(p.cxx_name.as_str())
                .expect("absorbed param must have a bytes owner");
            let view = AbsorbedSizeExprView {
                cxx_type: &p.cxx_type.spelling,
                owner,
            };
            call_args.push(CallArgView::Expr(
                AbsorbedSizeExprTemplate { view: &view }.render()?.trim_end().to_string(),
            ));
            continue;
        }

        match (p.direction, &p.kind) {
            (Direction::In, ParamKind::Value) => {
                call_args.push(CallArgView::Expr(request_read_expr(
                    p.proto_field.as_str(),
                    &p.proto_type,
                    &p.cxx_type.base_spelling,
                )?));
            }
            (Direction::In, ParamKind::CString) => {
                let view = CStringExprView { field: &p.proto_field };
                call_args.push(CallArgView::Expr(
                    CStringExprTemplate { view: &view }.render()?.trim_end().to_string(),
                ));
            }
            (Direction::In, ParamKind::Bytes { .. }) => {
                let view = BytesExprView {
                    cxx_type: &p.cxx_type.spelling,
                    field: &p.proto_field,
                };
                call_args.push(CallArgView::Expr(
                    BytesExprTemplate { view: &view }.render()?.trim_end().to_string(),
                ));
            }
            (Direction::In, ParamKind::Duration) => {
                let view = DurationExprView {
                    cxx_type: &p.cxx_type.spelling,
                    field: &p.proto_field,
                };
                call_args.push(CallArgView::Expr(
                    DurationExprTemplate { view: &view }.render()?.trim_end().to_string(),
                ));
            }
            (Direction::In, ParamKind::Bitmask { enum_cxx_type }) => {
                pre_call_decls.extend(render_bitmask_decl(
                    &p.cxx_type.spelling,
                    &p.cxx_name,
                    &p.proto_field,
                    enum_cxx_type,
                )?);
                call_args.push(CallArgView::Local {
                    cxx_name: p.cxx_name.clone(),
                    is_pointer: false,
                });
            }
            (Direction::In, ParamKind::Message { .. })
            | (Direction::In, ParamKind::Sequence(_))
            | (Direction::In, ParamKind::Map { .. }) => {
                pre_call_decls.push(render_local_decl(&p.cxx_type.base_spelling, &p.cxx_name, None)?);
                pre_call_decls.extend(assign_from_wire(
                    &local_name(&p.cxx_name),
                    &render_wire_read(&p.proto_field)?,
                    &p.cxx_type.base_spelling,
                    &p.proto_type,
                    &p.kind,
                    unit,
                    0,
                )?);
                call_args.push(CallArgView::Local {
                    cxx_name: p.cxx_name.clone(),
                    is_pointer: p.cxx_type.is_pointer,
                });
            }
            (Direction::Out, ParamKind::Value) | (Direction::InOut, ParamKind::Value) => {
                let init = if p.direction == Direction::InOut {
                    Some(request_read_expr(
                        p.proto_field.as_str(),
                        &p.proto_type,
                        &p.cxx_type.base_spelling,
                    )?)
                } else {
                    None
                };
                pre_call_decls.push(render_local_decl(&p.cxx_type.base_spelling, &p.cxx_name, init)?);
                call_args.push(CallArgView::Local {
                    cxx_name: p.cxx_name.clone(),
                    is_pointer: p.cxx_type.is_pointer,
                });
                out_param_stmts.extend(write_wire(
                    "response->",
                    &p.proto_field,
                    &local_name(&p.cxx_name),
                    &p.proto_type,
                    &p.kind,
                    proto_cpp_ns,
                    unit,
                    0,
                )?);
            }
            (Direction::Out, ParamKind::Message { .. })
            | (Direction::InOut, ParamKind::Message { .. })
            | (Direction::Out, ParamKind::Sequence(_))
            | (Direction::InOut, ParamKind::Sequence(_))
            | (Direction::Out, ParamKind::Map { .. })
            | (Direction::InOut, ParamKind::Map { .. }) => {
                pre_call_decls.push(render_local_decl(&p.cxx_type.base_spelling, &p.cxx_name, None)?);
                if p.direction == Direction::InOut {
                    pre_call_decls.extend(assign_from_wire(
                        &local_name(&p.cxx_name),
                        &render_wire_read(&p.proto_field)?,
                        &p.cxx_type.base_spelling,
                        &p.proto_type,
                        &p.kind,
                        unit,
                        0,
                    )?);
                }
                call_args.push(CallArgView::Local {
                    cxx_name: p.cxx_name.clone(),
                    is_pointer: p.cxx_type.is_pointer,
                });
                out_param_stmts.extend(write_wire(
                    "response->",
                    &p.proto_field,
                    &local_name(&p.cxx_name),
                    &p.proto_type,
                    &p.kind,
                    proto_cpp_ns,
                    unit,
                    0,
                )?);
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

    let (has_return_value, return_value_stmts) = match &m.return_value {
        Some(rv) => (
            true,
            write_wire(
                "response->",
                &rv.proto_field,
                "rpc_result",
                &rv.proto_type,
                &rv.kind,
                proto_cpp_ns,
                unit,
                0,
            )?,
        ),
        None => (false, Vec::new()),
    };

    Ok(MethodView {
        rpc_name: m.rpc_name.clone(),
        pre_call_decls,
        cxx_symbol: m.cxx_symbol.clone(),
        call_args,
        has_return_value,
        return_value_stmts,
        out_param_stmts,
    })
}

/// Emits statement(s) that assign into the already-existing lvalue
/// `dest_expr` (a variable name, or a field-access expression like
/// "local.field") by reading `wire_expr` (a proto accessor call — a nested
/// message reference for `Message`, a repeated-field range for `Sequence`).
/// The recursive workhorse behind all "read from the wire into real C++"
/// marshalling: used directly for a top-level struct/sequence parameter,
/// and recursively for its fields/elements, however deeply nested. Rust
/// only decides *which* of the three statement shapes applies at each
/// level and gathers the already-rendered `inner` text for the recursive
/// case; the shape's actual C++ text always comes from a `cpp_assign_*.askama`
/// template render.
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
) -> Result<Vec<String>> {
    Ok(match kind {
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
                )?);
            }
            stmts
        }
        ParamKind::Sequence(elem) => {
            let wire_item = format!("rpc_wire_{depth}");
            let item = format!("rpc_item_{depth}");
            let inner = assign_from_wire(&item, &wire_item, &elem.cxx_type, &elem.proto_type, &elem.kind, unit, depth + 1)?
                .join(" ");
            let view = AssignSequenceView {
                wire_item: &wire_item,
                wire_expr,
                elem_ty: &elem.cxx_type,
                item: &item,
                inner: &inner,
                dest_expr,
            };
            lines_of(AssignSequenceTemplate { view: &view }.render()?)
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
            let inner = assign_from_wire(
                &val,
                &format!("{kv}.second"),
                &value.cxx_type,
                &value.proto_type,
                &value.kind,
                unit,
                depth + 1,
            )?
            .join(" ");
            let view = AssignMapView {
                kv: &kv,
                wire_expr,
                val_ty: &value.cxx_type,
                val: &val,
                inner: &inner,
                dest_expr,
            };
            lines_of(AssignMapTemplate { view: &view }.render()?)
        }
        _ => {
            let enum_cast = match proto_type {
                ProtoType::Enum(_) => Some(cxx_type_spelling),
                _ => None,
            };
            let view = AssignScalarView {
                dest: dest_expr,
                wire: wire_expr,
                enum_cast,
            };
            lines_of(AssignScalarTemplate { view: &view }.render()?)
        }
    })
}

/// Emits statement(s) that write the real C++ value `src_expr` onto the
/// proto field reached via `target_prefix` + `field` (`set_field`/
/// `mutable_field`/`add_field`, chosen by `kind`) — the write counterpart of
/// `assign_from_wire`. `target_prefix` is the C++ prefix to reach the
/// setter, e.g. "response->" for a top-level field or "event." for a signal
/// event (both fixed by the top-level template's own declared variable
/// names — `response`/`event` — not a generation choice made here);
/// recursion passes a nested message pointer's `->` prefix instead, itself
/// only ever produced by a template render (`cpp_set_message.askama`'s
/// `msg_var`).
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
) -> Result<Vec<String>> {
    Ok(match kind {
        ParamKind::Message { struct_proto_name } => {
            let struct_def = unit
                .find_struct_by_proto_name(struct_proto_name)
                .expect("Message kind always names a registered struct");
            let msg_var = format!("rpc_msg_{depth}");
            let mut inner_stmts = Vec::new();
            for f in &struct_def.fields {
                let field_src = format!("{src_expr}.{}", f.cxx_name);
                inner_stmts.extend(write_wire(
                    &format!("{msg_var}->"),
                    &f.proto_field,
                    &field_src,
                    &f.proto_type,
                    &f.kind,
                    proto_cpp_ns,
                    unit,
                    depth + 1,
                )?);
            }
            let inner = inner_stmts.join(" ");
            let view = SetMessageView {
                msg_var: &msg_var,
                target_prefix,
                field,
                inner: &inner,
            };
            lines_of(SetMessageTemplate { view: &view }.render()?)
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
                        )?);
                    }
                    let inner = field_stmts.join(" ");
                    let view = SetSequenceMessageView {
                        item: &item,
                        src_expr,
                        elem_var: &elem_var,
                        target_prefix,
                        field,
                        inner: &inner,
                    };
                    lines_of(SetSequenceMessageTemplate { view: &view }.render()?)
                }
                ParamKind::Sequence(_) => {
                    unreachable!("sequence-of-sequence is not supported")
                }
                ParamKind::Map { .. } => {
                    unreachable!("sequence-of-map is rejected at parse time (see resolve_value_type)")
                }
                _ => {
                    let enum_cast = match &elem.proto_type {
                        ProtoType::Enum(name) => Some(name.as_str()),
                        _ => None,
                    };
                    let view = SetSequenceScalarView {
                        item: &item,
                        src_expr,
                        target_prefix,
                        field,
                        proto_ns: proto_cpp_ns,
                        enum_cast,
                    };
                    lines_of(SetSequenceScalarTemplate { view: &view }.render()?)
                }
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
                    let mut field_stmts = Vec::new();
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
                        )?);
                    }
                    let inner = field_stmts.join(" ");
                    let view = SetMapMessageView {
                        kv: &kv,
                        src_expr,
                        map_val_var: &map_val_var,
                        target_prefix,
                        field,
                        inner: &inner,
                    };
                    lines_of(SetMapMessageTemplate { view: &view }.render()?)
                }
                ParamKind::Sequence(_) | ParamKind::Map { .. } => unreachable!(
                    "a map value can't be a sequence or another map (rejected at parse time)"
                ),
                _ => {
                    let enum_cast = match &value.proto_type {
                        ProtoType::Enum(name) => Some(name.as_str()),
                        _ => None,
                    };
                    let view = SetMapScalarView {
                        kv: &kv,
                        src_expr,
                        target_prefix,
                        field,
                        proto_ns: proto_cpp_ns,
                        enum_cast,
                    };
                    lines_of(SetMapScalarTemplate { view: &view }.render()?)
                }
            }
        }
        _ => {
            let enum_cast = match proto_type {
                ProtoType::Enum(name) => Some(name.as_str()),
                _ => None,
            };
            let view = SetScalarView {
                target_prefix,
                field,
                src: src_expr,
                proto_ns: proto_cpp_ns,
                enum_cast,
            };
            lines_of(SetScalarTemplate { view: &view }.render()?)
        }
    })
}

fn signal_view(s: &Signal, unit: &Unit, proto_cpp_ns: &str) -> Result<SignalView> {
    let fields = s
        .fields
        .iter()
        .map(|f| SignalFieldView {
            cxx_type: f.cxx_type.spelling.clone(),
            cxx_name: f.cxx_name.clone(),
        })
        .collect();

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
        )?);
    }

    Ok(SignalView {
        rpc_name: s.rpc_name.clone(),
        cxx_symbol: s.cxx_symbol.clone(),
        fields,
        event_set_stmts,
    })
}
