# Controlling Workrave over gRPC

Workrave exposes its core state — operation mode, breaks, configuration —
over gRPC whenever it's built with `-DWITH_RPC=ON`. This is a guide for
*using* that interface (e.g. with `grpcurl`), not for the code generator
that produces it — see [`tool/README.md`](tool/README.md) for that, and
[`../corenext/src/RpcCoreServer.hh`](../corenext/src/RpcCoreServer.hh) for
how the server itself is wired into the running app.

## DBus implementation selection

`WITH_DBUS` still controls whether Workrave exposes DBus. When it is enabled,
`WITH_RPC` selects which Core, Break, and Config implementation is built:

| Build options | DBus implementation |
|---|---|
| `WITH_DBUS=ON`, `WITH_RPC=OFF` | Legacy Python/Jinja generator |
| `WITH_DBUS=ON`, `WITH_RPC=ON` | Clang/Rust RPC generator |
| `WITH_DBUS=OFF` | No DBus API |

Only one implementation is present in a build. Both implementations use the
established `org.workrave.Workrave` service, `/org/workrave/Workrave` object
root, and `org.workrave.CoreInterface`, `org.workrave.BreakInterface`, and
`org.workrave.ConfigInterface` interface names.

The Clang-generated implementation currently requires the Qt UI, QtDBus, and
CoreNext. Small compatibility facades preserve historical details such as
32-bit Break timer replies, Config setter arguments/reply ordering, and the
`BreakStateChanged` event without restricting the richer native/gRPC APIs.

## Transport

The server listens on a Unix domain socket in the per-user state directory
— on macOS/Linux typically:

```
~/.workrave-qt/rpc.sock
```

