# clang-rpc-gen

Generates gRPC C++ service adapters and optional QtDBus/GDBus bindings directly from
an annotated C++ header—no separate IDL file and no generated fragments stored
in the Rust context model.

Looking to *use* Workrave's gRPC interface (e.g. with `grpcurl`) rather than
extend this tool? See [`../README.md`](../README.md) instead.

## Idea

Normal gRPC workflow: write a `.proto`, run `protoc`, get an abstract
`Service` base class you implement by hand. This tool inverts that: you
annotate the *real* C++ declarations you want to expose with lightweight
comment tags, and it parses them directly (via libclang) to generate both
the `.proto` schema and the glue that forwards each RPC straight into the
real method. The `.proto` is a disposable, regenerated build artifact —
never hand-edited.

## Tag vocabulary

```cpp
// @rpc(service="workrave.TestService")
class RpcTestServer
{
public:
  // @rpc(name="Ping")
  std::string ping(std::string message);

  // @rpc(name="GetMode")
  // @rpc.param(mode, dir=out)
  bool get_mode(TestMode &mode);

  // @rpc.signal(name="ModeChanged")
  boost::signals2::signal<void(TestMode)> &signal_mode_changed();
};
```

Add `@rpc.dbus(interface="org.example.Interface")` to the class and pass
`--out-dbus-hh` plus `--out-dbus-cc` to render a DBus binding from the same
model. The DBus interface name is wire-visible and is not affected by
`--adapter-namespace`. `--dbus-backend qt|gio` selects the native renderer;
Workrave's CMake integration derives it from the UI toolkit.

Tags are found by regex against the raw comment text — **any** comment
style works (`//`, `///`, `//!`, `/* */`, `/** */`), including plain `//`.
This is deliberate: `//!`/`///`/`/** */` are Doxygen's markers for "this
comment is the public documentation of the following declaration" — an
`@rpc` tag is a machine-readable instruction to this tool, not API prose,
and annotating a method shouldn't force its comment to masquerade as
documentation (or silently become part of it) for whatever doc generator
the project uses. Use a real Doxygen comment alongside a tag when you also
want to document the method — they can share one comment block or live in
separate ones; the tool doesn't care either way, it just greps for `@rpc`.

- `@rpc(service="package.Name"[, keyed_by="Type"])` on a class marks it as
  an RPC-exposed interface. This fully-qualified name is the public gRPC wire
  name and produces `package Name` in the generated `.proto`; `keyed_by` is
  for interfaces with multiple live instances (see below).
- `@rpc(name="Name")` on a method (each overload individually) marks it as
  a unary RPC.
- `@rpc.signal(name="Name"[, fields="a,b,..."])` on a
  `boost::signals2::signal<void(Args...)> &` accessor marks it as a push
  event source (a server-streaming RPC) — the gRPC analog of a DBus signal.
  `fields` names the signal's arguments (required for more than one
  argument; defaults to a single field named `value` otherwise). **Requires
  a real signal accessor to connect to** — a class that only *calls into*
  some other push mechanism internally (e.g. firing a DBus signal directly
  from inside a method, rather than exposing its own
  `boost::signals2::signal`) has nothing for this tag to attach to. Refactor
  the class to expose a real accessor for the event if you need to annotate
  one of these; there's no code-free way to do it.
- `@rpc.dbus(return_type="int32")` changes an `@rpc` method's D-Bus scalar
  representation. `@rpc.param(value, dir=in, dbus_type="int32")` does the
  same for an input or output parameter, and
  `@rpc.signal(name="Progress", dbus_types="int32")` does it for signal
  fields. All D-Bus basic types are available, plus typed variants: `byte` (`y`), `boolean`/`bool`
  (`b`), `int16` (`n`), `uint16` (`q`), `int32` (`i`), `uint32` (`u`),
  `int64` (`x`), `uint64` (`t`), `double` (`d`), `string` (`s`),
  `object_path` (`o`), `signature` (`g`), `unix_fd` (`h`), and a typed
  `variant` (`v`). The native C++
  and gRPC types remain unchanged. Generated code performs checked numeric
  conversions, preserves object-path/signature/file-descriptor semantics in
  backend-neutral wrapper types, and reports invalid values as D-Bus argument
  errors. Native C++ types already map to their natural D-Bus basic type;
  for example, `uint8_t` is a D-Bus byte without an override.
- D-Bus containers are inferred from the existing C++ API rather than named
  by `dbus_type`: `std::vector<T>` and `std::list<T>` become arrays (`aT`),
  `std::map<K, V>` becomes a dictionary (`a{KV}`), and value structs become
  D-Bus structs (`(...)`). These shapes nest recursively. A `variant`
  override wraps the API's static C++ type in a D-Bus variant while keeping
  that underlying type checked on decode; it does not expose Qt or GIO
  dynamic-value classes to the API.
