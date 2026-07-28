//! Owned, serializable semantic model exposed to whole-file templates.
//!
//! This is deliberately distinct from both clang's AST and the parser IR.
//! It resolves cross-references, wire-field membership, numbering, and type
//! shapes before rendering, while containing no generated source fragments.

use std::collections::{BTreeMap, HashMap, HashSet};

use anyhow::{bail, Context, Result};
use serde::Serialize;

use crate::ir::{
    CxxType, DbusType, Direction, Interface, ParamKind, ProtoType, SequenceElement, Signal,
    StructDef, Unit,
};

pub(crate) type TemplateTypeId = String;

#[derive(Debug, Serialize)]
pub(crate) struct GenerationModel {
    pub enums: Vec<EnumModel>,
    pub structs: Vec<StructModel>,
    pub interfaces: Vec<InterfaceModel>,
    pub types: BTreeMap<TemplateTypeId, TypeModel>,
}

#[derive(Debug, Serialize)]
pub(crate) struct EnumModel {
    pub proto_name: String,
    pub cxx_symbol: String,
    pub canonical_name: Option<String>,
    pub values: Vec<EnumValueModel>,
}

#[derive(Debug, Serialize)]
pub(crate) struct EnumValueModel {
    pub proto_name: String,
    pub cxx_symbol: String,
    pub canonical_name: Option<String>,
    pub number: usize,
}

#[derive(Debug, Serialize)]
pub(crate) struct StructModel {
    pub proto_name: String,
    pub cxx_symbol: String,
    pub fields: Vec<StructFieldModel>,
}

#[derive(Debug, Serialize)]
pub(crate) struct StructFieldModel {
    pub cxx_name: String,
    pub proto_name: String,
    pub number: usize,
    pub type_id: TemplateTypeId,
}

#[derive(Debug, Serialize)]
pub(crate) struct InterfaceModel {
    pub proto_package: String,
    pub service_name: String,
    pub cxx_class: String,
    pub cxx_qualified_class: String,
    pub cxx_namespace: Vec<String>,
    pub has_duration: bool,
    pub keyed_by: Option<KeyModel>,
    pub dbus: Option<DbusInterfaceModel>,
    pub methods: Vec<MethodModel>,
    pub signals: Vec<SignalModel>,
}

#[derive(Debug, Serialize)]
pub(crate) struct KeyModel {
    pub type_id: TemplateTypeId,
}

#[derive(Debug, Serialize)]
pub(crate) struct MethodModel {
    pub rpc_name: String,
    pub cxx_symbol: String,
    pub is_const: bool,
    pub params: Vec<ParamModel>,
    pub request_fields: Vec<WireFieldModel>,
    pub response_fields: Vec<WireFieldModel>,
    pub return_value: Option<ReturnModel>,
    pub dbus_num_in: usize,
}

#[derive(Debug, Serialize)]
pub(crate) struct ParamModel {
    pub cxx_name: String,
    pub type_id: TemplateTypeId,
    pub direction: DirectionModel,
    pub proto_field: Option<String>,
    pub role: ParamRoleModel,
    pub dbus_arg_index: Option<usize>,
    pub dbus_cpp_type: Option<String>,
    pub dbus_signature: Option<String>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "snake_case")]
pub(crate) enum DirectionModel {
    In,
    Out,
    InOut,
}

impl From<Direction> for DirectionModel {
    fn from(value: Direction) -> Self {
        match value {
            Direction::In => Self::In,
            Direction::Out => Self::Out,
            Direction::InOut => Self::InOut,
        }
    }
}

#[derive(Debug, Serialize)]
#[serde(tag = "role", rename_all = "snake_case")]
pub(crate) enum ParamRoleModel {
    Value,
    AbsorbedSize { owner_field: String },
}

#[derive(Debug, Serialize)]
pub(crate) struct ReturnModel {
    pub proto_field: String,
    pub type_id: TemplateTypeId,
    pub dbus_cpp_type: Option<String>,
    pub dbus_signature: Option<String>,
}

#[derive(Debug, Serialize)]
pub(crate) struct WireFieldModel {
    pub proto_name: String,
    pub number: usize,
    pub type_id: TemplateTypeId,
}

#[derive(Debug, Serialize)]
pub(crate) struct SignalModel {
    pub rpc_name: String,
    pub cxx_symbol: String,
    pub request_fields: Vec<WireFieldModel>,
    pub event_fields: Vec<SignalFieldModel>,
}

#[derive(Debug, Serialize)]
pub(crate) struct SignalFieldModel {
    pub cxx_name: String,
    pub proto_name: String,
    pub number: usize,
    pub type_id: TemplateTypeId,
    pub dbus_cpp_type: Option<String>,
    pub dbus_signature: Option<String>,
}

#[derive(Debug, Serialize)]
pub(crate) struct TypeModel {
    pub id: TemplateTypeId,
    pub cxx: CxxTypeModel,
    pub grpc: GrpcTypeModel,
    pub dbus_signature: String,
    pub shape: TypeShapeModel,
}

#[derive(Debug, Serialize)]
pub(crate) struct DbusInterfaceModel {
    pub name: String,
    pub has_duration: bool,
    pub custom_types: Vec<DbusCustomTypeModel>,
}

