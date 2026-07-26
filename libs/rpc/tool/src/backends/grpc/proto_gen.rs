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

#[derive(Serialize)]
struct ProtoContext<'a> {
    package: String,
    model: &'a GenerationModel,
}

pub fn render_proto(model: &GenerationModel, package: &str) -> Result<String> {
    let context = ProtoContext {
        package: package.to_string(),
        model,
    };
    render_template(ROOT_TEMPLATE_NAME, &[ROOT_TEMPLATE], &context)
}
