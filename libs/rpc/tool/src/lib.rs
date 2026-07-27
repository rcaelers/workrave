//! `clang-rpc-gen`: generates C++ RPC glue (gRPC, DBus) from an annotated
//! C++ header, using libclang to parse the header directly — no separate
//! IDL, no changes to the annotated code. See `README.md` for the tag
//! vocabulary (`@rpc(service=...)`, `@rpc(name=...)`, `@rpc.param(...)`,
//! `@rpc.dbus(interface=...)`).
//!
//! This crate is deliberately generic: it knows nothing about any specific
//! consumer project. All project-specific input (which header, which proto
//! package, where outputs go) comes in through [`GenerateOptions`].
//!
//! [`generate`] itself is target-agnostic: it parses the header once, then
//! asks each requested [`backend::Backend`] (`backends::grpc`, always;
//! `backends::dbus`, when requested) for its output files, and writes
//! whatever they return. It has no knowledge of what either backend
//! actually produces — see `backend.rs` and `backends/`.

pub mod annotations;
pub mod backend;
pub mod backends;
pub mod clang_index;
pub mod compile_db;
pub mod external_annotations;
pub mod ir;
mod template_engine;
mod template_model;

use std::fs;
use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};

use backend::Backend;
use external_annotations::ExternalAnnotations;

pub struct GenerateOptions {
    pub header: PathBuf,
    pub anchor_source: PathBuf,
    pub compile_commands: PathBuf,
    pub out_proto: PathBuf,
    pub out_adapter_hh: PathBuf,
    pub out_adapter_cc: PathBuf,
    pub proto_package: String,
    /// Optional separate schema/package for generated payload types. When
    /// present, the service schema imports this file, keeping the service's
    /// wire package independent from the payloads' generated C++ namespace.
    /// Both fields must be supplied together.
    pub out_types_proto: Option<PathBuf>,
    pub proto_types_package: Option<String>,
    /// Optional namespace appended to the protobuf package for generated
    /// gRPC service/stub classes. This is the `grpc_cpp_plugin`
    /// `services_namespace` option and does not change the wire service name.
    pub grpc_services_namespace: Option<String>,
    /// Optional C++ namespace that wraps generated ServiceImpl and DBus
    /// binding adapters. Wire package/interface names remain unchanged.
    pub adapter_namespace: Option<String>,
    /// Literal text for the generated `#include "..."` of the annotated
    /// header. Defaults to the header's file name.
    pub header_include: Option<String>,
    /// A file supplying `@rpc` tags by fully-qualified name, for
    /// declarations that can't carry an annotation comment of their own
    /// (third-party/generated headers) — see `external_annotations`.
    pub external_annotations: Option<PathBuf>,
    /// Where to write the generated DBus binding header/source (see
    /// `backends::dbus`), if requested. Both must be given together, and
    /// the interface must carry `@rpc.dbus(interface="...")` — DBus
    /// generation is entirely opt-in, on top of (never instead of) the
    /// gRPC output above.
    pub out_dbus_hh: Option<PathBuf>,
    pub out_dbus_cc: Option<PathBuf>,
}

#[derive(Debug)]
pub struct GeneratedFiles {
    pub proto: PathBuf,
    pub types_proto: Option<PathBuf>,
    pub adapter_hh: PathBuf,
    pub adapter_cc: PathBuf,
    pub dbus_hh: Option<PathBuf>,
    pub dbus_cc: Option<PathBuf>,
}