#[derive(Debug, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub(crate) enum DbusCustomTypeModel {
    Enum {
        type_id: TemplateTypeId,
        values: Vec<DbusEnumValueModel>,
    },
    Struct {
        type_id: TemplateTypeId,
    },
    Vector {
        type_id: TemplateTypeId,
        element_type: TemplateTypeId,
    },
    Duration {
        type_id: TemplateTypeId,
    },
    Bitmask {
        type_id: TemplateTypeId,
        enum_type: TemplateTypeId,
    },
}

#[derive(Debug, Serialize)]
pub(crate) struct DbusEnumValueModel {
    pub cxx_symbol: String,
    pub canonical_name: String,
}

#[derive(Debug, Serialize)]
pub(crate) struct CxxTypeModel {
    pub spelling: String,
    pub base_spelling: String,
    pub is_pointer: bool,
    pub is_reference: bool,
    pub is_const: bool,
}

impl From<&CxxType> for CxxTypeModel {
    fn from(value: &CxxType) -> Self {
        Self {
            spelling: value.spelling.clone(),
            base_spelling: value.base_spelling.clone(),
            is_pointer: value.is_pointer,
            is_reference: value.is_ref,
            is_const: value.is_const,
        }
    }
}

#[derive(Debug, Serialize)]
pub(crate) struct GrpcTypeModel {
    /// Complete protobuf field type spelling, for example `int32`,
    /// `repeated MenuItem`, or `map<string, int32>`.
    pub spelling: String,
}

#[derive(Debug, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub(crate) enum TypeShapeModel {
    Scalar,
    Enum {
        proto_name: String,
    },
    Struct {
        proto_name: String,
        fields: Vec<TypeFieldModel>,
    },
    Sequence {
        element_type: TemplateTypeId,
    },
    Map {
        key_type: TemplateTypeId,
        value_type: TemplateTypeId,
    },
    Duration,
    Bitmask {
        enum_type: TemplateTypeId,
        enum_cxx_type: String,
    },
    CString,
    Bytes {
        size_param: String,
    },
}

#[derive(Debug, Serialize)]
pub(crate) struct TypeFieldModel {
    pub cxx_name: String,
    pub proto_name: String,
    pub type_id: TemplateTypeId,
}

impl GenerationModel {
    #[cfg(test)]
    pub(crate) fn build(unit: &Unit) -> Result<Self> {
        Self::build_with_dbus(unit, true)
    }

    pub(crate) fn build_with_dbus(unit: &Unit, include_dbus: bool) -> Result<Self> {
        let mut builder = ModelBuilder::new(unit, include_dbus);

        let enums = unit
            .enums
            .iter()
            .map(|definition| EnumModel {
                proto_name: definition.proto_name.clone(),
                cxx_symbol: definition.cxx_symbol.clone(),
                canonical_name: definition.canonical_name.clone(),
                values: definition
                    .values
                    .iter()
                    .enumerate()
                    .map(|(number, value)| EnumValueModel {
                        proto_name: value.proto_name.clone(),
                        cxx_symbol: value.cxx_symbol.clone(),
                        canonical_name: value.canonical_name.clone(),
                        number,
                    })
                    .collect(),
            })
            .collect();

        let mut structs = Vec::new();
        for definition in &unit.structs {
            structs.push(builder.build_struct(definition)?);
        }

        let mut interfaces = Vec::new();
        for interface in &unit.interfaces {
            interfaces.push(builder.build_interface(interface)?);
        }

        Ok(Self {
            enums,
            structs,
            interfaces,
            types: builder.types,
        })
    }
}

struct ModelBuilder<'a> {
    unit: &'a Unit,
    include_dbus: bool,
    next_type_id: usize,
    types: BTreeMap<TemplateTypeId, TypeModel>,
    type_ids_by_key: HashMap<String, TemplateTypeId>,
    active_type_keys: HashSet<String>,
}

impl<'a> ModelBuilder<'a> {
    fn dbus_type_override(
        dbus_type: DbusType,
        native_type: &ProtoType,
        native_cxx_type: &str,
    ) -> Result<(Option<String>, String)> {
        let numeric = matches!(
            native_type,
            ProtoType::Bool
                | ProtoType::Int32
                | ProtoType::Int64
                | ProtoType::UInt32
                | ProtoType::UInt64
                | ProtoType::Float
                | ProtoType::Double
                | ProtoType::Enum(_)
        );
        let integer = matches!(
            native_type,
            ProtoType::Int32
                | ProtoType::Int64
                | ProtoType::UInt32
                | ProtoType::UInt64
                | ProtoType::Enum(_)
        );

        let (cxx, signature, compatible) = match dbus_type {
            DbusType::Byte => (Some("uint8_t".to_string()), "y", numeric),
            DbusType::Boolean => (Some("bool".to_string()), "b", numeric),
            DbusType::Int16 => (Some("int16_t".to_string()), "n", numeric),
            DbusType::UInt16 => (Some("uint16_t".to_string()), "q", numeric),
            DbusType::Int32 => (Some("int32_t".to_string()), "i", numeric),
            DbusType::UInt32 => (Some("uint32_t".to_string()), "u", numeric),
            DbusType::Int64 => (Some("int64_t".to_string()), "x", numeric),
            DbusType::UInt64 => (Some("uint64_t".to_string()), "t", numeric),
            DbusType::Double => (Some("double".to_string()), "d", numeric),
            DbusType::String => (
                None,
                "s",
                matches!(native_type, ProtoType::String | ProtoType::Enum(_)),
            ),
            DbusType::ObjectPath => (
                Some("::workrave::rpc::dbus::ObjectPath".to_string()),
                "o",
                matches!(native_type, ProtoType::String),
            ),
            DbusType::Signature => (
                Some("::workrave::rpc::dbus::Signature".to_string()),
                "g",
                matches!(native_type, ProtoType::String),
            ),
            DbusType::UnixFd => (
                Some("::workrave::rpc::dbus::UnixFd".to_string()),
                "h",
                integer,
            ),
            DbusType::Variant => (
                Some(format!("::workrave::rpc::dbus::Variant<{native_cxx_type}>")),
                "v",
                true,
            ),
        };
        if !compatible {
            bail!("DBus type '{signature}' is incompatible with native wire type '{native_type}'");
        }
        Ok((cxx, signature.to_string()))
    }

