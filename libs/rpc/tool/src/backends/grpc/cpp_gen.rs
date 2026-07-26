//! Renders the gRPC C++ adapter from the driver's shared semantic model.
//!
//! Rust resolves and validates the API before rendering. The whole-file
//! templates own iteration over methods, parameters, and signals; recursive
//! MiniJinja macros own wire conversion for structs, sequences, and maps.

use anyhow::Result;
use serde::Serialize;

use crate::template_engine::{render_template, EmbeddedTemplate};
use crate::template_model::GenerationModel;

const HEADER_TEMPLATE_NAME: &str = "grpc/service_impl.hh.jinja";
const HEADER_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: HEADER_TEMPLATE_NAME,
    source: include_str!("templates/service_impl.hh.jinja"),
};

const SOURCE_TEMPLATE_NAME: &str = "grpc/service_impl.cc.jinja";
const SOURCE_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: SOURCE_TEMPLATE_NAME,
    source: include_str!("templates/service_impl.cc.jinja"),
};

const SOURCE_MACROS: EmbeddedTemplate = EmbeddedTemplate {
    name: "grpc/service_impl_macros.jinja",
    source: include_str!("templates/service_impl_macros.jinja"),
};

#[derive(Serialize)]
struct AdapterContext<'a> {
    proto_cpp_ns: String,
    adapter_namespace: Option<&'a str>,
    impl_class_name: String,
    header_include: String,
    proto_basename: String,
    adapter_header_filename: String,
    has_duration: bool,
    model: &'a GenerationModel,
}

pub struct RenderedAdapter {
    pub header: String,
    pub source: String,
}

pub fn render_adapter(
    model: &GenerationModel,
    package: &str,
    adapter_namespace: Option<&str>,
    header_include: &str,
    proto_basename: &str,
    adapter_header_filename: &str,
) -> Result<RenderedAdapter> {
    let interface = &model.interfaces[0];
    let context = AdapterContext {
        proto_cpp_ns: package.replace('.', "::"),
        adapter_namespace,
        impl_class_name: format!("{}ServiceImpl", interface.service_name),
        header_include: header_include.to_string(),
        proto_basename: proto_basename.to_string(),
        adapter_header_filename: adapter_header_filename.to_string(),
        has_duration: interface.has_duration,
        model,
    };

    let header = render_template(HEADER_TEMPLATE_NAME, &[HEADER_TEMPLATE], &context)?;
    let source = render_template(
        SOURCE_TEMPLATE_NAME,
        &[SOURCE_TEMPLATE, SOURCE_MACROS],
        &context,
    )?;

    Ok(RenderedAdapter { header, source })
}
