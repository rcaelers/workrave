//! Walks a header's AST via libclang, finds `@rpc`-annotated classes and
//! methods, and builds the [`crate::ir::Unit`] describing them.

use std::collections::HashSet;
use std::path::Path;

use anyhow::{bail, Context, Result};
use clang::{Accessibility, Clang, Entity, EntityKind, Index, Type, TypeKind};

use crate::annotations::{
    has_bitmask_tag, parse_enum_tag, parse_enum_value_tag, parse_method_tag, parse_param_tags,
    parse_service_tag, parse_signal_tag, ParamKindTag,
};
use crate::external_annotations::{effective_comment, ExternalAnnotations};
use crate::ir::{
    CxxType, Direction, EnumDef, EnumValue, Interface, KeyType, MapKey, Method, Param, ParamKind,
    ProtoType, ReturnValue, SequenceElement, Signal, SignalField, StructDef, StructField, Unit,
};

pub struct ParseInput<'a> {
    pub header: &'a Path,
    pub compiler_args: &'a [String],
    /// `@rpc` tags supplied by fully-qualified name instead of (or in
    /// addition to) a comment on the declaration itself — see
    /// `external_annotations`.
    pub external: &'a ExternalAnnotations,
}

pub fn parse_unit(input: &ParseInput) -> Result<Unit> {
    let clang = Clang::new().map_err(|e| anyhow::anyhow!("failed to initialize libclang: {e}"))?;
    let index = Index::new(&clang, false, true);

    let header = input
        .header
        .canonicalize()
        .with_context(|| format!("resolving header {}", input.header.display()))?;

    let mut args: Vec<String> = vec![
        "-x".to_string(),
        "c++".to_string(),
        "-fparse-all-comments".to_string(),
    ];
    args.extend(input.compiler_args.iter().cloned());
    ensure_macos_sysroot(&mut args);

    let tu = index
        .parser(&header)
        .arguments(&args)
        .parse()
        .with_context(|| format!("parsing {}", header.display()))?;

    let mut had_error = false;
    for diagnostic in tu.get_diagnostics() {
        use clang::diagnostic::Severity;
        if matches!(diagnostic.get_severity(), Severity::Error | Severity::Fatal) {
            eprintln!("clang: {diagnostic}");
            had_error = true;
        }
    }
    if had_error {
        bail!(
            "clang reported errors while parsing {} (see above)",
            header.display()
        );
    }

    let mut unit = Unit::default();
    let tu_root = tu.get_entity();
    visit_for_interfaces(&tu_root, &tu_root, &header, input.external, &mut unit)?;

    if unit.interfaces.is_empty() {
        bail!(
            "no `@rpc(service=\"...\")`-annotated class found in {}",
            header.display()
        );
    }

    Ok(unit)
}

/// On macOS, `/usr/bin/c++` (the driver `compile_commands.json` records)
/// auto-injects its SDK sysroot when invoked as a real process — libclang's
/// direct in-process parsing does not get that same default, so
/// `<cstdint>`/`<string>`/... fail to resolve unless `-isysroot` is already
/// present in the recorded flags. Add it ourselves when it's missing.
fn ensure_macos_sysroot(args: &mut Vec<String>) {
    if !cfg!(target_os = "macos") || args.iter().any(|a| a == "-isysroot") {
        return;
    }
    if let Ok(output) = std::process::Command::new("xcrun")
        .args(["--show-sdk-path"])
        .output()
    {
        if output.status.success() {
            if let Ok(path) = String::from_utf8(output.stdout) {
                let path = path.trim();
                if !path.is_empty() {
                    args.push("-isysroot".to_string());
                    args.push(path.to_string());
                }
            }
        }
    }
}

fn is_in_header(entity: &Entity, header: &Path) -> bool {
    let Some(location) = entity.get_location() else {
        return false;
    };
    let Some(file) = location.get_file_location().file else {
        return false;
    };
    match file.get_path().canonicalize() {
        Ok(p) => p == header,
        Err(_) => file.get_path() == header,
    }
}

