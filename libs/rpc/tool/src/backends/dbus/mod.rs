//! The DBus target: renders introspection XML plus QtDBus dispatch/
//! marshalling C++ for an `@rpc.dbus(interface="...")`-tagged interface.
//! Opt-in — `lib.rs::generate()` only constructs a `DbusBackend` when both
//! `--out-dbus-hh`/`--out-dbus-cc` are given, but once constructed, a
//! missing `@rpc.dbus` tag on the interface is still a hard error (from
//! `dbus_gen::render_dbus_binding`) rather than a silent no-op — the caller
//! asked for DBus output, so an interface that can't provide it is a real
//! mistake to report, not something to paper over.

mod dbus_gen;

use std::path::PathBuf;

use anyhow::{Context, Result};

use crate::backend::{Backend, GeneratedFile};
use crate::template_model::GenerationModel;

pub struct DbusBackend {
    pub out_hh: PathBuf,
    pub out_cc: PathBuf,
}

impl Backend for DbusBackend {
    fn name(&self) -> &'static str {
        "dbus"
    }

    fn generate(
        &self,
        model: &GenerationModel,
        header_include: &str,
    ) -> Result<Vec<GeneratedFile>> {
        let dbus_header_filename = self
            .out_hh
            .file_name()
            .context("--out-dbus-hh has no file name")?
            .to_string_lossy()
            .to_string();

        let binding =
            dbus_gen::render_dbus_binding(model, header_include, &dbus_header_filename)?;

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
