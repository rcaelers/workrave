# Standalone generated DBus architecture

## Status

This document records the architecture and migration plan for the
annotation-driven DBus support in `libs/rpc`. The standalone runtime,
QtDBus/GDBus renderers, direct `libs/core` and CoreNext bindings, generated event forwarding,
the application Control and Applet endpoints, name watching, and dual-backend
checked-in artifacts are implemented. The legacy `libs/dbus` runtime,
Python/Jinja generator, XML inputs, compatibility bridges, public `IDBus`
API, and tests have been removed.

## Goals

- `libs/rpc` is the sole Workrave D-Bus runtime.
- Generated bindings call the annotated C++ API directly. No handwritten
  facade or compatibility adapter sits between the transport and the API.
- Qt builds use QtDBus and GTK builds use GDBus automatically. There is no
  user-facing backend option.
- Qt and GIO expose identical service names, object paths, interface names,
  methods, signals, signatures, and conversion checks.
- Generated parameters, return values, compound types, and events share one
  semantic model and differ only in their final transport rendering.
- Builds without Rust or libclang use checked-in output for both backends.
- Runtime registration and signal subscriptions have explicit RAII ownership.

## Non-goals

- Do not move `IDBus`, `DBusGeneric`, or the legacy generated binding API into
  `libs/rpc` unchanged.
- Do not create a universal runtime DBus value class merely to hide `QVariant`
  and `GVariant`.
- Do not select Qt or GIO at runtime in a production Workrave build. The UI
  toolkit already determines the native event loop and DBus implementation.

## Retired legacy coupling

The old DBus generator emits Qt-specific code which uses these parts of
`libs/dbus`:

- `IDBus::connect()` for `(object path, interface) -> void *` routing;
- `register_binding()` and `find_binding()` for a global interface registry;
- `DBusBindingQt` for dispatch and introspection;
- `DBusMarshall<T>` for QVariant/QDBusArgument conversion;
- `IDBusPrivateQt` for signal transmission;
- `DBusRemoteException` for wire errors.

This is a small subset of `IDBus`. Service watching, process discovery, the
dummy backend, and the old Python-generated binding lifecycle are unrelated to
generated server dispatch.

The legacy registry is also a poor fit for generated typed bindings: it stores
raw `void *` implementations and raw binding pointers, requires global
`init_*()`/`instance()` functions, and has no registration lifetime object.

## Target dependency structure

```text
clang-rpc-gen semantic model
        |                         (build time only)
        +------------------+
        |                  |
        v                  v
generated Qt binding   generated GIO binding
        |                  |
        v                  v
rpc-dbus-qt           rpc-dbus-gio
        |                  |
      QtDBus              GDBus

rpc-dbus-common: errors and move-only registration lifetime
rpc-common: transport-independent RPC helpers such as Duration
rpc-grpc: gRPC server runtime
```

The former `libs/dbus` library is not present in this graph or in the source
tree.

## CMake target model

- `workrave-libs-rpc-common`: transport-independent helpers; no gRPC, Qt, GIO,
  or `libs/dbus` dependency.
- `workrave-libs-rpc`: the existing gRPC runtime target, retained for source
  compatibility; links `workrave-libs-rpc-common` and gRPC.
- `workrave-libs-rpc-dbus-common`: DBus errors and RAII registration
  primitives.
- `workrave-libs-rpc-dbus-qt`: Qt dispatcher and codecs; links QtDBus.
- `workrave-libs-rpc-dbus-gio`: GIO dispatcher and codecs; links GIO.

Backend selection is internal:

- `WITH_UI=Qt` selects the Qt target.
- `WITH_UI=Gtk+3` selects the GIO target.
- Tests may build both targets when both development packages are available.

`libs/rpc` is added when either gRPC or generated DBus is enabled. Enabling
generated DBus alone must not discover or link gRPC.

## Runtime interfaces

The common runtime owns only concepts which are genuinely common: errors and
registration lifetime. There is no runtime backend enum; CMake selects exactly
one native backend for a production build. Dispatch and wire values remain
backend-specific.

