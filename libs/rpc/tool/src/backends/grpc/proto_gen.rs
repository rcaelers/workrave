//! Renders the disposable `.proto` schema from the shared semantic template
//! model. The template owns all traversal and protobuf syntax; Rust performs
//! one render at the artifact boundary.

use anyhow::Result;
use serde::Serialize;

use crate::template_engine::{render_template, EmbeddedTemplate};
use crate::template_model::GenerationModel;

const ROOT_TEMPLATE_NAME: &str = "grpc/service.proto.jinja";
const ROOT_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: ROOT_TEMPLATE_NAME,
    source: include_str!("templates/service.proto.jinja"),
};

const TYPES_TEMPLATE_NAME: &str = "grpc/types.proto.jinja";
const TYPES_TEMPLATE: EmbeddedTemplate = EmbeddedTemplate {
    name: TYPES_TEMPLATE_NAME,
    source: include_str!("templates/types.proto.jinja"),
};

#[derive(Serialize)]
struct ProtoContext<'a> {
    package: String,
    types_package: Option<&'a str>,
    types_proto_filename: Option<&'a str>,
    model: &'a GenerationModel,
}

pub fn render_service_proto(
    model: &GenerationModel,
    package: &str,
    types_package: Option<&str>,
    types_proto_filename: Option<&str>,
) -> Result<String> {
    let context = ProtoContext {
        package: package.to_string(),
        types_package,
        types_proto_filename,
        model,
    };
    render_template(ROOT_TEMPLATE_NAME, &[ROOT_TEMPLATE], &context)
}

#[derive(Serialize)]
struct TypesContext<'a> {
    package: String,
    model: &'a GenerationModel,
}

pub fn render_types_proto(model: &GenerationModel, package: &str) -> Result<String> {
    let context = TypesContext {
        package: package.to_string(),
        model,
    };
    render_template(TYPES_TEMPLATE_NAME, &[TYPES_TEMPLATE], &context)
}