    fn new(unit: &'a Unit, include_dbus: bool) -> Self {
        Self {
            unit,
            include_dbus,
            next_type_id: 0,
            types: BTreeMap::new(),
            type_ids_by_key: HashMap::new(),
            active_type_keys: HashSet::new(),
        }
    }

    fn build_struct(&mut self, definition: &StructDef) -> Result<StructModel> {
        let mut fields = Vec::new();
        for (index, field) in definition.fields.iter().enumerate() {
            fields.push(StructFieldModel {
                cxx_name: field.cxx_name.clone(),
                proto_name: field.proto_field.clone(),
                number: index + 1,
                type_id: self.register_type(&field.cxx_type, &field.proto_type, &field.kind)?,
            });
        }
        Ok(StructModel {
            proto_name: definition.proto_name.clone(),
            cxx_symbol: definition.cxx_symbol.clone(),
            fields,
        })
    }

    fn build_interface(&mut self, interface: &Interface) -> Result<InterfaceModel> {
        let dbus_enabled = self.include_dbus && interface.dbus_interface.is_some();
        let keyed_by = interface
            .keyed_by
            .as_ref()
            .map(|key| {
                let cxx_type = plain_cxx_type(&key.cxx_type);
                self.register_type(&cxx_type, &key.proto_type, &ParamKind::Value)
                    .map(|type_id| KeyModel { type_id })
            })
            .transpose()?;

        let mut methods = Vec::new();
        for method in &interface.methods {
            let size_owners: HashMap<&str, &str> = method
                .params
                .iter()
                .filter_map(|param| match &param.kind {
                    ParamKind::Bytes { size_param } => {
                        Some((size_param.as_str(), param.proto_field.as_str()))
                    }
                    _ => None,
                })
                .collect();

            let mut params = Vec::new();
            let mut dbus_num_in = 0usize;
            for param in &method.params {
                let proto_field =
                    (!param.proto_field.is_empty()).then(|| param.proto_field.clone());
                let role = match &proto_field {
                    Some(_) => ParamRoleModel::Value,
                    None => ParamRoleModel::AbsorbedSize {
                        owner_field: size_owners
                            .get(param.cxx_name.as_str())
                            .with_context(|| {
                                format!(
                                    "{}: absorbed parameter '{}' has no byte-buffer owner",
                                    method.rpc_name, param.cxx_name
                                )
                            })?
                            .to_string(),
                    },
                };
                let dbus_arg_index = if dbus_enabled {
                    if proto_field.is_none() {
                        bail!(
                            "{}: absorbed byte-buffer size parameter '{}' is not supported by the DBus backend",
                            method.rpc_name,
                            param.cxx_name
                        );
                    }
                    if matches!(param.kind, ParamKind::CString | ParamKind::Bytes { .. }) {
                        bail!(
                            "{}: parameter '{}' uses a pointer/string-buffer representation that is not supported by the DBus backend",
                            method.rpc_name,
                            param.cxx_name
                        );
                    }
                    match param.direction {
                        Direction::In => {
                            let index = dbus_num_in;
                            dbus_num_in += 1;
                            Some(index)
                        }
                        Direction::Out => None,
                        Direction::InOut => bail!(
                            "{}: parameter '{}' is `inout`, which DBus has no native representation for (only in/out args)",
                            method.rpc_name,
                            param.cxx_name
                        ),
                    }
                } else {
                    None
                };
                let (dbus_cpp_type, dbus_signature) = param
                    .dbus_type
                    .map(|value| {
                        Self::dbus_type_override(
                            value,
                            &param.proto_type,
                            &param.cxx_type.base_spelling,
                        )
                    })
                    .transpose()?
                    .map_or((None, None), |(cxx, signature)| (cxx, Some(signature)));
                params.push(ParamModel {
                    cxx_name: param.cxx_name.clone(),
                    type_id: self.register_type(&param.cxx_type, &param.proto_type, &param.kind)?,
                    direction: param.direction.into(),
                    proto_field,
                    role,
                    dbus_arg_index,
                    dbus_cpp_type,
                    dbus_signature,
                });
            }

            let mut request_fields = Vec::new();
            if let Some(key) = &keyed_by {
                request_fields.push(WireFieldModel {
                    proto_name: "id".to_string(),
                    number: 1,
                    type_id: key.type_id.clone(),
                });
            }
            for param in &params {
                if matches!(param.direction, DirectionModel::In | DirectionModel::InOut) {
                    if let Some(proto_name) = &param.proto_field {
                        request_fields.push(WireFieldModel {
                            proto_name: proto_name.clone(),
                            number: request_fields.len() + 1,
                            type_id: param.type_id.clone(),
                        });
                    }
                }
            }

            let mut response_fields = Vec::new();
            for param in &params {
                if matches!(param.direction, DirectionModel::Out | DirectionModel::InOut) {
                    if let Some(proto_name) = &param.proto_field {
                        response_fields.push(WireFieldModel {
                            proto_name: proto_name.clone(),
                            number: response_fields.len() + 1,
                            type_id: param.type_id.clone(),
                        });
                    }
                }
            }

            let return_value = method
                .return_value
                .as_ref()
                .map(|value| {
                    if dbus_enabled
                        && matches!(value.kind, ParamKind::CString | ParamKind::Bytes { .. })
                    {
                        bail!(
                            "{}: the return value uses a pointer/string-buffer representation that is not supported by the DBus backend",
                            method.rpc_name
                        );
                    }
                    self.register_type(&value.cxx_type, &value.proto_type, &value.kind)
                        .and_then(|type_id| {
                            let (dbus_cpp_type, dbus_signature) = value
                                .dbus_type
                                .map(|dbus_type| {
                                    Self::dbus_type_override(
                                        dbus_type,
                                        &value.proto_type,
                                        &value.cxx_type.base_spelling,
                                    )
                                })
                                .transpose()?
                                .map_or((None, None), |(cxx, signature)| (cxx, Some(signature)));
                            Ok(ReturnModel {
                                proto_field: value.proto_field.clone(),
                                type_id,
                                dbus_cpp_type,
                                dbus_signature,
                            })
                        })
                })
                .transpose()?;
            if let Some(value) = &return_value {
                response_fields.push(WireFieldModel {
                    proto_name: value.proto_field.clone(),
                    number: response_fields.len() + 1,
                    type_id: value.type_id.clone(),
                });
            }

            methods.push(MethodModel {
                rpc_name: method.rpc_name.clone(),
                cxx_symbol: method.cxx_symbol.clone(),
                is_const: method.is_const,
                params,
                request_fields,
                response_fields,
                return_value,
                dbus_num_in,
            });
        }

        let mut signals = Vec::new();
        for signal in &interface.signals {
            if dbus_enabled
                && signal
                    .fields
                    .iter()
                    .any(|field| matches!(field.kind, ParamKind::CString | ParamKind::Bytes { .. }))
            {
                bail!(
                    "{}: pointer/string-buffer signal fields are not supported by the DBus backend",
                    signal.rpc_name
                );
            }
            signals.push(self.build_signal(signal, keyed_by.as_ref())?);
        }

        let dbus = if self.include_dbus {
            interface
                .dbus_interface
                .as_ref()
                .map(|name| {
                    self.collect_dbus_custom_types(interface)
                        .map(|custom_types| DbusInterfaceModel {
                            name: name.clone(),
                            has_duration: custom_types.iter().any(|custom| {
                                matches!(custom, DbusCustomTypeModel::Duration { .. })
                            }),
                            custom_types,
                        })
                })
                .transpose()?
        } else {
            None
        };

        Ok(InterfaceModel {
            proto_package: interface.proto_package.clone(),
            service_name: interface.service_name.clone(),
            cxx_class: interface.cxx_class.clone(),
            cxx_qualified_class: interface.cxx_qualified_class(),
            cxx_namespace: interface.cxx_namespace.clone(),
            has_duration: interface.methods.iter().any(|method| {
                method
                    .params
                    .iter()
                    .any(|param| matches!(param.kind, ParamKind::Duration))
            }),
            keyed_by,
            dbus,
            methods,
            signals,
        })
    }

