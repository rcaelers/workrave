//! In-memory model produced by parsing an annotated C++ header.
//!
//! This is intentionally free of any Workrave-specific concepts: it only
//! knows about C++ declarations, `@rpc`/`@rpc.param` tags, and the proto
//! types they map to.

use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Direction {
    In,
    Out,
    InOut,
}

/// How a parameter's wire representation relates to its C++ type.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ParamKind {
    /// Direct mapping (bool, integers, double, enum, std::string, ...).
    Value,
    /// `char*`/`const char*` treated as a 0-terminated string.
    CString,
    /// A pointer paired with a separate length parameter, collapsed into a
    /// single proto `bytes` field. `size_param` names the C++ parameter that
    /// carries the length; that parameter is dropped from the RPC surface.
    Bytes { size_param: String },
    /// A `std::chrono::duration<Rep, Period>` (detected structurally by
    /// type shape, any Rep/Period — e.g. minutes, seconds, hours all count),
    /// put on the wire as a human-friendly proto `string` ("1h30m") rather
    /// than a raw integer whose unit a caller would have to guess. Parsed
    /// server-side via `rpc::parse_duration()` then `duration_cast` to
    /// whatever the real parameter's period happens to be.
    Duration,
    /// A `workrave::utils::Flags<Enum>` bitmask (detected structurally by
    /// type name, gated on the `@rpc.bitmask` tag on the `Flags` template
    /// itself — see clang_index.rs::flags_enum_type()), put on the wire as
    /// `repeated Enum` rather than a raw integer whose bit layout a caller
    /// would have to know. `enum_cxx_type` is the real, fully-qualified
    /// enum type to `static_cast` each wire value to/from.
    Bitmask { enum_cxx_type: String },
    /// A `std::vector<T>`/`std::list<T>` (detected structurally by
    /// declaration name, any T), put on the wire as `repeated <T's proto
    /// type>`. `element` fully describes T recursively (T may itself be a
    /// scalar, an enum, a `Message`, or even another `Sequence`).
    Sequence(SequenceElement),
    /// A plain struct/class value type with no other recognized shape,
    /// marshalled field-by-field via its registered `StructDef` (see
    /// `Unit::structs`). `struct_proto_name` names that `StructDef`.
    Message { struct_proto_name: String },
    /// A `std::map<K, V>` (detected structurally), put on the wire as a
    /// native proto `map<K, V>`. `key` is always a plain scalar/string
    /// (protobuf only allows integral/string map keys — anything else is
    /// rejected at generation time, see `resolve_value_type`); `value`
    /// describes V recursively the same way a sequence element does (V may
    /// be a struct, but not itself a sequence or another map — protobuf
    /// disallows both).
    Map { key: MapKey, value: Box<SequenceElement> },
}

/// A `Map`'s key type. Always a protobuf-map-legal scalar (validated at
/// generation time), so unlike a sequence element or a map's own value it
/// never needs a recursive `ParamKind` — unconditionally `ParamKind::Value`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MapKey {
    pub cxx_type: String,
    pub proto_type: ProtoType,
}

/// A `Sequence`'s element type (or a `Map`'s value type, which needs the
/// exact same shape), recursive: carries its own `ParamKind` so a sequence
/// of structs, a sequence of enums, or (in principle) a sequence of
/// sequences are all representable without a separate mechanism.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SequenceElement {
    pub cxx_type: String,
    pub proto_type: ProtoType,
    pub kind: Box<ParamKind>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ProtoType {
    Bool,
    Int32,
    Int64,
    UInt32,
    UInt64,
    Double,
    Float,
    String,
    Bytes,
    Enum(String),
    /// A nested message type, from a plain struct/class field-by-field —
    /// see `Unit::structs`.
    Message(String),
    Repeated(Box<ProtoType>),
    Map(Box<ProtoType>, Box<ProtoType>),
}

