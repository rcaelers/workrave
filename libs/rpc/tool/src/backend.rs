//! The generic contract every codegen target (gRPC, DBus, ...) implements.
//! `generate()` in `lib.rs` is a plain, target-agnostic driver: it parses
//! the annotated header once into a [`crate::ir::Unit`], builds whichever
//! [`Backend`]s were requested (`backends::grpc::GrpcBackend` always,
//! `backends::dbus::DbusBackend` when `--out-dbus-hh`/`--out-dbus-cc` are
//! given), and asks each one for its output files — it has no
//! target-specific knowledge of its own. Adding a third target later means
//! adding a `backends/<name>/` directory implementing this trait, not
//! touching the driver.

use std::path::PathBuf;

use anyhow::Result;

use crate::template_model::GenerationModel;

/// One file this backend wants written, relative to nothing in particular —
/// the path is always supplied by whoever constructed the `Backend`
/// (ultimately from CLI flags), never invented by the backend itself.
pub(crate) struct GeneratedFile {
    pub path: PathBuf,
    pub contents: String,
}

pub(crate) trait Backend {
    /// Short, stable identifier for logging and error messages, e.g.
    /// "grpc", "dbus" — not used for any file naming.
    fn name(&self) -> &'static str;

    /// Renders this backend's output from the single immutable semantic model
    /// built by the driver (`clang-rpc-gen` is one-header-one-interface in v1;
    /// see `lib.rs::generate`). `header_include` is the already-resolved literal
    /// text for the generated `#include "..."` of the original annotated
    /// header, shared by every backend so they don't each re-derive it.
    fn generate(
        &self,
        model: &GenerationModel,
        header_include: &str,
    ) -> Result<Vec<GeneratedFile>>;
}
