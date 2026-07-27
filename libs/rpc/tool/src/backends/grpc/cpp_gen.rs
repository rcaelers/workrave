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
    service_cpp_ns: String,
    types_cpp_ns: String,
    split_proto_types: bool,
    proto_descriptor_symbol: String,
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

pub struct RenderAdapterOptions<'a> {
    pub package: &'a str,
    pub proto_types_package: Option<&'a str>,
    pub grpc_services_namespace: Option<&'a str>,
    pub adapter_namespace: Option<&'a str>,
    pub header_include: &'a str,
    pub proto_basename: &'a str,
    pub adapter_header_filename: &'a str,
}

pub fn render_adapter(
    model: &GenerationModel,
    options: RenderAdapterOptions<'_>,
) -> Result<RenderedAdapter> {
    let interface = &model.interfaces[0];
    let package_cpp_ns = options.package.replace('.', "::");
    let service_cpp_ns = options
        .grpc_services_namespace
        .map(|namespace| format!("{package_cpp_ns}::{namespace}"))
        .unwrap_or_else(|| package_cpp_ns.clone());
    let context = AdapterContext {
        service_cpp_ns,
        types_cpp_ns: options
            .proto_types_package
            .unwrap_or(options.package)
            .replace('.', "::"),
        split_proto_types: options.proto_types_package.is_some(),
        proto_descriptor_symbol: format!("descriptor_table_{}_2eproto", options.proto_basename),
        adapter_namespace: options.adapter_namespace,
        impl_class_name: format!("{}ServiceImpl", interface.service_name),
        header_include: options.header_include.to_string(),
        proto_basename: options.proto_basename.to_string(),
        adapter_header_filename: options.adapter_header_filename.to_string(),
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