fn visit_for_interfaces(
    tu_root: &Entity,
    entity: &Entity,
    header: &Path,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<()> {
    for child in entity.get_children() {
        match child.get_kind() {
            EntityKind::Namespace => {
                visit_for_interfaces(tu_root, &child, header, external, unit)?;
            }
            EntityKind::ClassDecl | EntityKind::StructDecl => {
                if is_in_header(&child, header) && child.is_definition() {
                    let qualified = qualified_name(&namespace_path(&child), &child.get_name().unwrap_or_default());
                    let comment = effective_comment(child.get_comment(), external.lookup(&qualified));
                    if let Some(tag) = comment.as_deref().and_then(parse_service_tag) {
                        let interface = build_interface(tu_root, &child, tag, external, unit)?;
                        unit.interfaces.push(interface);
                    }
                }
                visit_for_interfaces(tu_root, &child, header, external, unit)?;
            }
            _ => {}
        }
    }
    Ok(())
}

/// Finds an `EnumDecl` anywhere in the translation unit (not just the
/// annotated header — `keyed_by` types are often declared in a different,
/// transitively-included header, e.g. `workrave::BreakId`) whose
/// fully-qualified name matches `qualified_name`.
fn find_enum_by_qualified_name<'tu>(
    entity: &Entity<'tu>,
    qualified_name: &str,
) -> Option<Entity<'tu>> {
    for child in entity.get_children() {
        match child.get_kind() {
            EntityKind::EnumDecl => {
                let ns = namespace_path(&child);
                let name = child.get_name()?;
                let qname = if ns.is_empty() {
                    name
                } else {
                    format!("{}::{}", ns.join("::"), name)
                };
                if qname == qualified_name {
                    return Some(child);
                }
            }
            EntityKind::Namespace | EntityKind::ClassDecl | EntityKind::StructDecl => {
                if let Some(found) = find_enum_by_qualified_name(&child, qualified_name) {
                    return Some(found);
                }
            }
            _ => {}
        }
    }
    None
}