(the exact path comes from `workrave::utils::Paths::get_state_directory()`
— check the app's startup log for a line like `RPC server listening on
unix:/home/you/.workrave/rpc.sock` if you're not sure). There's no port to
guess and no separate discovery step. Override the address with the
`WORKRAVE_RPC_ADDRESS` environment variable before starting Workrave — e.g.
`WORKRAVE_RPC_ADDRESS=127.0.0.1:0` for an ephemeral TCP port during
development (the actual bound port is then printed in the same log line).

Only running under a real desktop session starts the server — the test
harness (`HAVE_TESTS` + hook-based monitor) explicitly skips it, so you
won't see it while running `ctest`.

## Installing grpcurl

```bash
brew install grpcurl        # macOS
# or: go install github.com/fullstorydev/grpcurl/cmd/grpcurl@latest
```

No `.proto` files are needed — the server has [gRPC server
reflection](https://github.com/grpc/grpc/blob/master/doc/server-reflection.md)
enabled by default, so `grpcurl` can discover everything at runtime.

## Discovering the API

```bash
# List services
grpcurl -plaintext unix:$HOME/.workrave-qt/rpc.sock list

# workrave.ConfigService
# workrave.BreakService
# workrave.CoreService
# grpc.reflection.v1.ServerReflection
# grpc.reflection.v1alpha.ServerReflection

# List a service's methods
grpcurl -plaintext unix:$HOME/.workrave-qt/rpc.sock list workrave.CoreService

# Describe a method's request/response shape, including enum values
grpcurl -plaintext unix:$HOME/.workrave-qt/rpc.sock describe workrave.CoreService.SetOperationMode
grpcurl -plaintext unix:$HOME/.workrave-qt/rpc.sock describe workrave.rpc.core.OperationMode
```

`describe` is the fastest way to find the exact field/enum-value names
below without re-reading this doc — the service surface is regenerated from
the real C++ headers on every build, so it's always current; this document
isn't.

## A gotcha: "empty" responses that aren't actually empty

`grpcurl` (like all standard protobuf JSON printers) omits any field whose
value equals its type's zero value — `false` for `bool`, `0`/`""` for
numbers/strings, and **enum value `0`** for an enum. For most of Workrave's
enums, value `0` is the ordinary/default state (`OperationMode.NORMAL`,
`UsageMode.NORMAL`, ...), so this shows up constantly: switching back to
normal operation mode prints `{}`, not `{"value": "OPERATION_MODE_NORMAL"}`
— for both a plain call and an `OperationModeChanged` stream event.

This is not data loss and not a bug in Workrave's RPC layer: the field genuinely
has that value, protobuf just doesn't bother putting a zero value on the wire
(any correct client already treats "absent" as "default" when decoding), and
`grpcurl`'s pretty-printer mirrors that by default. Pass `-emit-defaults` to
see it explicitly:

```bash
grpcurl -plaintext -emit-defaults -d '{}' \
  unix:$SOCKET workrave.CoreService/GetActiveOperationMode
# {
#   "result": "OPERATION_MODE_NORMAL"
# }
```

Every example in this document omits `-emit-defaults` for brevity — add it
yourself whenever a response looks suspiciously empty.

Every example below sets `SOCKET=$HOME/.workrave-qt/rpc.sock` for brevity:

```bash
SOCKET=$HOME/.workrave-qt/rpc.sock
```

## CoreService — global state

```bash
# Is the user currently active?
grpcurl -plaintext -d '{}' unix:$SOCKET workrave.CoreService/IsActive

# Current operation mode
grpcurl -plaintext -d '{}' unix:$SOCKET workrave.CoreService/GetActiveOperationMode

# Suspend Workrave (NORMAL / SUSPENDED / QUIET)
grpcurl -plaintext -d '{"mode":"OPERATION_MODE_SUSPENDED"}' \
  unix:$SOCKET workrave.CoreService/SetOperationMode

# Suspend for a while, then automatically revert. `duration` accepts
# human-friendly text — "1h30m", "1h 30m", "90m", "45s", or a bare number
# (treated as minutes) — parsed server-side, not a raw integer whose unit
# you'd have to guess.
grpcurl -plaintext -d '{"mode":"OPERATION_MODE_SUSPENDED","duration":"1h30m"}' \
  unix:$SOCKET workrave.CoreService/SetOperationModeFor

# ForceBreak's request shape — `break_hint` is a repeated enum (a bitmask on
# the C++ side, workrave::utils::Flags<BreakHint>) — pass zero or more
# values:
#   {"id":"BREAK_ID_BREAK_ID_REST_BREAK","break_hint":["BREAK_HINT_USER_INITIATED"]}
# *Known broken*: calling it currently crashes the app. It reaches real
# break-window UI code, but the gRPC call runs on a worker thread separate
# from Qt's main thread, and that UI code isn't safe to call cross-thread —
# confirmed by an actual crash, not a guess. Don't call it until that's
# fixed (dispatching RPC-triggered Core calls onto the Qt main thread).

# Tell Workrave about activity from an external source (e.g. a script
# watching some other input device)
grpcurl -plaintext -d '{"who":"my-script","act":true}' \
  unix:$SOCKET workrave.CoreService/ReportActivity
```

The three service descriptors share the `workrave` package, keeping their wire
names short and consistent. Their generated payload types use separate
packages to prevent C++ and descriptor-name collisions:
`workrave.rpc.config`, `workrave.rpc.core`, and `workrave.rpc.breaks`.
Consequently, a Config response such as `SetStringResponse` is
`workrave::rpc::config::SetStringResponse` in C++, while the service/stub
classes are `workrave::rpc::ConfigService`, `workrave::rpc::CoreService`, and
`workrave::rpc::BreakService`. Run `describe
workrave.rpc.core.BreakId` or `describe workrave.rpc.breaks.BreakId` if in
doubt.

## BreakService — per-break state (micro-break / rest break / daily limit)

`BreakService` is *keyed*: there are three live `Break` instances (one per
`BreakId`), so every request carries an `id` field selecting which one to
target — the gRPC analog of DBus's per-object-path routing.

```bash
# Is the rest break currently enabled / running?
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_REST_BREAK"}' \
  unix:$SOCKET workrave.BreakService/IsEnabled
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_REST_BREAK"}' \
  unix:$SOCKET workrave.BreakService/IsTimerRunning

# Seconds until the rest break is due
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_REST_BREAK"}' \
  unix:$SOCKET workrave.BreakService/GetTimerRemaining

# Postpone / skip the currently showing break
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_MICRO_BREAK"}' \
  unix:$SOCKET workrave.BreakService/PostponeBreak
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_MICRO_BREAK"}' \
  unix:$SOCKET workrave.BreakService/SkipBreak
```

## ConfigService — read/write Workrave's own settings

Config keys are the same slash-separated paths Workrave's `config.xml`/ini
backend already uses internally (e.g. `timers/micro_pause/limit`).

```bash
# Typed get/set — pick the RPC matching the value's real C++ type
grpcurl -plaintext -d '{"key":"my/custom/key"}' unix:$SOCKET workrave.ConfigService/GetString
grpcurl -plaintext -d '{"key":"my/custom/key","v":"hello"}' unix:$SOCKET workrave.ConfigService/SetString

# GetString/GetBool/GetInt/GetInt64/GetDouble all respond with
# {out, result} — result is false (and out empty/zero) if the key doesn't
# exist, mirroring the real Configurator::get_value()'s bool return. Prints
# as `{}` for a missing key, not `{"result": false}` — see "A gotcha" above.
grpcurl -plaintext -d '{"key":"does/not/exist"}' unix:$SOCKET workrave.ConfigService/GetString
# {}

# ...WithDefault variants never fail — they return the fallback if unset.
grpcurl -plaintext -d '{"key":"does/not/exist","s":"fallback"}' \
  unix:$SOCKET workrave.ConfigService/GetStringWithDefault

grpcurl -plaintext -d '{"key":"my/custom/key"}' unix:$SOCKET workrave.ConfigService/HasUserValue
grpcurl -plaintext -d '{"key":"my/custom/key"}' unix:$SOCKET workrave.ConfigService/RemoveKey
```

## Signals — streaming events

Signals are server-streaming RPCs: the call doesn't return until the
connection is closed, and `grpcurl` prints one JSON object per event as it
arrives. Run this in one terminal, then trigger the corresponding change
(e.g. change the mode from the app, or from another `grpcurl` call) in
another:

```bash
# Operation mode changes
grpcurl -plaintext -d '{}' unix:$SOCKET workrave.CoreService/OperationModeChanged

# Break lifecycle events for the rest break specifically (keyed, like the
# unary BreakService calls above)
grpcurl -plaintext -d '{"id":"BREAK_ID_BREAK_ID_REST_BREAK"}' \
  unix:$SOCKET workrave.BreakService/BreakEvent
```

Ctrl-C to stop watching.

## Errors

A malformed request (e.g. an unparsable `duration` string) comes back as a
normal gRPC error status rather than crashing anything:

```bash
grpcurl -plaintext -d '{"mode":"OPERATION_MODE_NORMAL","duration":"garbage"}' \
  unix:$SOCKET workrave.CoreService/SetOperationModeFor
# ERROR:
#   Code: InvalidArgument
#   Message: invalid duration 'garbage': expected a number
```

## Security

The socket is a local Unix domain socket with no additional authentication
— access is whatever the socket file's own permissions grant (the
containing state directory is created `0700`, owner-only). There's no
network exposure by default; if you override `WORKRAVE_RPC_ADDRESS` to bind
a TCP address, you're opting into that yourself, and should bind to
loopback (`127.0.0.1:...`) unless you have a specific reason and a plan for
authentication — none is built in.