impl fmt::Display for ProtoType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ProtoType::Bool => write!(f, "bool"),
            ProtoType::Int32 => write!(f, "int32"),
            ProtoType::Int64 => write!(f, "int64"),
            ProtoType::UInt32 => write!(f, "uint32"),
            ProtoType::UInt64 => write!(f, "uint64"),
            ProtoType::Double => write!(f, "double"),
            ProtoType::Float => write!(f, "float"),
            ProtoType::String => write!(f, "string"),
            ProtoType::Bytes => write!(f, "bytes"),
            ProtoType::Enum(name) => write!(f, "{name}"),
            ProtoType::Message(name) => write!(f, "{name}"),
            ProtoType::Repeated(inner) => write!(f, "repeated {inner}"),
            ProtoType::Map(key, value) => write!(f, "map<{key}, {value}>"),
        }
    }
}

/// A C++ type as spelled at the annotated declaration site.
#[derive(Debug, Clone)]
pub struct CxxType {
    /// e.g. "workrave::OperationMode", "std::string", "bool", "int32_t"
    pub spelling: String,
    /// `spelling` with any pointer/reference/cv stripped, e.g. "workrave::OperationMode"
    /// for both `OperationMode` and `OperationMode &`. Used to declare local
    /// variables and build cast expressions in generated adapter code.
    pub base_spelling: String,
    pub is_pointer: bool,
    pub is_ref: bool,
    pub is_const: bool,
}

/// A single real C++ parameter, annotated (explicitly or by inference) with
/// wire direction/kind and the proto field it maps to (if any — a "bytes"
/// pointer's paired size parameter maps to no field of its own).
#[derive(Debug, Clone)]
pub struct Param {
    pub cxx_name: String,
    pub cxx_type: CxxType,
    pub direction: Direction,
    pub kind: ParamKind,
    pub proto_field: String,
    pub proto_type: ProtoType,
}

/// The method's non-parameter return value, if any (`void` -> None).
#[derive(Debug, Clone)]
pub struct ReturnValue {
    pub cxx_type: CxxType,
    pub proto_field: String,
    pub proto_type: ProtoType,
    pub kind: ParamKind,
}

#[derive(Debug, Clone)]
pub struct Method {
    pub rpc_name: String,
    pub cxx_symbol: String,
    /// All real parameters, in original declaration order (including the
    /// "size" half of a `bytes` pair, which carries no proto field of its
    /// own but is still needed to reconstruct the real call).
    pub params: Vec<Param>,
    pub return_value: Option<ReturnValue>,
    pub is_const: bool,
}

impl Method {
    pub fn request_fields(&self) -> Vec<&Param> {
        self.params
            .iter()
            .filter(|p| {
                matches!(p.direction, Direction::In | Direction::InOut) && !p.proto_field.is_empty()
            })
            .collect()
    }

    pub fn response_fields(&self) -> Vec<&Param> {
        self.params
            .iter()
            .filter(|p| {
                matches!(p.direction, Direction::Out | Direction::InOut)
                    && !p.proto_field.is_empty()
            })
            .collect()
    }
}

/// The type used to select one of several live instances of an `@rpc`
/// interface (see `Interface::keyed_by`) — the gRPC analog of a DBus object
/// path. Resolved either as a recognized primitive or as a real enum type
/// found by qualified name anywhere in the translation unit.
#[derive(Debug, Clone)]
pub struct KeyType {
    pub proto_type: ProtoType,
    /// The C++ type to declare/cast against in generated code, e.g.
    /// "workrave::BreakId" or "int32_t".
    pub cxx_type: String,
}

/// One argument of a `boost::signals2::signal<void(Args...)>` event, mapped
/// to a field of the generated `<Name>Event` streamed message.
#[derive(Debug, Clone)]
pub struct SignalField {
    pub cxx_name: String,
    pub cxx_type: CxxType,
    pub proto_field: String,
    pub proto_type: ProtoType,
    pub kind: ParamKind,
}

/// An `@rpc.signal(...)`-annotated `boost::signals2::signal<...> &` accessor
/// — a push event source, the gRPC analog of a DBus signal. Unlike DBus,
/// which needs a hand-written bridge class (e.g. CoreDBus/BreakDBus) to
/// forward each boost::signals2 firing onto the bus, the generated adapter
/// connects to the real signal directly: no bridge code required.
#[derive(Debug, Clone)]
pub struct Signal {
    pub rpc_name: String,
    /// The real accessor method name, e.g. "signal_operation_mode_changed".
    pub cxx_symbol: String,
    pub fields: Vec<SignalField>,
}