    fn build_signal(
        &mut self,
        signal: &Signal,
        keyed_by: Option<&KeyModel>,
    ) -> Result<SignalModel> {
        let request_fields = keyed_by
            .map(|key| {
                vec![WireFieldModel {
                    proto_name: "id".to_string(),
                    number: 1,
                    type_id: key.type_id.clone(),
                }]
            })
            .unwrap_or_default();

        let mut event_fields = Vec::new();
        for (index, field) in signal.fields.iter().enumerate() {
            let (dbus_cpp_type, dbus_signature) = field
                .dbus_type
                .map(|value| {
                    Self::dbus_type_override(
                        value,
                        &field.proto_type,
                        &field.cxx_type.base_spelling,
                    )
                })
                .transpose()?
                .map_or((None, None), |(cxx, signature)| (cxx, Some(signature)));
            event_fields.push(SignalFieldModel {
                cxx_name: field.cxx_name.clone(),
                proto_name: field.proto_field.clone(),
                number: index + 1,
                type_id: self.register_type(&field.cxx_type, &field.proto_type, &field.kind)?,
                dbus_cpp_type,
                dbus_signature,
            });
        }

        Ok(SignalModel {
            rpc_name: signal.rpc_name.clone(),
            cxx_symbol: signal.cxx_symbol.clone(),
            request_fields,
            event_fields,
        })
    }

