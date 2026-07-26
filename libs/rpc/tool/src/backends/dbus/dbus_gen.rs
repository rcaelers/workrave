//! Whole-artifact MiniJinja rendering for the QtDBus binding.
//!
//! The shared template model contains DBus signatures, argument positions,
//! and ordered custom-type metadata. Templates own the generated C++ and the
//! embedded introspection XML; Rust performs no fragment rendering.

use anyhow::{Context, Result};
use serde::Serialize;

use crate::template_engine::{render_template, EmbeddedTemplate};
use crate::template_model::GenerationModel;

const HEADER_TEMPLATE_NAME: &str = "dbus/dbus_binding.hh.jinja";
const HEADER_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: HEADER_TEMPLATE_NAME,
    source: include_str!("templates/dbus_binding.hh.jinja"),
};

const SOURCE_TEMPLATE_NAME: &str = "dbus/dbus_binding.cc.jinja";
const SOURCE_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: SOURCE_TEMPLATE_NAME,
    source: include_str!("templates/dbus_binding.cc.jinja"),
};

const MARSHALLING_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: "dbus/dbus_marshalling.jinja",
    source: include_str!("templates/dbus_marshalling.jinja"),
};

#[derive(Serialize)]
struct DbusArtifactContext<'a> {
    header_include: String,
    dbus_header_filename: String,
    model: &'a GenerationModel,
}

pub struct RenderedDBusBinding {
    pub header: String,
    pub source: String,
}

pub fn render_dbus_binding(
    model: &GenerationModel,
    header_include: &str,
    dbus_header_filename: &str,
) -> Result<RenderedDBusBinding> {
    let context = DbusArtifactContext {
        header_include: header_include.to_string(),
        dbus_header_filename: dbus_header_filename.to_string(),
        model,
    };
    context.model.interfaces[0]
        .dbus
        .as_ref()
        .context("interface has no @rpc.dbus(interface=\"...\") tag")?;

    let header = render_template(HEADER_TEMPLATE_NAME, &[HEADER_TEMPLATE], &context)?;
    let source = render_template(
        SOURCE_TEMPLATE_NAME,
        &[SOURCE_TEMPLATE, MARSHALLING_TEMPLATE],
        &context,
    )?;
    Ok(RenderedDBusBinding { header, source })
}