fn resolve_key_type(
    tu_root: &Entity,
    keyed_by: &str,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<KeyType> {
    let primitive = |proto_type: ProtoType, cxx_type: &str| KeyType {
        proto_type,
        cxx_type: cxx_type.to_string(),
    };
    match keyed_by {
        "string" => return Ok(primitive(ProtoType::String, "std::string")),
        "int32" => return Ok(primitive(ProtoType::Int32, "int32_t")),
        "int64" => return Ok(primitive(ProtoType::Int64, "int64_t")),
        "uint32" => return Ok(primitive(ProtoType::UInt32, "uint32_t")),
        "uint64" => return Ok(primitive(ProtoType::UInt64, "uint64_t")),
        _ => {}
    }

    let decl = find_enum_by_qualified_name(tu_root, keyed_by).with_context(|| {
        format!(
            "keyed_by=\"{keyed_by}\" is not a recognized primitive (string/int32/int64/uint32/uint64) \
             and no enum with that fully-qualified name was found anywhere in the translation unit"
        )
    })?;
    let ty = decl.get_type().context("enum declaration has no type")?;
    let proto_name = register_enum(&decl, &ty, external, unit)?;
    Ok(KeyType {
        proto_type: ProtoType::Enum(proto_name),
        cxx_type: ty.get_display_name(),
    })
}

fn namespace_path(entity: &Entity) -> Vec<String> {
    let mut ns = Vec::new();
    let mut current = entity.get_semantic_parent();
    while let Some(e) = current {
        if e.get_kind() == EntityKind::Namespace {
            if let Some(name) = e.get_name() {
                ns.push(name);
            }
        }
        current = e.get_semantic_parent();
    }
    ns.reverse();
    ns
}

/// The general form of `namespace_path()`: every enclosing scope's name,
/// outermost first, including *class/struct* scopes as well as namespaces
/// (unlike `namespace_path()`, which only collects namespace segments).
/// Needed because `Type::get_display_name()` sometimes prints a
/// class-nested type (e.g. `GenericDBusApplet::TimerData`, referenced from
/// a method of that same class) the way it appears from *that*
/// declaration's own lexical scope — bare "TimerData", not qualified — which
/// is fine when re-used from inside that same class, but doesn't resolve at
/// all in generated code that lives outside it (confirmed by an actual
/// compile: clang's own fix-it suggests exactly this qualification). See
/// `qualified_type_spelling`.
fn enclosing_scope_path(entity: &Entity) -> Vec<String> {
    let mut path = Vec::new();
    let mut current = entity.get_semantic_parent();
    while let Some(e) = current {
        if matches!(
            e.get_kind(),
            EntityKind::Namespace | EntityKind::ClassDecl | EntityKind::StructDecl | EntityKind::ClassTemplate
        ) {
            if let Some(name) = e.get_name() {
                path.push(name);
            }
        }
        current = e.get_semantic_parent();
    }
    path.reverse();
    path
}

/// Fully qualifies `decl`'s own name by walking all its enclosing scopes —
/// used for `EnumDef`/`StructDef::cxx_symbol` (a memoization/lookup key
/// only, never emitted into generated code, so any stable qualified form is
/// fine here) and, for structs specifically, also for `CxxType::spelling`
/// (which *is* emitted — see `qualified_type_spelling`).
fn fully_qualified(decl: &Entity) -> Option<String> {
    let name = decl.get_name()?;
    let mut path = enclosing_scope_path(decl);
    path.push(name);
    Some(path.join("::"))
}

/// For a plain (non-template) struct/class or enum type, returns its fully
/// qualified spelling — see `enclosing_scope_path`. For `std::vector<T>`/
/// `std::list<T>`/`std::map<K, V>` specifically, recurses into the template
/// arguments and reconstructs e.g. "std::vector<Qualified::T>" — needed
/// because `Type::get_display_name()` prints a template argument the same
/// (possibly bare/unqualified) way it would print that type on its own, so
/// the class-nesting problem this function exists to fix propagates *inside*
/// a container's template arguments too, not just at the top level. Returns
/// `None` for anything else (scalars, other template specializations like
/// `Flags<T>`, which have no known class-nesting issue so far and are left
/// to `Type::get_display_name()`).
fn qualified_type_spelling(ty: &Type) -> Option<String> {
    // Always resolve through the canonical type, not `ty` itself: a plain
    // type alias (e.g. `using MenuItems = std::list<MenuItem>;`) has no
    // declaration of its own reachable via `Type::get_declaration()` (that's
    // only for real record/enum declarations) or template arguments of its
    // own, even though what it names does — querying `ty` directly would
    // silently fall through to the (still-broken) `Type::get_display_name()`
    // for exactly the alias case this function exists to fix.
    let canonical = ty.get_canonical_type();
    match canonical.get_kind() {
        TypeKind::Record if canonical.get_template_argument_types().is_none() => {
            fully_qualified(&canonical.get_declaration()?)
        }
        TypeKind::Enum => fully_qualified(&canonical.get_declaration()?),
        TypeKind::Record => {
            let decl = canonical.get_declaration()?;
            let name = decl.get_name()?;
            let mut args = canonical.get_template_argument_types()?.into_iter();
            match name.as_str() {
                "vector" | "list" => {
                    let element_ty = args.next()??;
                    let element_spelling = qualified_type_spelling(&element_ty)
                        .unwrap_or_else(|| element_ty.get_display_name());
                    Some(format!("std::{name}<{element_spelling}>"))
                }
                "map" => {
                    let key_ty = args.next()??;
                    let value_ty = args.next()??;
                    let key_spelling = qualified_type_spelling(&key_ty)
                        .unwrap_or_else(|| key_ty.get_display_name());
                    let value_spelling = qualified_type_spelling(&value_ty)
                        .unwrap_or_else(|| value_ty.get_display_name());
                    Some(format!("std::map<{key_spelling}, {value_spelling}>"))
                }
                _ => None,
            }
        }
        _ => None,
    }
}

fn qualified_name(ns: &[String], name: &str) -> String {
    if ns.is_empty() {
        name.to_string()
    } else {
        format!("{}::{}", ns.join("::"), name)
    }
}

/// The key an external annotations file uses to target one specific method
/// overload: `Class::method(ParamType1,ParamType2)`, param types spelled
/// exactly as `arg.get_type().get_display_name()` gives them (matching what
/// a reader sees at the declaration) — see `external_annotations`.
fn qualified_method_signature(class_qualified: &str, method_entity: &Entity) -> Option<String> {
    let name = method_entity.get_name()?;
    let params = method_entity
        .get_arguments()
        .unwrap_or_default()
        .iter()
        .map(|a| a.get_type().map(|t| t.get_display_name()).unwrap_or_default())
        .collect::<Vec<_>>()
        .join(",");
    Some(format!("{class_qualified}::{name}({params})"))
}

/// Whether `ty` is (a possibly-`using`-aliased) `std::chrono::duration<Rep,
/// Period>` — matched structurally by name, not by annotation: unlike
/// `Flags<Enum>` (see `flags_enum_type`), any duration is representable the
/// same way regardless of its Rep/Period, so there's nothing an annotation
/// could usefully opt in or out of. Matched on the canonical type's display
/// name rather than by walking namespace segments: libc++ implements
/// `std::chrono` inside an inline namespace (`std::__1::chrono::...`), which
/// is a real, distinct AST node when walked via `get_semantic_parent()` even
/// though it's invisible in normal diagnostics/display.
fn is_chrono_duration(ty: &Type) -> bool {
    ty.get_canonical_type()
        .get_display_name()
        .contains("chrono::duration<")
}

/// If `ty` is a specialization of some class template named `Flags` whose
/// own doc comment (real, external, or both — see `external_annotations`)
/// carries `@rpc.bitmask` (see `libs/utils/include/utils/Enum.hh`'s
/// `workrave::utils::Flags`, the only instance of this pattern in the tree
/// today — the check itself doesn't hardcode that namespace, staying
/// consistent with the rest of this tool's "no Workrave-specific concepts"
/// design), returns the enum type argument. `Type::get_declaration()` on a
/// template specialization type gives an (implicit) instantiation cursor,
/// not the primary template — `Entity::get_template()` (wrapping
/// `clang_getSpecializedCursorTemplate`) is what walks back to the cursor
/// that actually carries the doc comment.
fn flags_enum_type<'tu>(ty: &Type<'tu>, external: &ExternalAnnotations) -> Option<Type<'tu>> {
    let decl = ty.get_declaration()?;
    if decl.get_name().as_deref() != Some("Flags") {
        return None;
    }
    let primary_template = decl.get_template()?;
    let qualified = qualified_name(&namespace_path(&primary_template), "Flags");
    let comment = effective_comment(primary_template.get_comment(), external.lookup(&qualified));
    if !comment.as_deref().is_some_and(has_bitmask_tag) {
        return None;
    }
    let args = ty.get_template_argument_types()?;
    args.into_iter().next()?
}

