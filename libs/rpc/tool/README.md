# clang-rpc-gen

Generates a gRPC C++ service adapter directly from an annotated C++ header —
no separate IDL file, no changes to the annotated class or methods.

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
// @rpc(service="TestService")
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

- `@rpc(service="Name"[, keyed_by="Type"])` on a class marks it as an
  RPC-exposed interface. `keyed_by` is for interfaces with multiple live
  instances (see below).
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
  signature `e{KV}`) — not `repeated`. K must be a protobuf-legal map key
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
  using regardless of these tags. Not consumed by anything yet — carried
  through the IR (and surfaced as a `// canonical_name: "..."` comment in
  the generated `.proto`) for a future wire backend that needs to reproduce
  an exact existing name it doesn't get to choose, e.g. a DBus backend
  matching this tree's `workrave-service.xml`, whose `<enum name="operation_mode">`/
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
@rpc(service="ServiceName")

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
  --anchor-source path/to/Annotated.cc \
  --compile-commands build/compile_commands.json \
  --out-proto build/Annotated.proto \
  --out-adapter-hh build/AnnotatedServiceImpl.hh \
  --out-adapter-cc build/AnnotatedServiceImpl.cc \
  --proto-package your.proto.package \
  --annotations path/to/annotations.rpc   # optional, see "Out-of-band annotations"
```

`--anchor-source` is a `.cc`/`.cpp` that `#include`s `--header`, used to look
up its real compiler flags in `--compile-commands` (headers themselves are
never keys in a compile-commands database).

Run the generated `--out-proto` through `protoc` + `grpc_cpp_plugin` to get
the real `<Service>::Service` base class and `<Rpc>Request`/`<Rpc>Response`
message types the generated adapter (`--out-adapter-hh`/`--out-adapter-cc`)
compiles against.

## A note on the `clang` crate's `clang_10_0` feature

`Cargo.toml` enables `clang_10_0` on the `clang` crate together with `runtime`.
This does **not** cap the C++ language standard that can be parsed — it only
bounds which libclang *API functions* get Rust bindings generated (the crate
hasn't shipped bindings for APIs added after libclang 10.0). Parsing itself
is done by whatever `libclang` shared library is found at runtime (via
`clang-sys`'s `runtime`/dlopen feature) — e.g. a real clang 22 install — and
it parses whatever `-std=` flag is passed through from `compile_commands.json`
(C++23, C++26, whatever the project uses). All AST/type/comment traversal
this tool needs has existed since long before libclang 10.0. See
`tests/generate_test.rs`, which parses a fixture containing a C++23-only
construct (`if consteval`) end to end as a regression check for this.

## Design

This crate has no dependency on any particular consumer project — everything
project-specific comes in through CLI flags. See `src/lib.rs` for the
library API (`generate()`), `src/clang_index.rs` for the libclang traversal,
`src/ir.rs` for the parsed model, and `src/proto_gen.rs`/`src/cpp_gen.rs` for
the two code generators (both askama-templated, see `src/templates/`).
