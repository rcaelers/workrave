//! Renders the `.proto` schema for a [`Unit`]. This file is a disposable
//! build artifact — it is never hand-edited, only fed to `protoc` +
//! `grpc_cpp_plugin` to produce the real message/service C++ classes.

use anyhow::Result;
use askama::Template;

use crate::ir::Unit;

struct ProtoFieldView {
    number: usize,
    proto_type: String,
    field_name: String,
}

struct ProtoMessageView {
    name: String,
    fields: Vec<ProtoFieldView>,
}

struct ProtoMethodView {
    rpc_name: String,
    request: ProtoMessageView,
    response: ProtoMessageView,
}

struct ProtoEnumValueView {
    name: String,
    number: usize,
    /// From `@rpc.enum.value(name="...")`, if any — not used by the gRPC
    /// backend itself, surfaced as a comment so a future wire backend (e.g.
    /// DBus) has a documented, exact wire name to reproduce without
    /// re-parsing the C++ header.
    canonical_name: Option<String>,
}

struct ProtoEnumView {
    name: String,
    /// From `@rpc.enum(name="...")`, if any — same rationale as
    /// `ProtoEnumValueView::canonical_name`.
    canonical_name: Option<String>,
    values: Vec<ProtoEnumValueView>,
}

struct ProtoSignalView {
    rpc_name: String,
    request: ProtoMessageView,
    event: ProtoMessageView,
}

struct ProtoServiceView {
    name: String,
    methods: Vec<ProtoMethodView>,
    signals: Vec<ProtoSignalView>,
}

#[derive(Template)]
#[template(path = "service.proto.askama", escape = "none")]
struct ProtoTemplate {
    package: String,
    enums: Vec<ProtoEnumView>,
    /// Plain structs/classes auto-discovered via `Unit::structs` (see
    /// `register_struct` in clang_index.rs) — rendered as top-level
    /// messages, ahead of any service that references them.
    structs: Vec<ProtoMessageView>,
    services: Vec<ProtoServiceView>,
}

pub fn render_proto(unit: &Unit, package: &str) -> Result<String> {
    let enums = unit
        .enums
        .iter()
        .map(|e| ProtoEnumView {
            name: e.proto_name.clone(),
            canonical_name: e.canonical_name.clone(),
            values: e
                .values
                .iter()
                .enumerate()
                .map(|(i, v)| ProtoEnumValueView {
                    name: v.proto_name.clone(),
                    number: i,
                    canonical_name: v.canonical_name.clone(),
                })
                .collect(),
        })
        .collect();

    let structs = unit
        .structs
        .iter()
        .map(|s| ProtoMessageView {
            name: s.proto_name.clone(),
            fields: s
                .fields
                .iter()
                .enumerate()
                .map(|(i, f)| ProtoFieldView {
                    number: i + 1,
                    proto_type: f.proto_type.to_string(),
                    field_name: f.proto_field.clone(),
                })
                .collect(),
        })
        .collect();

    let services = unit
        .interfaces
        .iter()
        .map(|iface| ProtoServiceView {
            name: iface.service_name.clone(),
            methods: iface
                .methods
                .iter()
                .map(|m| method_view(m, iface.keyed_by.as_ref()))
                .collect(),
            signals: iface
                .signals
                .iter()
                .map(|s| signal_view(s, iface.keyed_by.as_ref()))
                .collect(),
        })
        .collect();

    let tmpl = ProtoTemplate {
        package: package.to_string(),
        enums,
        structs,
        services,
    };
    Ok(tmpl.render()?)
}

fn method_view(m: &crate::ir::Method, keyed_by: Option<&crate::ir::KeyType>) -> ProtoMethodView {
    // The instance-selector field always comes first (field 1), the gRPC
    // analog of a DBus object path — see Interface::keyed_by.
    let mut request_fields = Vec::new();
    if let Some(key) = keyed_by {
        request_fields.push(ProtoFieldView {
            number: 1,
            proto_type: key.proto_type.to_string(),
            field_name: "id".to_string(),
        });
    }
    let base = request_fields.len();
    request_fields.extend(m.request_fields().into_iter().enumerate().map(|(i, f)| {
        ProtoFieldView {
            number: base + i + 1,
            proto_type: f.proto_type.to_string(),
            field_name: f.proto_field.clone(),
        }
    }));

    let mut response_fields: Vec<ProtoFieldView> = m
        .response_fields()
        .into_iter()
        .enumerate()
        .map(|(i, f)| ProtoFieldView {
            number: i + 1,
            proto_type: f.proto_type.to_string(),
            field_name: f.proto_field.clone(),
        })
        .collect();
    if let Some(rv) = &m.return_value {
        response_fields.push(ProtoFieldView {
            number: response_fields.len() + 1,
            proto_type: rv.proto_type.to_string(),
            field_name: rv.proto_field.clone(),
        });
    }

    ProtoMethodView {
        rpc_name: m.rpc_name.clone(),
        request: ProtoMessageView {
            name: format!("{}Request", m.rpc_name),
            fields: request_fields,
        },
        response: ProtoMessageView {
            name: format!("{}Response", m.rpc_name),
            fields: response_fields,
        },
    }
}

fn signal_view(s: &crate::ir::Signal, keyed_by: Option<&crate::ir::KeyType>) -> ProtoSignalView {
    // Same "id selects the instance" convention as method_view, applied to
    // the subscribe request instead of a unary request.
    let mut request_fields = Vec::new();
    if let Some(key) = keyed_by {
        request_fields.push(ProtoFieldView {
            number: 1,
            proto_type: key.proto_type.to_string(),
            field_name: "id".to_string(),
        });
    }

    let event_fields = s
        .fields
        .iter()
        .enumerate()
        .map(|(i, f)| ProtoFieldView {
            number: i + 1,
            proto_type: f.proto_type.to_string(),
            field_name: f.proto_field.clone(),
        })
        .collect();

    ProtoSignalView {
        rpc_name: s.rpc_name.clone(),
        request: ProtoMessageView {
            name: format!("{}Request", s.rpc_name),
            fields: request_fields,
        },
        event: ProtoMessageView {
            name: format!("{}Event", s.rpc_name),
            fields: event_fields,
        },
    }
}