fn build_interface(
    tu_root: &Entity,
    class_entity: &Entity,
    tag: crate::annotations::ServiceTag,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<Interface> {
    let service_name = tag.name;
    let cxx_class = class_entity
        .get_name()
        .context("annotated class has no name")?;
    let cxx_namespace = namespace_path(class_entity);

    let keyed_by = tag
        .keyed_by
        .as_deref()
        .map(|k| resolve_key_type(tu_root, k, external, unit))
        .transpose()
        .with_context(|| format!("interface '{service_name}' ({cxx_class})"))?;

    let class_qualified = qualified_name(&cxx_namespace, &cxx_class);

    let mut methods = Vec::new();
    let mut signals = Vec::new();
    for child in class_entity.get_children() {
        if child.get_kind() != EntityKind::Method {
            continue;
        }
        let method_cxx_name = child.get_name().unwrap_or_default();
        let external_text = qualified_method_signature(&class_qualified, &child)
            .and_then(|sig| external.lookup(&sig));
        let Some(comment) = effective_comment(child.get_comment(), external_text) else {
            continue;
        };

        if let Some(tag) = parse_signal_tag(&comment) {
            let signal = build_signal(&child, tag, external, unit)
                .with_context(|| format!("processing {cxx_class}::{method_cxx_name}"))?;
            signals.push(signal);
            continue;
        }
        let Some(rpc_name) = parse_method_tag(&comment) else {
            continue;
        };
        let method = build_method(&child, rpc_name, &comment, external, unit)
            .with_context(|| format!("processing {cxx_class}::{method_cxx_name}"))?;
        methods.push(method);
    }

    if methods.is_empty() && signals.is_empty() {
        bail!(
            "interface '{service_name}' ({cxx_class}) has no `@rpc(name=\"...\")`- or \
             `@rpc.signal(name=\"...\")`-annotated members"
        );
    }

    Ok(Interface {
        service_name,
        cxx_class,
        cxx_namespace,
        methods,
        signals,
        keyed_by,
    })
}

/// A `boost::signals2::signal<void(Args...)> &` accessor's argument types,
/// or `None` if `ty` (the accessor's return type) isn't that shape at all.
fn signal_argument_types<'tu>(ty: &Type<'tu>) -> Option<Vec<Type<'tu>>> {
    let owned_pointee = ty.get_pointee_type();
    let pointee = owned_pointee.as_ref().unwrap_or(ty);

    let display = pointee.get_display_name();
    let unqualified = display.strip_prefix("const ").unwrap_or(&display);
    if !unqualified.starts_with("boost::signals2::signal") {
        return None;
    }

    let template_args = pointee.get_template_argument_types()?;
    let fn_type = template_args.into_iter().next()??;
    fn_type.get_argument_types()
}

