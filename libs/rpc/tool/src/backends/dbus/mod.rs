//! Native DBus targets: render introspection XML plus QtDBus or GDBus
//! dispatch/marshalling C++ for an `@rpc.dbus(interface="...")` interface.
//! Opt-in — `lib.rs::generate()` constructs the selected backend only when
//! both `--out-dbus-hh`/`--out-dbus-cc` are given, but once constructed, a
//! missing `@rpc.dbus` tag on the interface is still a hard error (from
//! `dbus_gen::render_dbus_binding`) rather than a silent no-op — the caller
//! asked for DBus output, so an interface that can't provide it is a real
//! mistake to report, not something to paper over.

mod dbus_gen;

use std::path::PathBuf;

use anyhow::{Context, Result};

use crate::backend::{Backend, GeneratedFile};
use crate::template_model::GenerationModel;

pub struct QtDbusBackend {
    pub out_hh: PathBuf,
    pub out_cc: PathBuf,
    pub header_filename: Option<String>,
    /// Optional C++ namespace for the generated binding class and init
    /// function. Marshalling helpers retain their ADL-sensitive namespaces.
    pub adapter_namespace: Option<String>,
}

pub struct GioDbusBackend {
    pub out_hh: PathBuf,
    pub out_cc: PathBuf,
    pub header_filename: Option<String>,
    pub adapter_namespace: Option<String>,
}

impl Backend for QtDbusBackend {
    fn name(&self) -> &'static str {
        "dbus-qt"
    }

    fn generate(
        &self,
        model: &GenerationModel,
        header_include: &str,
    ) -> Result<Vec<GeneratedFile>> {
        let dbus_header_filename = self.header_filename.clone().unwrap_or(
            self.out_hh
                .file_name()
                .context("--out-dbus-hh has no file name")?
                .to_string_lossy()
                .to_string(),
        );

        let binding = dbus_gen::render_dbus_binding(
            model,
            self.adapter_namespace.as_deref(),
            header_include,
            &dbus_header_filename,
        )?;

        Ok(vec![
            GeneratedFile {
                path: self.out_hh.clone(),
                contents: binding.header,
            },
            GeneratedFile {
                path: self.out_cc.clone(),
                contents: binding.source,
            },
        ])
    }
}

impl Backend for GioDbusBackend {
    fn name(&self) -> &'static str {
        "dbus-gio"
    }

    fn generate(
        &self,
        model: &GenerationModel,
        header_include: &str,
    ) -> Result<Vec<GeneratedFile>> {
        let dbus_header_filename = self.header_filename.clone().unwrap_or(
            self.out_hh
                .file_name()
                .context("--out-dbus-hh has no file name")?
                .to_string_lossy()
                .to_string(),
        );

        let binding = dbus_gen::render_gio_dbus_binding(
            model,
            self.adapter_namespace.as_deref(),
            header_include,
            &dbus_header_filename,
        )?;

        Ok(vec![
            GeneratedFile {
                path: self.out_hh.clone(),
                contents: binding.header,
            },
            GeneratedFile {
                path: self.out_cc.clone(),
                contents: binding.source,
            },
        ])
    }
}
