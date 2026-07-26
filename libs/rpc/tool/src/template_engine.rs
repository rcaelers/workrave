//! Shared MiniJinja configuration for whole-artifact rendering.
//!
//! Production templates are added as their backends are migrated. Keeping the
//! configuration here gives every artifact the same strict undefined-value,
//! escaping, recursion, and trailing-newline behavior.

use anyhow::{Context, Result};
use minijinja::{AutoEscape, Environment, UndefinedBehavior};
use serde::Serialize;

const TEMPLATE_RECURSION_LIMIT: usize = 256;

pub(crate) fn new_environment() -> Environment<'static> {
    let mut environment = Environment::new();
    environment.set_auto_escape_callback(|_| AutoEscape::None);
    environment.set_undefined_behavior(UndefinedBehavior::Strict);
    environment.set_recursion_limit(TEMPLATE_RECURSION_LIMIT);
    environment.set_keep_trailing_newline(true);
    environment
}

#[derive(Clone, Copy)]
pub(crate) struct EmbeddedTemplate {
    pub name: &'static str,
    pub source: &'static str,
}

/// Renders one root artifact from a fixed set of embedded templates. Imported
/// macro/include templates participate in this same render; callers invoke
/// this function once, at the artifact boundary.
pub(crate) fn render_template<T: Serialize>(
    root_name: &str,
    templates: &[EmbeddedTemplate],
    model: &T,
) -> Result<String> {
    let mut environment = new_environment();
    for template in templates {
        environment
            .add_template(template.name, template.source)
            .with_context(|| format!("parsing embedded template '{}'", template.name))?;
    }
    environment
        .get_template(root_name)
        .with_context(|| format!("loading root template '{root_name}'"))?
        .render(model)
        .with_context(|| format!("rendering root template '{root_name}'"))
}

#[cfg(test)]
mod tests {
    use std::collections::BTreeMap;

    use minijinja::context;
    use serde::Serialize;

    use super::new_environment;

    #[derive(Serialize)]
    struct TypeModel<'a> {
        cxx_type: &'a str,
        #[serde(flatten)]
        shape: TypeShape<'a>,
    }

    #[derive(Serialize)]
    #[serde(tag = "kind", rename_all = "snake_case")]
    enum TypeShape<'a> {
        Scalar,
        Struct { fields: Vec<FieldModel<'a>> },
        Sequence { element_type: &'a str },
        Map { value_type: &'a str },
    }

    #[derive(Serialize)]
    struct FieldModel<'a> {
        cxx_name: &'a str,
        proto_name: &'a str,
        type_id: &'a str,
    }

    #[derive(Serialize)]
    struct Model<'a> {
        types: BTreeMap<&'a str, TypeModel<'a>>,
    }

    const CONVERSION_MACROS: &str =
        include_str!("templates/tests/recursive_conversion_macros.jinja");
    const ROOT_TEMPLATE: &str = include_str!("templates/tests/recursive_conversion.jinja");

    fn nested_model() -> Model<'static> {
        Model {
            types: BTreeMap::from([
                (
                    "int",
                    TypeModel {
                        cxx_type: "int",
                        shape: TypeShape::Scalar,
                    },
                ),
                (
                    "leaf",
                    TypeModel {
                        cxx_type: "Leaf",
                        shape: TypeShape::Struct {
                            fields: vec![FieldModel {
                                cxx_name: "value",
                                proto_name: "value",
                                type_id: "int",
                            }],
                        },
                    },
                ),
                (
                    "leaves",
                    TypeModel {
                        cxx_type: "std::vector<Leaf>",
                        shape: TypeShape::Sequence {
                            element_type: "leaf",
                        },
                    },
                ),
                (
                    "leaf_map",
                    TypeModel {
                        cxx_type: "std::map<std::string, Leaf>",
                        shape: TypeShape::Map { value_type: "leaf" },
                    },
                ),
                (
                    "root",
                    TypeModel {
                        cxx_type: "Root",
                        shape: TypeShape::Struct {
                            fields: vec![
                                FieldModel {
                                    cxx_name: "items",
                                    proto_name: "items",
                                    type_id: "leaves",
                                },
                                FieldModel {
                                    cxx_name: "by_name",
                                    proto_name: "by_name",
                                    type_id: "leaf_map",
                                },
                            ],
                        },
                    },
                ),
            ]),
        }
    }

    #[test]
    fn recursively_renders_nested_decode_and_encode_macros() {
        let mut environment = new_environment();
        environment
            .add_template("conversion_macros.jinja", CONVERSION_MACROS)
            .unwrap();
        environment
            .add_template("root.jinja", ROOT_TEMPLATE)
            .unwrap();

        let model = nested_model();
        let output = environment
            .get_template("root.jinja")
            .unwrap()
            .render(context!(model => &model))
            .unwrap();

        assert!(
            output.contains("for (const auto &rpc_wire_0 : request->root().items())"),
            "{output}"
        );
        assert!(
            output.contains("rpc_item_0.value = rpc_wire_0.value();"),
            "{output}"
        );
        assert!(
            output.contains("local_root.items.push_back(rpc_item_0);"),
            "{output}"
        );
        assert!(
            output.contains("for (const auto &rpc_kv_0 : request->root().by_name())"),
            "{output}"
        );
        assert!(
            output.contains("rpc_value_0.value = rpc_kv_0.second.value();"),
            "{output}"
        );
        assert!(
            output.contains("local_root.by_name.emplace(rpc_kv_0.first, rpc_value_0);"),
            "{output}"
        );
        assert!(
            output.contains("auto *rpc_msg_0 = response->mutable_root();"),
            "{output}"
        );
        assert!(
            output.contains("for (const auto &rpc_item_1 : local_root.items)"),
            "{output}"
        );
        assert!(
            output.contains("auto *rpc_wire_1 = rpc_msg_0->add_items();"),
            "{output}"
        );
        assert!(
            output.contains("rpc_wire_1->set_value(rpc_item_1.value);"),
            "{output}"
        );
        assert!(
            output.contains("for (const auto &rpc_kv_1 : local_root.by_name)"),
            "{output}"
        );
        assert!(
            output.contains("auto &rpc_wire_1 = (*rpc_msg_0->mutable_by_name())[rpc_kv_1.first];"),
            "{output}"
        );
        assert!(
            output.contains("rpc_wire_1.set_value(rpc_kv_1.second.value);"),
            "{output}"
        );
        assert!(
            output.ends_with('\n'),
            "trailing newline was not preserved: {output:?}"
        );
    }

    #[test]
    fn strict_undefined_values_report_the_template_location() {
        let mut environment = new_environment();
        environment
            .add_template("strict.jinja", "before {{ missing.value }} after\n")
            .unwrap();

        let error = environment
            .get_template("strict.jinja")
            .unwrap()
            .render(())
            .expect_err("strict undefined values must fail");
        let rendered = format!("{error:#}");
        assert!(rendered.contains("strict.jinja"), "{rendered}");
        assert!(rendered.contains("undefined value"), "{rendered}");
    }

    #[test]
    fn code_generation_templates_do_not_auto_escape() {
        let mut environment = new_environment();
        environment
            .add_template("code.cc.jinja", "{{ expression }}\n")
            .unwrap();

        let output = environment
            .get_template("code.cc.jinja")
            .unwrap()
            .render(context!(expression => "left < right && value > 0"))
            .unwrap();
        assert_eq!(output, "left < right && value > 0\n");
    }
}