pub fn generate(opts: &GenerateOptions) -> Result<GeneratedFiles> {
    let flags = compile_db::resolve_flags(&opts.compile_commands, &opts.anchor_source)
        .context("resolving compiler flags via compile_commands.json")?;

    let external = match &opts.external_annotations {
        Some(path) => ExternalAnnotations::load(path)?,
        None => ExternalAnnotations::default(),
    };

    let unit = clang_index::parse_unit(&clang_index::ParseInput {
        header: &opts.header,
        compiler_args: &flags,
        external: &external,
    })
    .with_context(|| format!("parsing {}", opts.header.display()))?;

    if unit.interfaces.len() != 1 {
        bail!(
            "expected exactly one @rpc(service=\"...\") annotated class in {}, found {} \
             (one header -> one interface in v1; split multi-interface headers)",
            opts.header.display(),
            unit.interfaces.len()
        );
    }
    let header_include = opts.header_include.clone().unwrap_or_else(|| {
        opts.header
            .file_name()
            .expect("header has a file name")
            .to_string_lossy()
            .to_string()
    });

    // gRPC is unconditional; DBus is opt-in via a matched pair of CLI flags
    // — this is the one and only place that decides *which* backends run,
    // and with what configuration. Once a backend is in this list, the
    // driver loop below treats it identically to any other: it has no idea
    // "grpc" or "dbus" mean anything in particular.
    let mut active: Vec<Box<dyn Backend>> = vec![Box::new(backends::grpc::GrpcBackend {
        out_proto: opts.out_proto.clone(),
        out_adapter_hh: opts.out_adapter_hh.clone(),
        out_adapter_cc: opts.out_adapter_cc.clone(),
        proto_package: opts.proto_package.clone(),
        out_types_proto: opts.out_types_proto.clone(),
        proto_types_package: opts.proto_types_package.clone(),
        grpc_services_namespace: opts.grpc_services_namespace.clone(),
        adapter_namespace: opts.adapter_namespace.clone(),
    })];
    match (&opts.out_dbus_hh, &opts.out_dbus_cc) {
        (Some(out_hh), Some(out_cc)) => {
            active.push(Box::new(backends::dbus::DbusBackend {
                out_hh: out_hh.clone(),
                out_cc: out_cc.clone(),
                adapter_namespace: opts.adapter_namespace.clone(),
            }));
        }
        (None, None) => {}
        _ => bail!("--out-dbus-hh and --out-dbus-cc must be given together"),
    }

    let model = template_model::GenerationModel::build_with_dbus(
        &unit,
        opts.out_dbus_hh.is_some(),
    )?;

    let mut by_backend: Vec<(&'static str, Vec<backend::GeneratedFile>)> = Vec::new();
    for b in &active {
        let files = b
            .generate(&model, &header_include)
            .with_context(|| format!("generating {} output for {}", b.name(), opts.header.display()))?;
        for f in &files {
            write_if_changed(&f.path, &f.contents)?;
        }
        by_backend.push((b.name(), files));
    }

    // Translate the generic (backend name -> files) results back into the
    // named fields callers (this crate's CLI and tests) already expect.
    // GrpcBackend/DbusBackend each document their own fixed output order
    // (see their `generate()` bodies) — this is the one place that relies
    // on it, so a backend can't reorder its own outputs without updating
    // its match arm here too.
    let grpc_files = by_backend
        .iter()
        .find(|(name, _)| *name == "grpc")
        .map(|(_, files)| files)
        .expect("GrpcBackend is always active");
    let (proto, adapter_hh, adapter_cc, types_proto) = match &grpc_files[..] {
        [proto, adapter_hh, adapter_cc] => (proto, adapter_hh, adapter_cc, None),
        [proto, adapter_hh, adapter_cc, types_proto] => {
            (proto, adapter_hh, adapter_cc, Some(types_proto.path.clone()))
        }
        _ => bail!(
            "GrpcBackend produced {} file(s), expected exactly 3 or 4",
            grpc_files.len()
        ),
    };

    let (dbus_hh, dbus_cc) = match by_backend.iter().find(|(name, _)| *name == "dbus") {
        Some((_, files)) => match &files[..] {
            [hh, cc] => (Some(hh.path.clone()), Some(cc.path.clone())),
            _ => bail!("DbusBackend produced {} file(s), expected exactly 2", files.len()),
        },
        None => (None, None),
    };

    Ok(GeneratedFiles {
        proto: proto.path.clone(),
        types_proto,
        adapter_hh: adapter_hh.path.clone(),
        adapter_cc: adapter_cc.path.clone(),
        dbus_hh,
        dbus_cc,
    })
}

/// Skip the write if the content is unchanged, so downstream build systems
/// don't see a modified mtime (and re-trigger `protoc`/recompiles) on every
/// configure when nothing actually changed.
fn write_if_changed(path: &Path, contents: &str) -> Result<()> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).with_context(|| format!("creating {}", parent.display()))?;
    }
    if let Ok(existing) = fs::read_to_string(path) {
        if existing == contents {
            return Ok(());
        }
    }
    fs::write(path, contents).with_context(|| format!("writing {}", path.display()))
}