    fn collect_dbus_custom_types(
        &mut self,
        interface: &Interface,
    ) -> Result<Vec<DbusCustomTypeModel>> {
        let mut custom_types = Vec::new();
        let mut seen = HashSet::new();

        for method in &interface.methods {
            for param in &method.params {
                if !param.proto_field.is_empty() {
                    self.visit_dbus_custom_type(
                        &param.cxx_type,
                        &param.proto_type,
                        &param.kind,
                        &mut custom_types,
                        &mut seen,
                    )?;
                }
            }
            if let Some(value) = &method.return_value {
                self.visit_dbus_custom_type(
                    &value.cxx_type,
                    &value.proto_type,
                    &value.kind,
                    &mut custom_types,
                    &mut seen,
                )?;
            }
        }
        for signal in &interface.signals {
            for field in &signal.fields {
                self.visit_dbus_custom_type(
                    &field.cxx_type,
                    &field.proto_type,
                    &field.kind,
                    &mut custom_types,
                    &mut seen,
                )?;
            }
        }

        Ok(custom_types)
    }

    fn visit_dbus_custom_type(
        &mut self,
        cxx_type: &CxxType,
        proto_type: &ProtoType,
        kind: &ParamKind,
        out: &mut Vec<DbusCustomTypeModel>,
        seen: &mut HashSet<String>,
    ) -> Result<()> {
        match kind {
            ParamKind::Duration => {
                if seen.insert(cxx_type.spelling.clone()) {
                    let type_id = self.register_type(cxx_type, proto_type, kind)?;
                    out.push(DbusCustomTypeModel::Duration { type_id });
                }
            }
            ParamKind::Bitmask { enum_cxx_type } => {
                let ProtoType::Repeated(inner) = proto_type else {
                    bail!("bitmask wire type must be repeated, got {proto_type}");
                };
                let ProtoType::Enum(enum_proto_name) = inner.as_ref() else {
                    bail!("bitmask DBus element must be an enum, got {inner}");
                };
                let enum_cxx = plain_cxx_type(enum_cxx_type);
                if seen.insert(enum_cxx_type.clone()) {
                    let enum_type = self.register_type(&enum_cxx, inner, &ParamKind::Value)?;
                    out.push(DbusCustomTypeModel::Enum {
                        type_id: enum_type,
                        values: self.dbus_enum_values(enum_proto_name)?,
                    });
                }
                if seen.insert(cxx_type.spelling.clone()) {
                    let type_id = self.register_type(cxx_type, proto_type, kind)?;
                    let enum_type = self.register_type(&enum_cxx, inner, &ParamKind::Value)?;
                    out.push(DbusCustomTypeModel::Bitmask { type_id, enum_type });
                }
            }
            ParamKind::Message { struct_proto_name } => {
                if seen.insert(cxx_type.spelling.clone()) {
                    let definition = self
                        .unit
                        .find_struct_by_proto_name(struct_proto_name)
                        .with_context(|| format!("struct '{struct_proto_name}' not registered"))?;
                    for field in &definition.fields {
                        self.visit_dbus_custom_type(
                            &field.cxx_type,
                            &field.proto_type,
                            &field.kind,
                            out,
                            seen,
                        )?;
                    }
                    let type_id = self.register_type(cxx_type, proto_type, kind)?;
                    out.push(DbusCustomTypeModel::Struct { type_id });
                }
            }
            ParamKind::Sequence(element) => {
                let element_cxx = plain_cxx_type(&element.cxx_type);
                self.visit_dbus_custom_type(
                    &element_cxx,
                    &element.proto_type,
                    &element.kind,
                    out,
                    seen,
                )?;
                if cxx_type.spelling.starts_with("std::vector<")
                    && seen.insert(cxx_type.spelling.clone())
                {
                    let type_id = self.register_type(cxx_type, proto_type, kind)?;
                    let element_type =
                        self.register_type(&element_cxx, &element.proto_type, &element.kind)?;
                    out.push(DbusCustomTypeModel::Vector {
                        type_id,
                        element_type,
                    });
                }
            }
            ParamKind::Map { value, .. } => {
                let value_cxx = plain_cxx_type(&value.cxx_type);
                self.visit_dbus_custom_type(&value_cxx, &value.proto_type, &value.kind, out, seen)?;
            }
            ParamKind::Value => {
                if let ProtoType::Enum(proto_name) = proto_type {
                    if seen.insert(cxx_type.spelling.clone()) {
                        let type_id = self.register_type(cxx_type, proto_type, kind)?;
                        out.push(DbusCustomTypeModel::Enum {
                            type_id,
                            values: self.dbus_enum_values(proto_name)?,
                        });
                    }
                }
            }
            ParamKind::CString | ParamKind::Bytes { .. } => {}
        }
        Ok(())
    }