Each backend provides a server and an interface contract:

```cpp
class QtInterface
{
public:
  virtual std::string_view name() const = 0;
  virtual std::string_view introspection() const = 0;
  virtual bool dispatch(const QDBusMessage &, const QDBusConnection &) = 0;
};

class GioInterface
{
public:
  virtual std::string_view name() const = 0;
  virtual std::string_view introspection() const = 0;
  virtual void dispatch(std::string_view method,
                        GVariant *parameters,
                        GDBusMethodInvocation *invocation) = 0;
};
```

The servers register one object-path router and allow multiple interfaces at a
path. A move-only `Registration` unregisters its interface on destruction.
Bindings are owned with `shared_ptr` by the registration state; implementation
objects remain typed references whose lifetime must exceed the registration.

## Generated binding shape

For every annotated service, the generator emits one selected backend-specific
endpoint class from the same semantic model:

```cpp
class CoreDBusQtBinding final : public rpc::dbus::QtInterface
{
public:
  CoreDBusQtBinding(rpc::dbus::QtServer &, std::string path, Core &);
private:
  Core &implementation_;
};

class CoreDBusGioBinding final : public rpc::dbus::GioInterface
{
public:
  CoreDBusGioBinding(rpc::dbus::GioServer &, std::string path, Core &);
private:
  Core &implementation_;
};
```

There is no `void *`, global binding registry, `init_*()`, `instance()`, or
handwritten facade. A Break endpoint is instantiated once per Break object
path. The path belongs to the endpoint, so signal emission does not take an
extra path argument.

Annotated C++ events are connected by the generated endpoint. Scoped signal
connections are stored by the endpoint and released with its registration.

## Error and conversion behavior

- Common errors carry a DBus error name plus a diagnostic message without
  depending on Boost or `workrave-libs-utils`.
- Backend dispatch catches these errors and always sends the corresponding
  error reply.
- Qt conversion uses `QVariant`/`QDBusArgument`; GIO conversion uses
  `GVariant`.
- Integer narrowing requested by DBus annotations uses checked conversion in
  both directions and reports `org.freedesktop.DBus.Error.InvalidArgs` on
  overflow.
- Compound type traversal and field paths are derived from the shared type
  model, ensuring both backends diagnose the same logical field.

## D-Bus type coverage

The annotation model uses `DbusType` rather than a scalar-only enum. Method
inputs, output parameters, return values, and signal fields share the same
override path and backend codecs.

- Basic types: `y`, `b`, `n`, `q`, `i`, `u`, `x`, `t`, `d`, `s`, `o`, `g`,
  and `h`.
- Container types: arrays (`aT`), dictionaries (`a{KV}`), structs (`(...)`),
  and typed variants (`v`). Arrays, dictionaries, and structs are inferred
  recursively from the C++ API; `variant` is an explicit override because
  the variant boundary is not present in the native C++ type.
- UNIX file descriptors use native descriptor transfer on Unix. QtDBus uses
  `QDBusUnixFileDescriptor`; GDBus resolves incoming handles from the message's
  `GUnixFDList` and attaches outgoing descriptors to method replies and
  signals. On platforms without GIO's UNIX-FD APIs (notably Windows), the same
  generated bindings compile normally and report a D-Bus failure only if an
  `h` value is actually used.
- Numeric representation overrides are range-checked in both directions.
  Object paths and signatures retain distinct wrapper types until the native
  API boundary, so backend validation is not weakened into plain strings.

The generated endpoint surface currently consists of methods and signals.
Standard service-side peer/introspection/property machinery remains supplied
by QtDBus/GDBus; generating application properties from annotated C++
getters/setters would be a separate API feature, not a missing wire type.

## Generated fallback files

Adjacent `gen/` directories contain `*DBusQt.hh/.cc` and `*DBusGio.hh/.cc`.
Every file has a generated-file warning and participates in the source hash
check. Refreshing pre-generated output always runs both renderers, independent
of the local UI toolkit, so a developer cannot accidentally update only one
backend. A normal build copies the selected pair to the backend-neutral
`*DBus.hh/.cc` build names.