fn build_signal(
    method_entity: &Entity,
    tag: crate::annotations::SignalTag,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<Signal> {
    let cxx_symbol = method_entity
        .get_name()
        .context("signal accessor has no name")?;
    let rpc_name = tag.name;

    let result_ty = method_entity
        .get_result_type()
        .context("signal accessor has no return type")?;
    let arg_types = signal_argument_types(&result_ty).with_context(|| {
        format!(
            "{rpc_name}: @rpc.signal must annotate a `boost::signals2::signal<void(...)> &` accessor, \
             but {cxx_symbol}'s return type is '{}'",
            result_ty.get_display_name()
        )
    })?;

    let field_names: Vec<String> = match tag.fields {
        Some(names) => {
            if names.len() != arg_types.len() {
                bail!(
                    "{rpc_name}: fields=\"...\" names {} field(s) but the signal has {} argument(s)",
                    names.len(),
                    arg_types.len()
                );
            }
            names
        }
        None => match arg_types.len() {
            0 => Vec::new(),
            1 => vec!["value".to_string()],
            n => bail!(
                "{rpc_name}: signal has {n} arguments; specify fields=\"a,b,...\" to name them"
            ),
        },
    };

    let mut fields = Vec::new();
    for (name, ty) in field_names.into_iter().zip(arg_types.iter()) {
        let cxx_type = cxx_type_of(ty);
        let (proto_type, kind) = resolve_value_type(ty, external, unit)
            .with_context(|| format!("{rpc_name}: field '{name}'"))?;
        fields.push(SignalField {
            cxx_name: name.clone(),
            cxx_type,
            proto_field: name,
            proto_type,
            kind,
        });
    }

    Ok(Signal {
        rpc_name,
        cxx_symbol,
        fields,
    })
}

fn build_method(
    method_entity: &Entity,
    rpc_name: String,
    comment: &str,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<Method> {
    let cxx_symbol = method_entity.get_name().context("method has no name")?;
    let param_tags = parse_param_tags(comment)?;
    let is_const = method_entity.is_const_method();

    let absorbed: HashSet<String> = param_tags.iter().filter_map(|t| t.size.clone()).collect();

    let arg_entities = method_entity.get_arguments().unwrap_or_default();
    let mut params = Vec::new();
    for arg in &arg_entities {
        let name = arg.get_name().with_context(|| {
            format!("{rpc_name}: an @rpc method's parameters must all be named")
        })?;
        let ty = arg.get_type().context("parameter has no type")?;
        let cxx_type = cxx_type_of(&ty);

        if absorbed.contains(&name) {
            // The paired length of some other `kind=bytes` parameter: still a
            // real call argument, but not a proto field of its own.
            params.push(Param {
                cxx_name: name,
                cxx_type,
                direction: Direction::In,
                kind: ParamKind::Value,
                proto_field: String::new(),
                proto_type: ProtoType::Int64,
            });
            continue;
        }

        let tag = param_tags.iter().find(|t| t.name == name);
        let direction = match tag {
            Some(t) => t.direction,
            None => {
                if cxx_type.is_pointer || (cxx_type.is_ref && !cxx_type.is_const) {
                    bail!(
                        "{rpc_name}: parameter '{name}' is a non-const pointer/reference and has no \
                         @rpc.param(...) tag (add dir=in|out|inout; for char*/void* also add kind=cstring|bytes)"
                    );
                }
                Direction::In
            }
        };
        let kind_tag = tag.and_then(|t| t.kind);
        let size_param = tag.and_then(|t| t.size.clone());

        if kind_tag.is_some() && direction != Direction::In {
            bail!(
                "{rpc_name}: parameter '{name}' uses kind=cstring/bytes with dir={direction:?} — v1 only \
                 supports cstring/bytes buffers as `in` parameters (out/inout buffer semantics need a \
                 capacity/length contract that isn't defined yet, see the plan's non-goals)"
            );
        }

        let (proto_type, kind) = resolve_param_type(&ty, &cxx_type, kind_tag, size_param, external, unit)
            .with_context(|| format!("{rpc_name}: parameter '{name}'"))?;

        if matches!(kind, ParamKind::Duration | ParamKind::Bitmask { .. }) && direction != Direction::In {
            bail!(
                "{rpc_name}: parameter '{name}' is a chrono duration or Flags<> bitmask with \
                 dir={direction:?} — v1 only supports these as `in` parameters"
            );
        }

        params.push(Param {
            cxx_name: name.clone(),
            cxx_type,
            direction,
            kind,
            proto_field: name,
            proto_type,
        });
    }

    let result_ty = method_entity
        .get_result_type()
        .context("method has no return type")?;
    let return_value = if result_ty.get_display_name() == "void" {
        None
    } else {
        let cxx_type = cxx_type_of(&result_ty);
        let (proto_type, kind) = resolve_param_type(&result_ty, &cxx_type, None, None, external, unit)
            .with_context(|| format!("{rpc_name}: return type"))?;
        if matches!(kind, ParamKind::Duration | ParamKind::Bitmask { .. }) {
            bail!(
                "{rpc_name}: returning a chrono duration or Flags<> bitmask isn't supported yet \
                 (only as an input parameter) — '{}' returns {}",
                rpc_name,
                cxx_type.spelling
            );
        }
        Some(ReturnValue {
            cxx_type,
            proto_field: "result".to_string(),
            proto_type,
            kind,
        })
    };

    Ok(Method {
        rpc_name,
        cxx_symbol,
        params,
        return_value,
        is_const,
    })
}

fn resolve_param_type(
    ty: &Type,
    cxx_type: &CxxType,
    kind_tag: Option<ParamKindTag>,
    size_param: Option<String>,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<(ProtoType, ParamKind)> {
    let pointee = if cxx_type.is_pointer || cxx_type.is_ref {
        ty.get_pointee_type()
    } else {
        None
    };

    if cxx_type.is_pointer {
        if let Some(pointee_ty) = &pointee {
            if is_char_like(pointee_ty) {
                return match kind_tag {
                    Some(ParamKindTag::CString) => Ok((ProtoType::String, ParamKind::CString)),
                    Some(ParamKindTag::Bytes) => {
                        let size_param = size_param.context("kind=bytes requires size=<param-name>")?;
                        Ok((ProtoType::Bytes, ParamKind::Bytes { size_param }))
                    }
                    None => bail!("pointer to char/void needs kind=cstring or kind=bytes on its @rpc.param(...) tag"),
                };
            }
        }
    }

    let value_ty = pointee.as_ref().unwrap_or(ty);
    resolve_value_type(value_ty, external, unit)
}

fn is_char_like(ty: &Type) -> bool {
    matches!(
        ty.get_kind(),
        TypeKind::CharS | TypeKind::SChar | TypeKind::UChar | TypeKind::CharU | TypeKind::Void
    )
}

/// Resolves any C++ *value* type to its proto representation — the single
/// recursive place every "what proto type does this map to" question goes
/// through, whether the type is a (dereferenced) method parameter/return, a
/// signal field, or (recursively) a struct's own field or a sequence's
/// element type. Struct and sequence support (added once, here) therefore
/// apply uniformly everywhere a type can appear, not just at the top level.
fn resolve_value_type(
    ty: &Type,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<(ProtoType, ParamKind)> {
    if is_chrono_duration(ty) {
        return Ok((ProtoType::String, ParamKind::Duration));
    }

    if let Some(enum_ty) = flags_enum_type(ty, external) {
        let decl = enum_ty
            .get_declaration()
            .context("Flags<> enum argument has no declaration")?;
        let proto_name = register_enum(&decl, &enum_ty, external, unit)?;
        return Ok((
            ProtoType::Repeated(Box::new(ProtoType::Enum(proto_name))),
            ParamKind::Bitmask {
                enum_cxx_type: qualified_type_spelling(&enum_ty)
                    .unwrap_or_else(|| enum_ty.get_display_name()),
            },
        ));
    }

    if let Some(element_ty) = sequence_element_type(ty) {
        let (element_proto_type, element_kind) = resolve_value_type(&element_ty, external, unit)
            .context("sequence element type")?;
        if matches!(element_kind, ParamKind::Map { .. }) {
            bail!(
                "sequence element type '{}' can't be a map — protobuf has no \"repeated map\" \
                 syntax (a map field is already wire-repeated on its own)",
                element_ty.get_display_name()
            );
        }
        return Ok((
            ProtoType::Repeated(Box::new(element_proto_type.clone())),
            ParamKind::Sequence(SequenceElement {
                cxx_type: qualified_type_spelling(&element_ty)
                    .unwrap_or_else(|| element_ty.get_display_name()),
                proto_type: element_proto_type,
                kind: Box::new(element_kind),
            }),
        ));
    }

    if let Some((key_ty, value_ty)) = map_key_value_type(ty) {
        let (key_proto_type, key_kind) = resolve_value_type(&key_ty, external, unit)
            .context("map key type")?;
        if !matches!(key_kind, ParamKind::Value)
            || !matches!(
                key_proto_type,
                ProtoType::Bool
                    | ProtoType::Int32
                    | ProtoType::Int64
                    | ProtoType::UInt32
                    | ProtoType::UInt64
                    | ProtoType::String
            )
        {
            bail!(
                "map key type '{}' isn't a protobuf-legal map key (only bool, integer types, \
                 and std::string are allowed — not enums, floats, structs, or containers)",
                key_ty.get_display_name()
            );
        }

        let (value_proto_type, value_kind) = resolve_value_type(&value_ty, external, unit)
            .context("map value type")?;
        if matches!(value_kind, ParamKind::Sequence(_) | ParamKind::Map { .. }) {
            bail!(
                "map value type '{}' can't be a sequence or another map — protobuf doesn't \
                 allow a map value to be repeated",
                value_ty.get_display_name()
            );
        }

        return Ok((
            ProtoType::Map(
                Box::new(key_proto_type.clone()),
                Box::new(value_proto_type.clone()),
            ),
            ParamKind::Map {
                key: MapKey {
                    cxx_type: qualified_type_spelling(&key_ty)
                        .unwrap_or_else(|| key_ty.get_display_name()),
                    proto_type: key_proto_type,
                },
                value: Box::new(SequenceElement {
                    cxx_type: qualified_type_spelling(&value_ty)
                        .unwrap_or_else(|| value_ty.get_display_name()),
                    proto_type: value_proto_type,
                    kind: Box::new(value_kind),
                }),
            },
        ));
    }

    let display = ty.get_display_name();
    // `ty` here is often a dereferenced pointee (see resolve_param_type), so a
    // `const std::string &` parameter arrives as the pointee type "const
    // std::string" — strip a leading "const " before comparing rather than
    // requiring an exact "std::string" match.
    let unqualified = display.strip_prefix("const ").unwrap_or(&display);
    if unqualified == "std::string" || unqualified.contains("basic_string") {
        return Ok((ProtoType::String, ParamKind::Value));
    }

    let canonical = ty.get_canonical_type();
    match canonical.get_kind() {
        TypeKind::Bool => Ok((ProtoType::Bool, ParamKind::Value)),
        TypeKind::SChar | TypeKind::Short | TypeKind::Int => Ok((ProtoType::Int32, ParamKind::Value)),
        TypeKind::UChar | TypeKind::UShort | TypeKind::UInt => Ok((ProtoType::UInt32, ParamKind::Value)),
        TypeKind::Long | TypeKind::LongLong => Ok((ProtoType::Int64, ParamKind::Value)),
        TypeKind::ULong | TypeKind::ULongLong => Ok((ProtoType::UInt64, ParamKind::Value)),
        TypeKind::Double => Ok((ProtoType::Double, ParamKind::Value)),
        TypeKind::Float => Ok((ProtoType::Float, ParamKind::Value)),
        TypeKind::Enum => {
            let decl = canonical.get_declaration().context("enum type has no declaration")?;
            let proto_name = register_enum(&decl, ty, external, unit)?;
            Ok((ProtoType::Enum(proto_name), ParamKind::Value))
        }
        TypeKind::Record => {
            let struct_proto_name = register_struct(ty, external, unit)?;
            Ok((
                ProtoType::Message(struct_proto_name.clone()),
                ParamKind::Message { struct_proto_name },
            ))
        }
        other => bail!(
            "unsupported type '{display}' (canonical kind {other:?}); only bool, integers, \
             double/float, enums, std::string, std::vector/std::list, std::map, and plain \
             structs are supported"
        ),
    }
}

/// If `ty` is (a possibly cv/ref-qualified) `std::vector<T>` or `std::list<T>`
/// — matched structurally by declaration name, any T — returns T. The
/// container choice (vector vs list) only matters for which literal C++
/// container type generated code declares (`p.cxx_type.spelling` already
/// carries that), not for the wire representation, which is always
/// `repeated` either way.
fn sequence_element_type<'tu>(ty: &Type<'tu>) -> Option<Type<'tu>> {
    let canonical = ty.get_canonical_type();
    let decl = canonical.get_declaration()?;
    if !matches!(decl.get_name().as_deref(), Some("vector") | Some("list")) {
        return None;
    }
    let args = ty.get_template_argument_types()?;
    args.into_iter().next()?
}

/// If `ty` is (a possibly cv/ref-qualified) `std::map<K, V>` — matched
/// structurally by declaration name — returns (K, V).
fn map_key_value_type<'tu>(ty: &Type<'tu>) -> Option<(Type<'tu>, Type<'tu>)> {
    let canonical = ty.get_canonical_type();
    let decl = canonical.get_declaration()?;
    if decl.get_name().as_deref() != Some("map") {
        return None;
    }
    let mut args = ty.get_template_argument_types()?.into_iter();
    let key = args.next()??;
    let value = args.next()??;
    Some((key, value))
}

/// Registers a plain struct/class type the first time it's referenced by an
/// `@rpc` declaration (directly, or recursively as a struct/sequence field)
/// — the struct equivalent of `register_enum`. No annotation needed on the
/// struct itself, same reasoning as enums: it's only ever registered because
/// an already-`@rpc`-annotated declaration references it (directly or
/// transitively), so the gate is already at that outer declaration. Only
/// *public* data members become fields (silently skipping private/protected
/// ones, mirroring how private data isn't part of the class's public
/// contract); a struct with no public data members at all is a hard error,
/// not a silently empty message.
fn register_struct(ty: &Type, external: &ExternalAnnotations, unit: &mut Unit) -> Result<String> {
    let decl = ty.get_declaration().context("struct/class type has no declaration")?;
    let cxx_symbol = fully_qualified(&decl).unwrap_or_else(|| ty.get_display_name());
    if let Some(existing) = unit.find_struct(&cxx_symbol) {
        return Ok(existing.proto_name.clone());
    }

    let short_name = decl.get_name().unwrap_or_else(|| cxx_symbol.clone());

    let mut fields = Vec::new();
    for child in decl.get_children() {
        if child.get_kind() != EntityKind::FieldDecl {
            continue;
        }
        if child.get_accessibility() != Some(Accessibility::Public) {
            continue;
        }
        let Some(name) = child.get_name() else {
            continue;
        };
        let field_ty = child.get_type().context("field has no type")?;
        let field_cxx_type = cxx_type_of(&field_ty);
        let (proto_type, kind) = resolve_value_type(&field_ty, external, unit)
            .with_context(|| format!("{cxx_symbol}: field '{name}'"))?;
        fields.push(StructField {
            cxx_name: name.clone(),
            cxx_type: field_cxx_type,
            proto_field: name,
            proto_type,
            kind,
        });
    }

    if fields.is_empty() {
        bail!(
            "struct '{cxx_symbol}' has no public data members — @rpc doesn't know how to represent it"
        );
    }

    unit.structs.push(StructDef {
        proto_name: short_name.clone(),
        cxx_symbol,
        fields,
    });
    Ok(short_name)
}

/// Registers an enum type the first time it's referenced by an `@rpc`
/// declaration. The gRPC/protobuf side always uses the auto-derived
/// SCREAMING_SNAKE_CASE `proto_name`s below, unaffected by any of this —
/// `canonical_name` is a separate, optional, backend-agnostic name pinned
/// via `@rpc.enum(name="...")` (on the enum type) / `@rpc.enum.value(name="...")`
/// (on one enumerator), for a future backend that needs to reproduce an
/// exact existing wire name it doesn't get to choose (e.g. a DBus backend
/// matching `workrave-service.xml`'s `<enum name="operation_mode">` /
/// `<value name="normal">` — DBus has no native enum type, so this
/// tree's existing DBus bindings put the *name* on the wire as a string,
/// not the auto-derived C++ identifier).
fn register_enum(
    decl: &Entity,
    ty: &Type,
    external: &ExternalAnnotations,
    unit: &mut Unit,
) -> Result<String> {
    let cxx_symbol = fully_qualified(decl).unwrap_or_else(|| ty.get_display_name());
    if let Some(existing) = unit.find_enum(&cxx_symbol) {
        return Ok(existing.proto_name.clone());
    }

    let short_name = decl.get_name().unwrap_or_else(|| cxx_symbol.clone());
    let prefix = to_screaming_snake(&short_name);

    let enum_comment = effective_comment(decl.get_comment(), external.lookup(&cxx_symbol));
    let canonical_name = enum_comment.as_deref().and_then(parse_enum_tag);

    let mut values = Vec::new();
    for child in decl.get_children() {
        if child.get_kind() != EntityKind::EnumConstantDecl {
            continue;
        }
        let Some(name) = child.get_name() else {
            continue;
        };
        let value_qualified = format!("{cxx_symbol}::{name}");
        let value_comment = effective_comment(child.get_comment(), external.lookup(&value_qualified));
        let value_canonical_name = value_comment.as_deref().and_then(parse_enum_value_tag);

        values.push(EnumValue {
            proto_name: format!("{prefix}_{}", to_screaming_snake(&name)),
            cxx_symbol: value_qualified,
            canonical_name: value_canonical_name,
        });
    }

    if values.is_empty() {
        bail!("enum '{cxx_symbol}' has no enumerators");
    }

    unit.enums.push(EnumDef {
        proto_name: short_name.clone(),
        cxx_symbol,
        values,
        canonical_name,
    });
    Ok(short_name)
}

fn to_screaming_snake(s: &str) -> String {
    let mut out = String::new();
    let mut prev_lower_or_digit = false;
    for c in s.chars() {
        if c == '_' {
            out.push('_');
            prev_lower_or_digit = false;
            continue;
        }
        if c.is_uppercase() && prev_lower_or_digit {
            out.push('_');
        }
        out.push(c.to_ascii_uppercase());
        prev_lower_or_digit = c.is_lowercase() || c.is_ascii_digit();
    }
    out
}

fn cxx_type_of(ty: &Type) -> CxxType {
    let is_pointer = matches!(ty.get_kind(), TypeKind::Pointer);
    let is_ref = matches!(
        ty.get_kind(),
        TypeKind::LValueReference | TypeKind::RValueReference
    );
    let pointee = if is_pointer || is_ref {
        ty.get_pointee_type()
    } else {
        None
    };
    let is_const = match &pointee {
        Some(p) => p.is_const_qualified(),
        None => ty.is_const_qualified(),
    };
    let base_ty = pointee.as_ref().unwrap_or(ty);
    let base_spelling =
        qualified_type_spelling(base_ty).unwrap_or_else(|| base_ty.get_display_name());

    // The pointer/reference case keeps the original (possibly-unqualified,
    // for a class-nested struct) full spelling: none of the generated
    // code's `.spelling`-consuming paths (Bytes/CString reinterpret_cast,
    // Bitmask/Duration declarations) ever apply to a struct pointer/ref —
    // those always go through `.base_spelling` instead (see cpp_gen.rs) —
    // so there's nothing depending on this being qualified too.
    let spelling = if pointee.is_some() {
        ty.get_display_name()
    } else {
        base_spelling.clone()
    };

    CxxType {
        spelling,
        base_spelling,
        is_pointer,
        is_ref,
        is_const,
    }
}
