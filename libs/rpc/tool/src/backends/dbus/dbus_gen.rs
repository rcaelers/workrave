//! Whole-artifact MiniJinja rendering for the QtDBus and GDBus bindings.
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

const GIO_HEADER_TEMPLATE_NAME: &str = "dbus/gio_binding.hh.jinja";
const GIO_HEADER_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: GIO_HEADER_TEMPLATE_NAME,
    source: include_str!("templates/gio_binding.hh.jinja"),
};

const GIO_SOURCE_TEMPLATE_NAME: &str = "dbus/gio_binding.cc.jinja";
const GIO_SOURCE_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: GIO_SOURCE_TEMPLATE_NAME,
    source: include_str!("templates/gio_binding.cc.jinja"),
};

const GIO_MARSHALLING_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: "dbus/gio_marshalling.jinja",
    source: include_str!("templates/gio_marshalling.jinja"),
};

#[derive(Serialize)]
struct DbusArtifactContext<'a> {
    adapter_namespace: Option<String>,
    header_include: String,
    dbus_header_filename: String,
    model: &'a GenerationModel,
}

pub struct RenderedDBusBinding {
    pub header: String,
    pub source: String,
}

fn normalize_final_newline(text: String) -> String {
    let mut result = text.trim_end().to_string();
    result.push('\n');
    result
}

pub fn render_dbus_binding(
    model: &GenerationModel,
    adapter_namespace: Option<&str>,
    header_include: &str,
    dbus_header_filename: &str,
) -> Result<RenderedDBusBinding> {
    let context = DbusArtifactContext {
        adapter_namespace: adapter_namespace.map(str::to_string),
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
    Ok(RenderedDBusBinding {
        header: normalize_final_newline(header),
        source: normalize_final_newline(source),
    })
}

pub fn render_gio_dbus_binding(
    model: &GenerationModel,
    adapter_namespace: Option<&str>,
    header_include: &str,
    dbus_header_filename: &str,
) -> Result<RenderedDBusBinding> {
    let context = DbusArtifactContext {
        adapter_namespace: adapter_namespace.map(str::to_string),
        header_include: header_include.to_string(),
        dbus_header_filename: dbus_header_filename.to_string(),
        model,
    };
    context.model.interfaces[0]
        .dbus
        .as_ref()
        .context("interface has no @rpc.dbus(interface=\"...\") tag")?;

    let header = render_template(GIO_HEADER_TEMPLATE_NAME, &[GIO_HEADER_TEMPLATE], &context)?;
    let source = render_template(
        GIO_SOURCE_TEMPLATE_NAME,
        &[GIO_SOURCE_TEMPLATE, GIO_MARSHALLING_TEMPLATE],
        &context,
    )?;
    Ok(RenderedDBusBinding {
        header: normalize_final_newline(header),
        source: normalize_final_newline(source),
    })
}