    fn dbus_enum_values(&self, proto_name: &str) -> Result<Vec<DbusEnumValueModel>> {
        let definition = self
            .unit
            .enums
            .iter()
            .find(|definition| definition.proto_name == proto_name)
            .with_context(|| format!("enum '{proto_name}' not registered"))?;
        definition
            .values
            .iter()
            .map(|value| {
                let canonical_name = value.canonical_name.clone().with_context(|| {
                    format!(
                        "enum '{}' value '{}' has no @rpc.enum.value(name=\"...\") canonical name; DBus enums require stable string names",
                        definition.cxx_symbol, value.cxx_symbol
                    )
                })?;
                Ok(DbusEnumValueModel {
                    cxx_symbol: value.cxx_symbol.clone(),
                    canonical_name,
                })
            })
            .collect()
    }

    fn dbus_signature(
        &self,
        cxx_type: &CxxType,
        proto_type: &ProtoType,
        kind: &ParamKind,
    ) -> Result<String> {
        if matches!(
            cxx_type.base_spelling.as_str(),
            "uint8_t" | "std::uint8_t" | "unsigned char"
        ) {
            return Ok("y".to_string());
        }

        Ok(match proto_type {
            ProtoType::Bool => "b".to_string(),
            ProtoType::Int32 => "i".to_string(),
            ProtoType::Int64 => "x".to_string(),
            ProtoType::UInt32 => "u".to_string(),
            ProtoType::UInt64 => "t".to_string(),
            ProtoType::Double | ProtoType::Float => "d".to_string(),
            ProtoType::String | ProtoType::Enum(_) => "s".to_string(),
            ProtoType::Bytes => "ay".to_string(),
            ProtoType::Message(name) => {
                let definition = self
                    .unit
                    .find_struct_by_proto_name(name)
                    .with_context(|| format!("struct '{name}' not registered"))?;
                let mut signature = String::from("(");
                for field in &definition.fields {
                    signature.push_str(&self.dbus_signature(
                        &field.cxx_type,
                        &field.proto_type,
                        &field.kind,
                    )?);
                }
                signature.push(')');
                signature
            }
            ProtoType::Repeated(inner) => match kind {
                ParamKind::Sequence(element) => format!(
                    "a{}",
                    self.dbus_signature(&plain_cxx_type(&element.cxx_type), inner, &element.kind,)?
                ),
                _ => format!(
                    "a{}",
                    self.dbus_signature(cxx_type, inner, &ParamKind::Value)?
                ),
            },
            ProtoType::Map(key, value) => match kind {
                ParamKind::Map {
                    key: key_kind,
                    value: value_kind,
                } => format!(
                    "a{{{}{}}}",
                    self.dbus_signature(
                        &plain_cxx_type(&key_kind.cxx_type),
                        key,
                        &ParamKind::Value,
                    )?,
                    self.dbus_signature(
                        &plain_cxx_type(&value_kind.cxx_type),
                        value,
                        &value_kind.kind,
                    )?
                ),
                _ => bail!("map wire type has no map shape metadata"),
            },
        })
    }

    fn register_type(
        &mut self,
        cxx_type: &CxxType,
        proto_type: &ProtoType,
        kind: &ParamKind,
    ) -> Result<TemplateTypeId> {
        let key = format!(
            "{}|{}|{}|{}|{}|{:?}|{:?}",
            cxx_type.spelling,
            cxx_type.base_spelling,
            cxx_type.is_pointer,
            cxx_type.is_ref,
            cxx_type.is_const,
            proto_type,
            kind
        );
        if self.active_type_keys.contains(&key) {
            bail!(
                "recursive template type graph through '{}'",
                cxx_type.spelling
            );
        }
        if let Some(type_id) = self.type_ids_by_key.get(&key) {
            return Ok(type_id.clone());
        }

        self.active_type_keys.insert(key.clone());
        let shape = self.build_type_shape(proto_type, kind)?;
        self.active_type_keys.remove(&key);

        let type_id = format!("type_{}", self.next_type_id);
        self.next_type_id += 1;
        let model = TypeModel {
            id: type_id.clone(),
            cxx: cxx_type.into(),
            grpc: GrpcTypeModel {
                spelling: proto_type.to_string(),
            },
            dbus_signature: self.dbus_signature(cxx_type, proto_type, kind)?,
            shape,
        };
        self.types.insert(type_id.clone(), model);
        self.type_ids_by_key.insert(key, type_id.clone());
        Ok(type_id)
    }