- Direction is inferred for by-value/`const T&` parameters (`in`) and non-`void`
  returns (implicit response field). Any non-const pointer/reference parameter
  **must** carry `@rpc.param(name, dir=in|out|inout)`.
- `char*`/`void*` need `kind=cstring` (0-terminated string) or
  `kind=bytes, size=<other-param>` (length-paired binary buffer, collapsed
  into a single proto `bytes` field). v1 only supports these as `in` parameters.
- Enum types referenced by an annotated method are auto-discovered — no
  separate tag needed on the enum itself.
- A plain struct/class value type (no other recognized shape) is
  auto-discovered the same way — no tag needed on it either, only its
  *public* data members become fields (private ones are silently skipped),
  walked in declaration order into a nested proto `message`. Works as an
  `in`/`out`/`inout` parameter, a return value, or a signal field, and
  nests arbitrarily (a struct field can itself be a struct or a sequence).
  A struct with no public data members at all is an error, not a silently
  empty message.
- `std::vector<T>`/`std::list<T>` (any T, detected structurally by
  declaration name) is wire-encoded as `repeated <T's proto type>`, the same
  `in`/`out`/`inout`/return/signal-field positions as a struct. T may be a
  scalar, an enum, a struct, or (in principle) another sequence — but not a
  map (protobuf has no `repeated map<...>` syntax; a map field is already
  wire-repeated on its own).