#[derive(Debug, Clone)]
pub struct Interface {
    pub service_name: String,
    pub cxx_class: String,
    pub cxx_namespace: Vec<String>,
    pub methods: Vec<Method>,
    pub signals: Vec<Signal>,
    pub keyed_by: Option<KeyType>,
}

impl Interface {
    /// Fully-qualified C++ class name, e.g. "workrave::Core" or "RpcTestServer".
    pub fn cxx_qualified_class(&self) -> String {
        if self.cxx_namespace.is_empty() {
            self.cxx_class.clone()
        } else {
            format!("{}::{}", self.cxx_namespace.join("::"), self.cxx_class)
        }
    }
}

#[derive(Debug, Clone)]
pub struct EnumValue {
    pub proto_name: String,
    pub cxx_symbol: String,
    /// An explicit, backend-agnostic name from `@rpc.enum.value(name="...")`,
    /// if any — independent of `proto_name` (always the auto-derived
    /// SCREAMING_SNAKE_CASE protobuf constant, which the gRPC backend keeps
    /// using regardless of this field). Not consumed anywhere yet; carried
    /// through so a future wire backend (e.g. DBus) can reproduce an exact
    /// existing wire name it doesn't get to choose on its own.
    pub canonical_name: Option<String>,
}

#[derive(Debug, Clone)]
pub struct EnumDef {
    /// Name used for the proto enum type. Derived from the C++ type, e.g.
    /// "workrave::OperationMode" -> "OperationMode".
    pub proto_name: String,
    /// Fully-qualified C++ enum type, as it must be spelled to `static_cast` to/from.
    pub cxx_symbol: String,
    pub values: Vec<EnumValue>,
    /// An explicit, backend-agnostic name for the enum type itself, from
    /// `@rpc.enum(name="...")`, if any. Same rationale as
    /// `EnumValue::canonical_name`.
    pub canonical_name: Option<String>,
}

/// One public data member of a plain struct/class registered as a
/// `StructDef`, mapped to a field of the generated nested message.
#[derive(Debug, Clone)]
pub struct StructField {
    pub cxx_name: String,
    pub cxx_type: CxxType,
    pub proto_field: String,
    pub proto_type: ProtoType,
    pub kind: ParamKind,
}

/// A plain C++ struct/class used as a param/return/field/signal-field type
/// with no other recognized shape (not std::string, not a chrono duration,
/// not `Flags<Enum>`, not a sequence) — auto-discovered the same way an enum
/// is: only its *public* data members become fields, walked in declaration
/// order. No annotation needed on the struct itself, same reasoning as
/// enums: it's only ever registered because an already-`@rpc`-annotated
/// declaration references it, so the gate is already at that outer
/// declaration.
#[derive(Debug, Clone)]
pub struct StructDef {
    /// Name used for the proto message type. Derived from the C++ type,
    /// e.g. "GenericDBusApplet::MenuItem" -> "MenuItem".
    pub proto_name: String,
    /// Fully-qualified C++ type, as it must be spelled to declare a local
    /// variable of this type.
    pub cxx_symbol: String,
    pub fields: Vec<StructField>,
}

#[derive(Debug, Clone, Default)]
pub struct Unit {
    pub enums: Vec<EnumDef>,
    pub structs: Vec<StructDef>,
    pub interfaces: Vec<Interface>,
}

impl Unit {
    pub fn find_enum(&self, cxx_symbol: &str) -> Option<&EnumDef> {
        self.enums.iter().find(|e| e.cxx_symbol == cxx_symbol)
    }

    pub fn find_struct(&self, cxx_symbol: &str) -> Option<&StructDef> {
        self.structs.iter().find(|s| s.cxx_symbol == cxx_symbol)
    }

    /// Looks up a registered struct by its generated proto message name
    /// (what `ParamKind::Message::struct_proto_name` refers to) — used by
    /// the C++ codegen to recurse into a message's own fields.
    pub fn find_struct_by_proto_name(&self, proto_name: &str) -> Option<&StructDef> {
        self.structs.iter().find(|s| s.proto_name == proto_name)
    }
}