    fn build_type_shape(
        &mut self,
        proto_type: &ProtoType,
        kind: &ParamKind,
    ) -> Result<TypeShapeModel> {
        Ok(match kind {
            ParamKind::Value => match proto_type {
                ProtoType::Enum(proto_name) => TypeShapeModel::Enum {
                    proto_name: proto_name.clone(),
                },
                _ => TypeShapeModel::Scalar,
            },
            ParamKind::CString => TypeShapeModel::CString,
            ParamKind::Bytes { size_param } => TypeShapeModel::Bytes {
                size_param: size_param.clone(),
            },
            ParamKind::Duration => TypeShapeModel::Duration,
            ParamKind::Bitmask { enum_cxx_type } => {
                let ProtoType::Repeated(inner) = proto_type else {
                    bail!("bitmask wire type must be repeated, got {proto_type}");
                };
                let enum_type =
                    self.register_type(&plain_cxx_type(enum_cxx_type), inner, &ParamKind::Value)?;
                TypeShapeModel::Bitmask {
                    enum_type,
                    enum_cxx_type: enum_cxx_type.clone(),
                }
            }
            ParamKind::Sequence(element) => TypeShapeModel::Sequence {
                element_type: self.register_sequence_element(element)?,
            },
            ParamKind::Message { struct_proto_name } => {
                let definition = self
                    .unit
                    .find_struct_by_proto_name(struct_proto_name)
                    .with_context(|| format!("struct '{struct_proto_name}' not registered"))?;
                let mut fields = Vec::new();
                for field in &definition.fields {
                    fields.push(TypeFieldModel {
                        cxx_name: field.cxx_name.clone(),
                        proto_name: field.proto_field.clone(),
                        type_id: self.register_type(
                            &field.cxx_type,
                            &field.proto_type,
                            &field.kind,
                        )?,
                    });
                }
                TypeShapeModel::Struct {
                    proto_name: struct_proto_name.clone(),
                    fields,
                }
            }
            ParamKind::Map { key, value } => TypeShapeModel::Map {
                key_type: self.register_type(
                    &plain_cxx_type(&key.cxx_type),
                    &key.proto_type,
                    &ParamKind::Value,
                )?,
                value_type: self.register_sequence_element(value)?,
            },
        })
    }

    fn register_sequence_element(&mut self, element: &SequenceElement) -> Result<TemplateTypeId> {
        self.register_type(
            &plain_cxx_type(&element.cxx_type),
            &element.proto_type,
            &element.kind,
        )
    }
}

fn plain_cxx_type(spelling: &str) -> CxxType {
    CxxType {
        spelling: spelling.to_string(),
        base_spelling: spelling.to_string(),
        is_pointer: false,
        is_ref: false,
        is_const: false,
    }
}

#[cfg(test)]
mod tests {
    use crate::ir::{
        CxxType, DbusType, Direction, Interface, KeyType, Method, Param, ParamKind, ProtoType,
        ReturnValue, Unit,
    };

    use super::{DbusCustomTypeModel, GenerationModel, TypeShapeModel};

    fn cxx(spelling: &str) -> CxxType {
        CxxType {
            spelling: spelling.to_string(),
            base_spelling: spelling.to_string(),
            is_pointer: false,
            is_ref: false,
            is_const: false,
        }
    }

    #[test]
    fn numbers_keyed_request_and_response_fields_deterministically() {
        let unit = Unit {
            interfaces: vec![Interface {
                proto_package: "workrave.test".to_string(),
                service_name: "Settings".to_string(),
                cxx_class: "Settings".to_string(),
                cxx_namespace: Vec::new(),
                methods: vec![Method {
                    rpc_name: "Update".to_string(),
                    cxx_symbol: "update".to_string(),
                    params: vec![
                        Param {
                            cxx_name: "input".to_string(),
                            cxx_type: cxx("int"),
                            direction: Direction::In,
                            kind: ParamKind::Value,
                            proto_field: "input".to_string(),
                            proto_type: ProtoType::Int32,
                            dbus_type: None,
                        },
                        Param {
                            cxx_name: "output".to_string(),
                            cxx_type: cxx("std::string"),
                            direction: Direction::Out,
                            kind: ParamKind::Value,
                            proto_field: "output".to_string(),
                            proto_type: ProtoType::String,
                            dbus_type: None,
                        },
                    ],
                    return_value: Some(ReturnValue {
                        cxx_type: cxx("bool"),
                        proto_field: "result".to_string(),
                        proto_type: ProtoType::Bool,
                        kind: ParamKind::Value,
                        dbus_type: None,
                    }),
                    is_const: false,
                }],
                signals: Vec::new(),
                keyed_by: Some(KeyType {
                    proto_type: ProtoType::UInt32,
                    cxx_type: "uint32_t".to_string(),
                }),
                dbus_interface: None,
            }],
            ..Unit::default()
        };

        let model = GenerationModel::build(&unit).unwrap();
        let method = &model.interfaces[0].methods[0];
        assert_eq!(method.request_fields[0].proto_name, "id");
        assert_eq!(method.request_fields[0].number, 1);
        assert_eq!(method.request_fields[1].proto_name, "input");
        assert_eq!(method.request_fields[1].number, 2);
        assert_eq!(method.response_fields[0].proto_name, "output");
        assert_eq!(method.response_fields[0].number, 1);
        assert_eq!(method.response_fields[1].proto_name, "result");
        assert_eq!(method.response_fields[1].number, 2);
    }