- `std::map<K, V>` (detected structurally) is wire-encoded as a native proto
  `map<K, V>` field (the analog of dbusgen.py's `<dictionary>`, DBus
  signature `a{KV}`) — not `repeated`. K must be a protobuf-legal map key
  (bool, an integer type, or `std::string`; anything else — a float, an
  enum, a struct, a container — is rejected at generation time with a clear
  error, since protobuf itself doesn't allow it). V may be a scalar, an
  enum, or a struct, but not itself a sequence or another map (also a
  protobuf restriction, also rejected at generation time rather than
  silently producing a broken `.proto`).
- `keyed_by="Type"` on a service (a recognized primitive name, or a
  fully-qualified enum type found anywhere in the translation unit) means
  the interface has multiple live C++ instances, distinguished by an `id`
  field added to every request. The generated adapter resolves the target
  instance from an `rpc::InstanceRegistry<Type, ImplClass>` instead of
  holding a single fixed reference — the gRPC analog of DBus's
  per-object-path routing.
- A `std::chrono::duration<Rep, Period>` parameter (any Rep/Period — minutes,
  seconds, hours, ...) needs **no tag at all**: detected structurally, since
  the C++ type already says everything needed. It's wire-encoded as a plain
  proto `string` ("1h30m", "1h 30m", "90m", "45s", a bare number as minutes)
  rather than a raw integer whose unit a caller would have to guess, parsed
  server-side by `rpc::parse_duration()` (in `libs/rpc/include/rpc/Duration.hh`)
  then `duration_cast` to the real parameter's period. v1 only supports this
  for `in` parameters, not returns or out-params.
- `@rpc.enum(name="...")` on an enum type / `@rpc.enum.value(name="...")` on
  one of its enumerators pins an explicit, backend-agnostic canonical name,
  independent of the auto-derived protobuf enum name the gRPC backend keeps
  using regardless of these tags. The DBus renderers use these names to match
  this tree's established `workrave-service.xml`, whose `<enum name="operation_mode">`/
  `<value name="normal">` become the literal string sent on the wire (DBus
  has no native enum type).
- A `Flags<Enum>`-shaped bitmask parameter (any class template literally
  named `Flags`) is recognized once its own primary template carries an
  `@rpc.bitmask` tag — said on the template itself (e.g.
  `libs/utils/include/utils/Enum.hh`'s `workrave::utils::Flags`), not at
  every call site. Wire-encoded as `repeated Enum` rather than a raw integer
  whose bit layout a caller would have to know. Same `in`-only restriction as
  duration.

## Out-of-band annotations

For a header that can't carry `@rpc` comments of its own (third-party code,
generated code, anything you'd rather not touch), pass `--annotations
<file>` — a separate file mapping fully-qualified C++ names to tag blocks:

```
[Namespace::Class]
@rpc(service="example.ServiceName")

[Namespace::Class::method_name(ParamType1,ParamType2)]
@rpc(name="MethodName")
@rpc.param(x, dir=out)
```

A method's key includes its parenthesized, comma-joined parameter types
(spelled exactly as they appear at the declaration) to disambiguate
overloads — the same reason each overload needs its own comment block
in-source. Whitespace inside a key doesn't matter. If the header *also* has
real comments, both are merged (concatenated, then parsed as one blob) —
this isn't an all-or-nothing override, you can annotate some declarations
in-source and others externally, or add a stray `@rpc.param(...)` externally
to a method that already has its `@rpc(name=...)` in-source. See
`src/external_annotations.rs` for the exact format and merge rules.

## Usage

```
clang-rpc-gen \
  --header path/to/Annotated.hh \
  --parse-context build/Annotated.rpc-parse-context \
  --out-proto build/Annotated.proto \
  --out-adapter-hh build/AnnotatedServiceImpl.hh \
  --out-adapter-cc build/AnnotatedServiceImpl.cc \
  --out-types-proto build/AnnotatedTypes.proto \
  --proto-types-package your.proto.types.package \
  --grpc-services-namespace rpc \
  --adapter-namespace your::adapter::namespace \
  --annotations path/to/annotations.rpc \
  --dbus-backend gio \
  --out-dbus-hh build/AnnotatedDBus.hh \
  --out-dbus-cc build/AnnotatedDBus.cc
```

The annotations, split protobuf type output, and DBus output flags are
optional. `--out-types-proto` and `--proto-types-package` must be supplied
together. They move generated enums and messages into an imported schema and
package while leaving the service in the package declared by its
`@rpc(service="package.Service")` annotation. This lets multiple services
share one wire package without putting their C++ payload types in the same
namespace. `--grpc-services-namespace` appends one unqualified namespace
to the protobuf package for generated gRPC service/stub classes without
changing their wire names. `--adapter-namespace` wraps the generated
`<Service>ServiceImpl` and DBus binding classes; it does not change the
protobuf packages, gRPC service name, DBus interface name, or protoc-generated
C++ namespaces.

`--parse-context` is a compiler-independent file produced from the owning
CMake target. It contains one typed `kind=value` record per line, for example:

```
standard=gnu++20
include=/path/to/public/include
define=HAVE_CONFIG_H=1
sysroot=/path/to/sdk
target=aarch64-linux-gnu
```

Only properties that can change preprocessing or the C++ AST are represented.
Compiler build commands, dependency-scanner flags, warning flags, and output
options are never passed to libclang. `cmake/modules/Rpc.cmake` generates this
file from the effective usage requirements of the macro's `TARGET` argument.

Run the generated schemas through `protoc` + `grpc_cpp_plugin` to get the real
`<Service>::Service` base class and `<Rpc>Request`/`<Rpc>Response` message
types the generated adapter (`--out-adapter-hh`/`--out-adapter-cc`) compiles
against.

## Checked-in fallback

Workrave keeps both `*DBusQt.hh/.cc` and `*DBusGio.hh/.cc` outputs used by its
native interfaces in adjacent `gen/` directories. CMake's `RPC_CODEGEN=AUTO`
mode uses live Rust/libclang generation when both Cargo and libclang are
available and otherwise copies the UI toolkit's checked-in pair to the normal
backend-neutral build names.
Use `RPC_CODEGEN=OFF` to force the fallback or `RPC_CODEGEN=ON` to require live
generation. After changing RPC annotations, configure with codegen enabled and
build the `rpc_refresh_pregenerated` target; source-header hashes prevent stale
fallback files from being used silently.

## A note on the `clang` crate's `clang_10_0` feature

`Cargo.toml` enables `clang_10_0` on the `clang` crate together with `runtime`.
This does **not** cap the C++ language standard that can be parsed — it only
bounds which libclang *API functions* get Rust bindings generated (the crate
hasn't shipped bindings for APIs added after libclang 10.0). Parsing itself
is done by whatever `libclang` shared library is found at runtime (via
`clang-sys`'s `runtime`/dlopen feature) — e.g. a real clang 22 install — and
it parses whatever `standard=` value the build system writes to the parse
context (C++23, C++26, whatever the project uses). All AST/type/comment traversal
this tool needs has existed since long before libclang 10.0. See
`tests/generate_test.rs`, which parses a fixture containing a C++23-only
construct (`if consteval`) end to end as a regression check for this.

## Design

This crate has no dependency on any particular consumer project — everything
project-specific comes in through CLI flags. See `src/lib.rs` for the
library API (`generate()`), `src/clang_index.rs` for the libclang traversal,
`src/ir.rs` for the parsed model, `src/template_model.rs` for the owned
semantic context exposed to templates, and `src/backends/` for the artifact
renderers. Each generated file is rendered once from an embedded MiniJinja
root template. Iteration over interfaces, methods, parameters, signals, and
types lives in those templates; recursive gRPC wire conversion lives in
`service_impl_macros.jinja`. Rust remains responsible for parsing, semantic
validation, deterministic type lookup, and DBus wire signatures, and does not
construct generated statements or render per-method/per-argument fragments.