## Migration plan

### Phase 1: runtime foundation

Implemented.

1. Split `libs/rpc` into common, gRPC, DBus-common, Qt, and GIO targets.
2. Add common error and RAII registration types.
3. Add Qt and GIO server/interface contracts with backend-native dispatch.
4. Add unit tests for registration ownership, routing, introspection, replies,
   errors, and signals.

### Phase 2: generator backends

Implemented for methods, parameters, return/out values, compound types,
checked narrowing, and events. Both renderer outputs compile against their
real native development headers in generator tests.

1. Rename the existing DBus renderer to the Qt renderer.
2. Replace `IDBus`/`DBusBindingQt` output with typed `QtInterface` endpoints.
3. Add the GIO renderer using the same template model and annotations.
4. Generate compile tests and golden output for both backends.
5. Extend `rpc_refresh_pregenerated` and fallback copying to both outputs.

### Phase 3: CoreNext integration

Implemented. Event subscriptions are owned by the generated endpoints rather
than by `RpcDBusServer`.

1. Make `RpcDBusServer` own the selected native server.
2. Register typed Core and Config endpoints at the Core path.
3. Register one typed Break endpoint at each Break path.
4. Move generated event subscriptions into those endpoints.
5. Remove `IDBus` from `RpcDBusServer` and the generated public headers.

### Phase 4: application composition

Implemented. The selected core's generated server owns the service name. The generated
application server registers the UI Control and Applet interfaces at the same
object path on one native session-bus connection. Their endpoints call
`Menus` and `GenericDBusApplet` directly, including compound Applet method
values and emitted events, and have checked-in Qt and GIO output. Native RAII
name watchers track applet clients without using `IDBus`. Both standalone
generated DBus builds and mixed legacy-feature-plus-gRPC builds use these new
application endpoints. The GNOME Shell prelude availability check uses GIO
directly instead of obtaining an `IDBus` from Core.

1. Move DBus service-name ownership to the selected new server. Implemented.
2. Generate/migrate the UI Control interface. Implemented.
3. Migrate the legacy Applet interface and its emitted events. Implemented,
   with wire-compatible method, signal, argument, and compound-field types.
4. Stop exposing the legacy `IDBus` through Core APIs. Implemented.
5. Remove every `workrave-libs-dbus` dependency. Implemented.

### Phase 5: legacy retirement

Implemented. `WITH_DBUS=ON` selects the generated `libs/rpc` runtime without
changing the selected core, including checked-in fallback bindings when Rust
or libclang is unavailable. The established `WITH_DBUS` option controls the
generated implementation, so existing build scripts continue to work.

1. Make generated `libs/rpc` D-Bus the default for every CoreNext build.
   Implemented.
2. Remove the legacy core's `IDBus` API and direct signal emission.
   Implemented.
3. Remove the legacy application/CoreNext compatibility bridges and XML
   generator inputs. Implemented.
4. Delete `libs/dbus`, `dbusgen.py`, its templates, and self-tests.
   Implemented.

### Phase 6: direct `libs/core` support

Implemented. GTK may continue using `libs/core`; enabling D-Bus no longer
forces CoreNext. `Core`, `Break`, their existing Boost signals, and the shared
configurator are annotated directly. The generated server registers the same
Core, Break, and Config wire interfaces and object paths as the CoreNext
server. No facade or compatibility runtime sits between dispatch and these
objects. Separate core-selection macros ensure a CoreShadow build starts only
the live core's server.

## Acceptance criteria

- Qt and GTK DBus-only builds compile without gRPC, Rust, or libclang by using
  checked-in generated files.
- No build contains a `libs/dbus` include or link edge.
- Generated method calls and events work directly against both `libs/core`
  and CoreNext APIs.
- Introspection XML and observable wire behavior are equivalent across both
  backends and compatible with the existing Workrave interface.
- Registration teardown leaves no raw binding or implementation pointers in a
  dispatcher.
