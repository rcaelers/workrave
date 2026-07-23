//! `clang-rpc-gen`: generates a gRPC C++ service adapter from an annotated
//! C++ header, using libclang to parse the header directly — no separate
//! IDL, no changes to the annotated code. See `README.md` for the tag
//! vocabulary (`@rpc(service=...)`, `@rpc(name=...)`, `@rpc.param(...)`).
//!
//! This crate is deliberately generic: it knows nothing about any specific
//! consumer project. All project-specific input (which header, which proto
//! package, where outputs go) comes in through [`GenerateOptions`].

pub mod annotations;
pub mod clang_index;
pub mod compile_db;
pub mod cpp_gen;
pub mod external_annotations;
pub mod ir;
pub mod proto_gen;

use std::fs;
use std::path::{Path, PathBuf};

use anyhow::{bail, Context, Result};

use external_annotations::ExternalAnnotations;

pub struct GenerateOptions {
    pub header: PathBuf,
    pub anchor_source: PathBuf,
    pub compile_commands: PathBuf,
    pub out_proto: PathBuf,
    pub out_adapter_hh: PathBuf,
    pub out_adapter_cc: PathBuf,
    pub proto_package: String,
    /// Literal text for the generated `#include "..."` of the annotated
    /// header. Defaults to the header's file name.
    pub header_include: Option<String>,
    /// A file supplying `@rpc` tags by fully-qualified name, for
    /// declarations that can't carry an annotation comment of their own
    /// (third-party/generated headers) — see `external_annotations`.
    pub external_annotations: Option<PathBuf>,
}

#[derive(Debug)]
pub struct GeneratedFiles {
    pub proto: PathBuf,
    pub adapter_hh: PathBuf,
    pub adapter_cc: PathBuf,
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
    let iface = &unit.interfaces[0];

    let header_include = opts.header_include.clone().unwrap_or_else(|| {
        opts.header
            .file_name()
            .expect("header has a file name")
            .to_string_lossy()
            .to_string()
    });

    let proto_basename = opts
        .out_proto
        .file_stem()
        .context("--out-proto has no file stem")?
        .to_string_lossy()
        .to_string();

    let adapter_header_filename = opts
        .out_adapter_hh
        .file_name()
        .context("--out-adapter-hh has no file name")?
        .to_string_lossy()
        .to_string();

    let proto_text = proto_gen::render_proto(&unit, &opts.proto_package)?;
    let adapter = cpp_gen::render_adapter(
        iface,
        &unit,
        &opts.proto_package,
        &header_include,
        &proto_basename,
        &adapter_header_filename,
    )?;

    write_if_changed(&opts.out_proto, &proto_text)?;
    write_if_changed(&opts.out_adapter_hh, &adapter.header)?;
    write_if_changed(&opts.out_adapter_cc, &adapter.source)?;

    Ok(GeneratedFiles {
        proto: opts.out_proto.clone(),
        adapter_hh: opts.out_adapter_hh.clone(),
        adapter_cc: opts.out_adapter_cc.clone(),
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