    #[test]
    fn registers_recursive_sequence_shape_as_type_references() {
        let sequence = ParamKind::Sequence(crate::ir::SequenceElement {
            cxx_type: "int".to_string(),
            proto_type: ProtoType::Int32,
            kind: Box::new(ParamKind::Value),
        });
        let unit = Unit {
            interfaces: vec![Interface {
                proto_package: "workrave.test".to_string(),
                service_name: "Values".to_string(),
                cxx_class: "Values".to_string(),
                cxx_namespace: Vec::new(),
                methods: vec![Method {
                    rpc_name: "Set".to_string(),
                    cxx_symbol: "set".to_string(),
                    params: vec![Param {
                        cxx_name: "values".to_string(),
                        cxx_type: cxx("std::vector<int>"),
                        direction: Direction::In,
                        kind: sequence,
                        proto_field: "values".to_string(),
                        proto_type: ProtoType::Repeated(Box::new(ProtoType::Int32)),
                        dbus_type: None,
                    }],
                    return_value: None,
                    is_const: false,
                }],
                signals: Vec::new(),
                keyed_by: None,
                dbus_interface: None,
            }],
            ..Unit::default()
        };

        let model = GenerationModel::build(&unit).unwrap();
        let type_id = &model.interfaces[0].methods[0].params[0].type_id;
        let ty = &model.types[type_id];
        let TypeShapeModel::Sequence { element_type } = &ty.shape else {
            panic!("expected sequence, got {:?}", ty.shape);
        };
        assert!(matches!(
            model.types[element_type].shape,
            TypeShapeModel::Scalar
        ));
        assert_eq!(model.types[element_type].grpc.spelling, "int32");
    }

    #[test]
    fn builds_dbus_signatures_argument_positions_and_custom_dependencies() {
        let unit = Unit {
            interfaces: vec![Interface {
                proto_package: "workrave.test".to_string(),
                service_name: "Values".to_string(),
                cxx_class: "Values".to_string(),
                cxx_namespace: Vec::new(),
                methods: vec![Method {
                    rpc_name: "Set".to_string(),
                    cxx_symbol: "set".to_string(),
                    params: vec![
                        Param {
                            cxx_name: "values".to_string(),
                            cxx_type: cxx("std::vector<int>"),
                            direction: Direction::In,
                            kind: ParamKind::Sequence(crate::ir::SequenceElement {
                                cxx_type: "int".to_string(),
                                proto_type: ProtoType::Int32,
                                kind: Box::new(ParamKind::Value),
                            }),
                            proto_field: "values".to_string(),
                            proto_type: ProtoType::Repeated(Box::new(ProtoType::Int32)),
                            dbus_type: None,
                        },
                        Param {
                            cxx_name: "bytes".to_string(),
                            cxx_type: cxx("std::vector<uint8_t>"),
                            direction: Direction::In,
                            kind: ParamKind::Sequence(crate::ir::SequenceElement {
                                cxx_type: "uint8_t".to_string(),
                                proto_type: ProtoType::UInt32,
                                kind: Box::new(ParamKind::Value),
                            }),
                            proto_field: "bytes".to_string(),
                            proto_type: ProtoType::Repeated(Box::new(ProtoType::UInt32)),
                            dbus_type: None,
                        },
                        Param {
                            cxx_name: "narrow".to_string(),
                            cxx_type: cxx("int64_t"),
                            direction: Direction::In,
                            kind: ParamKind::Value,
                            proto_field: "narrow".to_string(),
                            proto_type: ProtoType::Int64,
                            dbus_type: Some(DbusType::Byte),
                        },
                    ],
                    return_value: None,
                    is_const: false,
                }],
                signals: Vec::new(),
                keyed_by: None,
                dbus_interface: Some("org.example.Values".to_string()),
            }],
            ..Unit::default()
        };

        let model = GenerationModel::build(&unit).unwrap();
        let interface = &model.interfaces[0];
        let method = &interface.methods[0];
        assert_eq!(method.dbus_num_in, 3);
        assert_eq!(method.params[0].dbus_arg_index, Some(0));
        assert_eq!(model.types[&method.params[0].type_id].dbus_signature, "ai");
        assert_eq!(model.types[&method.params[1].type_id].dbus_signature, "ay");
        assert_eq!(method.params[2].dbus_cpp_type.as_deref(), Some("uint8_t"));
        assert_eq!(method.params[2].dbus_signature.as_deref(), Some("y"));

        let dbus = interface.dbus.as_ref().unwrap();
        assert_eq!(dbus.name, "org.example.Values");
        assert!(matches!(
            dbus.custom_types.as_slice(),
            [
                DbusCustomTypeModel::Vector { .. },
                DbusCustomTypeModel::Vector { .. }
            ]
        ));
    }

    #[test]
    fn rejects_dbus_inout_before_rendering() {
        let unit = Unit {
            interfaces: vec![Interface {
                proto_package: "workrave.test".to_string(),
                service_name: "Values".to_string(),
                cxx_class: "Values".to_string(),
                cxx_namespace: Vec::new(),
                methods: vec![Method {
                    rpc_name: "Update".to_string(),
                    cxx_symbol: "update".to_string(),
                    params: vec![Param {
                        cxx_name: "value".to_string(),
                        cxx_type: cxx("int"),
                        direction: Direction::InOut,
                        kind: ParamKind::Value,
                        proto_field: "value".to_string(),
                        proto_type: ProtoType::Int32,
                        dbus_type: None,
                    }],
                    return_value: None,
                    is_const: false,
                }],
                signals: Vec::new(),
                keyed_by: None,
                dbus_interface: Some("org.example.Values".to_string()),
            }],
            ..Unit::default()
        };

        assert!(GenerationModel::build_with_dbus(&unit, false).is_ok());
        let error = GenerationModel::build(&unit).unwrap_err().to_string();
        assert!(
            error.contains("DBus has no native representation"),
            "{error}"
        );
    }
}
