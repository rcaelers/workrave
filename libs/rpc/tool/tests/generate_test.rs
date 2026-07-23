//! End-to-end test of the generator against `tests/fixtures/simple.hh`,
//! independent of any consumer project's build system: sets up its own
//! tiny `compile_commands.json` + anchor `.cc` in a tempdir.

use std::fs;
use std::path::PathBuf;
use std::sync::{Mutex, OnceLock};

use tempfile::TempDir;

use clang_rpc_gen::{generate, GenerateOptions, GeneratedFiles};

fn fixtures_dir() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("tests/fixtures")
}

/// `clang::Clang::new()` (called inside `generate()`) is a process-wide
/// singleton guard — only one may exist at a time. `cargo test` runs tests in
/// this binary concurrently on separate threads of the same process, so
/// without serializing, two tests racing to parse hit "an instance of Clang
/// already exists". Real usage (the CLI) never hits this: one process, one
/// call. This lock is a test-only concern, not a production limitation.
fn clang_test_lock() -> &'static Mutex<()> {
    static LOCK: OnceLock<Mutex<()>> = OnceLock::new();
    LOCK.get_or_init(|| Mutex::new(()))
}

/// Copies `fixture_name` into a fresh tempdir with a matching anchor `.cc`
/// and a minimal `compile_commands.json`, then runs the generator against it.
/// Deliberately no `-isysroot` in the recorded flags (real compile_commands.json
/// entries from `/usr/bin/c++` don't carry one either — the driver injects it
/// implicitly when run as a process); this exercises
/// `clang_index::ensure_macos_sysroot`'s fallback rather than papering over it.
fn generate_fixture(fixture_name: &str, name: &str) -> (TempDir, GeneratedFiles) {
    generate_fixture_with_annotations(fixture_name, name, None)
}

/// Like `generate_fixture`, but also writes `annotations` (if given) to a
/// file in the same tempdir and passes it as `--annotations`.
fn generate_fixture_with_annotations(
    fixture_name: &str,
    name: &str,
    annotations: Option<&str>,
) -> (TempDir, GeneratedFiles) {
    let _guard = clang_test_lock().lock().unwrap_or_else(|e| e.into_inner());
    let dir = tempfile::tempdir().unwrap();
    let header_src = fixtures_dir().join(fixture_name);
    let header = dir.path().join(fixture_name);
    fs::copy(&header_src, &header).unwrap();

    let anchor_name = format!("{fixture_name}.anchor.cc");
    let anchor = dir.path().join(&anchor_name);
    fs::write(&anchor, format!("#include \"{fixture_name}\"\n")).unwrap();

    let mut arguments = vec!["c++".to_string(), "-std=c++23".to_string()];
    // Boost isn't always on the default search path (e.g. Homebrew on
    // Apple Silicon doesn't symlink into /usr/local/include); add common
    // locations if present. Real project builds pass their own -I via
    // compile_commands.json — this is only for this crate's own fixtures.
    for candidate in ["/opt/homebrew/include", "/usr/local/include"] {
        if std::path::Path::new(candidate)
            .join("boost/signals2/signal.hpp")
            .exists()
        {
            arguments.push(format!("-I{candidate}"));
        }
    }
    arguments.extend([
        "-c".to_string(),
        "-o".to_string(),
        format!("{anchor_name}.o"),
        anchor_name.clone(),
    ]);

    let compile_commands = dir.path().join("compile_commands.json");
    let db = serde_json::json!([{
        "directory": dir.path().to_string_lossy(),
        "file": anchor_name,
        "arguments": arguments,
    }]);
    fs::write(&compile_commands, serde_json::to_string(&db).unwrap()).unwrap();

    let external_annotations = annotations.map(|text| {
        let path = dir.path().join("annotations.rpc");
        fs::write(&path, text).unwrap();
        path
    });

    let out_dir = dir.path().join("out");
    let opts = GenerateOptions {
        header,
        anchor_source: anchor,
        compile_commands,
        out_proto: out_dir.join(format!("{name}.proto")),
        out_adapter_hh: out_dir.join(format!("{name}ServiceImpl.hh")),
        out_adapter_cc: out_dir.join(format!("{name}ServiceImpl.cc")),
        proto_package: "workrave.rpc".to_string(),
        header_include: None,
        external_annotations,
    };

    let generated = generate(&opts).expect("generate should succeed");
    (dir, generated)
}

#[test]
fn generates_proto_and_adapter_for_simple_fixture() {
    let (_dir, generated) = generate_fixture("simple.hh", "RpcTest");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("service TestService"), "{proto}");
    assert!(proto.contains("message PingRequest"), "{proto}");
    assert!(proto.contains("string message = 1;"), "{proto}");
    assert!(proto.contains("message AddResponse"), "{proto}");
    assert!(proto.contains("int32 result = 1;"), "{proto}");
    assert!(proto.contains("enum TestMode"), "{proto}");
    assert!(proto.contains("TEST_MODE_IDLE = 0;"), "{proto}");
    assert!(proto.contains("message GreetRequest"), "{proto}");
    assert!(proto.contains("string name = 1;"), "{proto}");
    // Signal: empty subscribe Request (not keyed), Event with the default
    // single-argument field name "value", and a server-streaming rpc.
    assert!(
        proto.contains("message ModeChangedRequest {\n\n}"),
        "{proto}"
    );
    assert!(
        proto.contains("message ModeChangedEvent {\n\n  TestMode value = 1;"),
        "{proto}"
    );
    assert!(
        proto.contains("rpc ModeChanged(ModeChangedRequest) returns (stream ModeChangedEvent);"),
        "{proto}"
    );

    let header_out = fs::read_to_string(&generated.adapter_hh).unwrap();
    assert!(
        header_out.contains(
            "class TestServiceServiceImpl final : public ::workrave::rpc::TestService::Service"
        ),
        "{header_out}"
    );
    assert!(header_out.contains("RpcTestServer &impl"), "{header_out}");
    assert!(
        header_out.contains("#include \"rpc/EventQueue.hh\""),
        "{header_out}"
    );
    assert!(
        header_out
            .contains("::grpc::ServerWriter<::workrave::rpc::ModeChangedEvent> *writer) override;"),
        "{header_out}"
    );

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    // The .cc must #include the adapter header's real output name
    // (RpcTestServiceImpl.hh, from --out-adapter-hh), not a name derived
    // from the service ("TestService" -> "TestServiceServiceImpl.hh") — the
    // CMake NAME parameter and the C++ service name are independent.
    assert!(
        source_out.contains("#include \"RpcTestServiceImpl.hh\""),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.ping(request->message())"),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.add(request->a(), request->b())"),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.set_flag(request->value())"),
        "{source_out}"
    );
    assert!(
        source_out.contains("TestMode local_mode{};"),
        "{source_out}"
    );
    // `TestMode &mode` is a reference, not a pointer, so the call passes the
    // local directly — no `&` needed, references bind at the call site.
    assert!(
        source_out.contains("impl_.get_mode(local_mode)"),
        "{source_out}"
    );
    assert!(
        source_out.contains("static_cast<::workrave::rpc::TestMode>(local_mode)"),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.greet(request->name())"),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.signal_mode_changed().connect(\n    [&queue](TestMode value)"),
        "{source_out}"
    );
    assert!(
        source_out.contains("event.set_value(static_cast<::workrave::rpc::TestMode>(value));"),
        "{source_out}"
    );
    assert!(
        source_out.contains("queue.wait_and_pop(event, context)"),
        "{source_out}"
    );
}

#[test]
fn generates_keyed_service_with_instance_registry() {
    let (_dir, generated) = generate_fixture("keyed.hh", "RpcKeyed");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("service WidgetService"), "{proto}");
    assert!(proto.contains("enum WidgetId"), "{proto}");
    assert!(proto.contains("WIDGET_ID_FIRST = 0;"), "{proto}");
    // The `id` selector field must be field 1 in every request, ahead of the
    // method's own real parameters (SetValue's `v` becomes field 2, not 1).
    assert!(
        proto.contains("message GetValueRequest {\n\n  WidgetId id = 1;"),
        "{proto}"
    );
    assert!(
        proto.contains("message SetValueRequest {\n\n  WidgetId id = 1;\n\n  int32 v = 2;"),
        "{proto}"
    );

    let header_out = fs::read_to_string(&generated.adapter_hh).unwrap();
    assert!(
        header_out.contains("#include \"rpc/InstanceRegistry.hh\""),
        "{header_out}"
    );
    assert!(
        header_out.contains("::rpc::InstanceRegistry<WidgetId, RpcKeyedFixture> &registry_"),
        "{header_out}"
    );
    assert!(
        header_out.contains("explicit WidgetServiceServiceImpl(::rpc::InstanceRegistry<WidgetId, RpcKeyedFixture> &registry)"),
        "{header_out}"
    );

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    assert!(
        source_out
            .contains("auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));"),
        "{source_out}"
    );
    assert!(source_out.contains("impl_.get_value()"), "{source_out}");
    assert!(
        source_out.contains("impl_.set_value(request->v())"),
        "{source_out}"
    );

    // Signal on a keyed interface: the request carries `id` (to pick the
    // instance) but the resolve happens before connecting to the signal.
    assert!(
        proto.contains("message ValueChangedRequest {\n\n  WidgetId id = 1;"),
        "{proto}"
    );
    assert!(
        proto.contains("message ValueChangedEvent {\n\n  int32 value = 1;"),
        "{proto}"
    );
    assert!(
        proto.contains("rpc ValueChanged(ValueChangedRequest) returns (stream ValueChangedEvent);"),
        "{proto}"
    );
    assert!(
        source_out.contains(
            "auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));\n\n  ::rpc::EventQueue<::workrave::rpc::ValueChangedEvent> queue;"
        ),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.signal_value_changed().connect(\n    [&queue](int32_t value)"),
        "{source_out}"
    );
    assert!(
        source_out.contains("event.set_value(value);"),
        "{source_out}"
    );
}

/// `@rpc.enum(name="...")`/`@rpc.enum.value(name="...")` (see
/// fixtures/enum_names.hh) pin an explicit, backend-agnostic name for an
/// enum type/value, independent of the auto-derived protobuf enum name the
/// gRPC backend keeps using regardless — e.g. matching
/// workrave-service.xml's `<enum name="operation_mode">`/
/// `<value name="normal">` for a future DBus backend. Not consumed by
/// anything today; surfaced as a `.proto` comment so it's visible/testable
/// without that backend existing yet.
#[test]
fn pins_canonical_enum_and_value_names() {
    let (_dir, generated) = generate_fixture("enum_names.hh", "RpcEnumNames");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("// canonical_name: \"operation_mode\""), "{proto}");
    assert!(proto.contains("enum OperationMode {"), "{proto}");
    assert!(proto.contains("// canonical_name: \"normal\""), "{proto}");
    assert!(
        proto.contains("// canonical_name: \"normal\"\n\n  OPERATION_MODE_NORMAL = 0;"),
        "{proto}"
    );
    assert!(proto.contains("// canonical_name: \"suspended\""), "{proto}");
    // Quiet has no tag — the auto-derived protobuf name still appears, but
    // with no canonical_name comment attached to it.
    assert!(
        !proto.contains("canonical_name: \"quiet\""),
        "{proto}"
    );
    assert!(proto.contains("OPERATION_MODE_QUIET = 2;"), "{proto}");
}

/// Same as `pins_canonical_enum_and_value_names`, but supplied entirely via
/// an external annotations file instead of comments in the header — proves
/// `@rpc.enum`/`@rpc.enum.value` work out-of-band too, the case that
/// actually matters for a header nothing is allowed to touch.
#[test]
fn pins_canonical_enum_names_via_external_annotations() {
    let annotations = r#"
[OperationMode]
@rpc.enum(name="operation_mode")

[OperationMode::Normal]
@rpc.enum.value(name="normal")
"#;
    let (_dir, generated) = generate_fixture_with_annotations(
        "enum_names_unannotated.hh",
        "RpcEnumNamesExternal",
        Some(annotations),
    );

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("// canonical_name: \"operation_mode\""), "{proto}");
    assert!(
        proto.contains("// canonical_name: \"normal\"\n\n  OPERATION_MODE_NORMAL = 0;"),
        "{proto}"
    );
}

/// `unannotated.hh` (see fixtures/unannotated.hh) carries no `@rpc` tags at
/// all in the header itself — every tag comes from a separate annotations
/// file, keyed by fully-qualified name, exactly the mechanism needed for a
/// header nothing is allowed to modify.
#[test]
fn generates_service_from_external_annotations_only() {
    let annotations = r#"
# Class-level tag, keyed by fully-qualified name (no parens).
[testutil::RpcUnannotatedFixture]
@rpc(service="UnannotatedService")

# Method-level tag, keyed by Class::method(ParamType1,ParamType2) to
# disambiguate overloads. Whitespace inside the key doesn't matter.
[testutil::RpcUnannotatedFixture::ping( std::string )]
@rpc(name="Ping")

[testutil::RpcUnannotatedFixture::add(int32_t,int32_t)]
@rpc(name="Add")
"#;

    let (_dir, generated) =
        generate_fixture_with_annotations("unannotated.hh", "RpcExternal", Some(annotations));

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("service UnannotatedService"), "{proto}");
    assert!(
        proto.contains("rpc Ping(PingRequest) returns (PingResponse);"),
        "{proto}"
    );
    assert!(
        proto.contains("rpc Add(AddRequest) returns (AddResponse);"),
        "{proto}"
    );

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    assert!(
        source_out.contains("impl_.ping(request->message())"),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.add(request->a(), request->b())"),
        "{source_out}"
    );
}

/// `std::map<K, V>` (the gRPC analog of dbusgen.py's `<dictionary>` — a
/// protobuf `map<K, V>` field, not `repeated`) with both a scalar value
/// (`SetCounters`) and a struct value (`GetMenuByAction`, exercising the
/// same field-by-field recursion a struct return value uses, just reached
/// through `(*mutable_field())[key]` instead of `mutable_field()`).
#[test]
fn generates_map_marshalling() {
    let (_dir, generated) = generate_fixture("map_types.hh", "RpcMapTypes");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(
        proto.contains("message SetCountersRequest {\n\n  map<string, int32> counters = 1;"),
        "{proto}"
    );
    assert!(
        proto.contains("message GetMenuByActionResponse {\n\n  map<string, MenuItem> out = 1;"),
        "{proto}"
    );

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    // Scalar-valued map in-parameter: real std::map built by looping map
    // entries (`.first`/`.second`, same as google::protobuf::Map's own
    // iterator shape).
    assert!(
        source_out.contains("std::map<std::string, int> local_counters{};"),
        "{source_out}"
    );
    assert!(
        source_out.contains(
            "for (const auto &rpc_kv_0 : request->counters()) { int32_t rpc_val_0{}; rpc_val_0 = rpc_kv_0.second; local_counters.emplace(rpc_kv_0.first, rpc_val_0); }"
        ),
        "{source_out}"
    );
    // Struct-valued map out-parameter: `(*mutable_field())[key]` returns a
    // reference (not pointer) to the value message, populated field-by-field.
    assert!(
        source_out.contains("std::map<std::string, MenuItem> local_out{};"),
        "{source_out}"
    );
    assert!(
        source_out.contains(
            "for (const auto &rpc_kv_0 : local_out) { auto &rpc_map_val_0 = (*response->mutable_out())[rpc_kv_0.first]; rpc_map_val_0.set_text(rpc_kv_0.second.text); rpc_map_val_0.set_command(rpc_kv_0.second.command); }"
        ),
        "{source_out}"
    );
}

/// protobuf only allows bool/integer/string map keys — an illegal key type
/// (float/double, an enum, a struct, a container) must fail loudly at
/// generation time, not silently produce a broken `.proto`.
#[test]
fn rejects_illegal_map_key_type() {
    let dir = tempfile::tempdir().unwrap();
    let header = dir.path().join("bad_key.hh");
    fs::write(
        &header,
        r#"
#pragma once
#include <map>

// @rpc(service="BadKeyService")
class RpcBadKeyFixture
{
public:
  // @rpc(name="SetScores")
  void set_scores(std::map<double, int> scores);
};
"#,
    )
    .unwrap();
    let anchor = dir.path().join("anchor.cc");
    fs::write(&anchor, "#include \"bad_key.hh\"\n").unwrap();
    let compile_commands = dir.path().join("compile_commands.json");
    let db = serde_json::json!([{
        "directory": dir.path().to_string_lossy(),
        "file": "anchor.cc",
        "arguments": ["c++", "-std=c++23", "-c", "-o", "anchor.cc.o", "anchor.cc"],
    }]);
    fs::write(&compile_commands, serde_json::to_string(&db).unwrap()).unwrap();

    let _guard = clang_test_lock().lock().unwrap_or_else(|e| e.into_inner());
    let out_dir = dir.path().join("out");
    let opts = GenerateOptions {
        header,
        anchor_source: anchor,
        compile_commands,
        out_proto: out_dir.join("BadKey.proto"),
        out_adapter_hh: out_dir.join("BadKeyServiceImpl.hh"),
        out_adapter_cc: out_dir.join("BadKeyServiceImpl.cc"),
        proto_package: "workrave.rpc".to_string(),
        header_include: None,
        external_annotations: None,
    };

    let err = generate(&opts).expect_err("a double map key must be rejected");
    assert!(
        format!("{err:#}").contains("isn't a protobuf-legal map key"),
        "{err:#}"
    );
}

/// A plain struct/class value type (`TimerData`/`MenuItem`) used as an
/// in-parameter, a return value, an out-parameter, and a signal field, plus
/// `std::vector<T>`/`std::list<T>` sequences of both scalar and struct
/// elements — the capabilities ui/app/workrave-gui.xml's DBus interface
/// needs (TimerData, MenuItem, MenuItems, TimersUpdated/MenuUpdated) that
/// weren't supported before. Generated marshalling code is verified to
/// actually *compile* for this fixture, not just pattern-matched as text —
/// see the `compiles_with_real_protoc_and_grpc_headers` test below, which
/// runs a real protoc + clang++ over exactly this fixture's output.
#[test]
fn generates_struct_and_sequence_marshalling() {
    let (_dir, generated) = generate_fixture("struct_sequence.hh", "RpcStructSeq");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(
        proto.contains("message TimerData {\n\n  string bar_text = 1;\n\n  int32 slot = 2;\n\n  uint32 bar_primary_val = 3;"),
        "{proto}"
    );
    assert!(
        proto.contains("message MenuItem {\n\n  string text = 1;\n\n  uint32 command = 2;\n\n  uint32 flags = 3;"),
        "{proto}"
    );
    // Struct as in-parameter / return value.
    assert!(proto.contains("message SetTimerDataRequest {\n\n  TimerData data = 1;"), "{proto}");
    assert!(proto.contains("message GetTimerDataResponse {\n\n  TimerData result = 1;"), "{proto}");
    // Sequence-of-struct out-parameter (the `MenuItems` alias resolves to
    // `repeated MenuItem`, same as if `std::list<MenuItem>` had been
    // spelled directly).
    assert!(proto.contains("message GetMenuResponse {\n\n  repeated MenuItem out = 1;"), "{proto}");
    // Sequence of scalar.
    assert!(proto.contains("message SetTagsRequest {\n\n  repeated int32 tags = 1;"), "{proto}");
    // Signal field of struct type.
    assert!(proto.contains("message TimerUpdatedEvent {\n\n  TimerData value = 1;"), "{proto}");

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    // Struct in-parameter: field-by-field construction from the request.
    assert!(source_out.contains("TimerData local_data{};"), "{source_out}");
    assert!(source_out.contains("local_data.bar_text = request->data().bar_text();"), "{source_out}");
    assert!(source_out.contains("impl_.set_timer_data(local_data);"), "{source_out}");
    // Struct return value: field-by-field write via mutable_result().
    assert!(source_out.contains("auto *rpc_msg_0 = response->mutable_result();"), "{source_out}");
    assert!(source_out.contains("rpc_msg_0->set_bar_text(rpc_result.bar_text);"), "{source_out}");
    // Sequence-of-struct out-parameter: real C++ container declared and
    // passed by reference (not by pointer — `MenuItems &out` binds directly),
    // then a real `for` loop populates the repeated field element-by-element.
    assert!(source_out.contains("std::list<MenuItem> local_out{};"), "{source_out}");
    assert!(source_out.contains("impl_.get_menu(local_out);"), "{source_out}");
    assert!(
        source_out.contains(
            "for (const auto &rpc_item_0 : local_out) { auto *rpc_elem_0 = response->add_out(); rpc_elem_0->set_text(rpc_item_0.text);"
        ),
        "{source_out}"
    );
    // Sequence of scalar in-parameter: real container built by looping the
    // repeated field.
    assert!(
        source_out.contains(
            "for (const auto &rpc_wire_0 : request->tags()) { int32_t rpc_item_0{}; rpc_item_0 = rpc_wire_0; local_tags.push_back(rpc_item_0); }"
        ),
        "{source_out}"
    );
    // Signal field of struct type: the lambda receives the real struct by
    // value and writes it field-by-field into the event.
    assert!(
        source_out.contains("[&queue](TimerData value)"),
        "{source_out}"
    );
    assert!(
        source_out.contains("auto *rpc_msg_0 = event.mutable_value();"),
        "{source_out}"
    );
    assert!(
        source_out.contains("rpc_msg_0->set_bar_text(value.bar_text);"),
        "{source_out}"
    );
}

/// The struct-nesting equivalent of `generates_struct_and_sequence_marshalling`:
/// `TimerData`/`MenuItem` are nested *inside* the annotated class itself
/// (`RpcNestedFixture::TimerData`), matching the real
/// ui/app/GenericDBusApplet.hh layout exactly. This is the case that
/// surfaced a real bug: `Type::get_display_name()` prints a class-nested
/// type the way it appears from its own declaration's lexical scope (bare
/// "TimerData"), which doesn't resolve in generated code living outside
/// that class — confirmed by an actual failed compile before the fix (clang
/// itself suggested the exact qualification needed). See
/// `qualified_type_spelling()` in clang_index.rs.
#[test]
fn qualifies_class_nested_struct_types() {
    let (_dir, generated) = generate_fixture("nested_struct.hh", "RpcNested");

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    assert!(
        source_out.contains("RpcNestedFixture::TimerData local_data{};"),
        "{source_out}"
    );
    assert!(
        source_out.contains("std::list<RpcNestedFixture::MenuItem> local_out{};"),
        "{source_out}"
    );
}

/// Runs the exact output of `generates_struct_and_sequence_marshalling` and
/// `qualifies_class_nested_struct_types` through real `protoc` +
/// `grpc_cpp_plugin` + `clang++ -c`, the same tools CMake's
/// `rpc_generate_source()` uses — text-pattern assertions alone can't catch
/// a type that fails to resolve (exactly the class-nesting bug above), only
/// an actual compile can. Skips (doesn't fail) if `protoc`/`grpc_cpp_plugin`
/// aren't on PATH, since not every dev machine running `cargo test` has the
/// full C++ toolchain installed — the real build (`ninja`) still catches
/// regressions there.
#[test]
fn compiles_with_real_protoc_and_grpc_headers() {
    use std::process::Command;

    let Ok(protoc) = which("protoc") else {
        eprintln!("skipping: protoc not on PATH");
        return;
    };
    let Ok(grpc_plugin) = which("grpc_cpp_plugin") else {
        eprintln!("skipping: grpc_cpp_plugin not on PATH");
        return;
    };

    for (fixture, name) in [
        ("struct_sequence.hh", "RpcStructSeqCompile"),
        ("nested_struct.hh", "RpcNestedCompile"),
        ("map_types.hh", "RpcMapTypesCompile"),
    ] {
        let (dir, generated) = generate_fixture(fixture, name);
        let out_dir = generated.proto.parent().unwrap();

        let status = Command::new(&protoc)
            .arg(format!("--cpp_out={}", out_dir.display()))
            .arg(format!("--grpc_out={}", out_dir.display()))
            .arg(format!("--plugin=protoc-gen-grpc={}", grpc_plugin.display()))
            .arg("-I")
            .arg(out_dir)
            .arg(&generated.proto)
            .status()
            .expect("failed to run protoc");
        assert!(status.success(), "protoc failed for {fixture}");

        let rpc_include = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("../include")
            .canonicalize()
            .expect("libs/rpc/include must exist");
        let boost_include = ["/opt/homebrew/include", "/usr/local/include"]
            .into_iter()
            .find(|c| std::path::Path::new(c).join("boost/signals2/signal.hpp").exists());
        let grpc_include = ["/opt/homebrew/include", "/usr/local/include", "/usr/include"]
            .into_iter()
            .find(|c| std::path::Path::new(c).join("grpcpp/grpcpp.h").exists());

        let mut cmd = Command::new("clang++");
        cmd.arg("-std=c++23")
            .arg("-I")
            .arg(out_dir)
            .arg("-I")
            .arg(dir.path())
            .arg("-I")
            .arg(&rpc_include)
            .arg("-c")
            .arg(&generated.adapter_cc)
            .arg("-o")
            .arg(out_dir.join(format!("{name}.o")));
        if let Some(inc) = boost_include {
            cmd.arg("-I").arg(inc);
        }
        if let Some(inc) = grpc_include {
            cmd.arg("-I").arg(inc);
        }

        let output = cmd.output().expect("failed to run clang++");
        assert!(
            output.status.success(),
            "clang++ failed to compile generated adapter for {fixture}:\n{}",
            String::from_utf8_lossy(&output.stderr)
        );
    }
}

fn which(program: &str) -> Result<PathBuf, ()> {
    std::env::var_os("PATH")
        .and_then(|paths| {
            std::env::split_paths(&paths).find_map(|dir| {
                let candidate = dir.join(program);
                candidate.is_file().then_some(candidate)
            })
        })
        .ok_or(())
}

/// A `std::chrono::duration<>` parameter (any Rep/Period, no annotation
/// needed) and a `Flags<Enum>` parameter (gated on the `@rpc.bitmask` tag on
/// the `Flags` template itself, see duration_flags.hh) — the two cases
/// Core::set_operation_mode_for/force_break needed and were originally
/// excluded from annotation over, before this support existed.
#[test]
fn generates_duration_and_bitmask_parameters() {
    let (_dir, generated) = generate_fixture("duration_flags.hh", "RpcDurationFlags");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("enum Perm"), "{proto}");
    assert!(proto.contains("PERM_READ = 0;"), "{proto}");
    // Duration is wire-encoded as a plain string, not a raw integer.
    assert!(
        proto.contains("message SetTimeoutRequest {\n\n  string duration = 1;"),
        "{proto}"
    );
    // Bitmask is wire-encoded as `repeated Enum`, not a raw integer.
    assert!(
        proto.contains("message SetPermissionsRequest {\n\n  repeated Perm perms = 1;"),
        "{proto}"
    );

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    assert!(
        source_out.contains("#include \"rpc/Duration.hh\""),
        "{source_out}"
    );
    assert!(
        source_out.contains(
            "impl_.set_timeout(std::chrono::duration_cast<std::chrono::minutes>(rpc::parse_duration(request->duration())));"
        ),
        "{source_out}"
    );
    assert!(
        source_out.contains("testutil::Flags<testutil::Perm> local_perms;"),
        "{source_out}"
    );
    assert!(
        source_out.contains(
            "for (int i = 0; i < request->perms_size(); ++i) { local_perms |= static_cast<testutil::Perm>(request->perms(i)); }"
        ),
        "{source_out}"
    );
    assert!(
        source_out.contains("impl_.set_permissions(local_perms);"),
        "{source_out}"
    );
    // Every method body is wrapped so a malformed request (e.g. an
    // unparsable duration string) reaches the caller as a proper gRPC
    // status instead of crashing the server.
    assert!(
        source_out.contains("catch (const std::exception &e)\n    {\n      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());"),
        "{source_out}"
    );
}

/// `//!`/`///`/`/** */` are Doxygen's "this is documentation" markers; @rpc
/// tags must be recognized from plain, non-documentation comments too (see
/// -fparse-all-comments in clang_index::parse_unit) so annotating a method
/// doesn't force its comment to masquerade as public API docs.
#[test]
fn recognizes_tags_in_plain_non_doc_comments() {
    let (_dir, generated) = generate_fixture("plain_comments.hh", "RpcPlain");

    let proto = fs::read_to_string(&generated.proto).unwrap();
    assert!(proto.contains("service PlainService"), "{proto}");
    assert!(proto.contains("rpc GetValue(GetValueRequest) returns (GetValueResponse);"), "{proto}");

    let source_out = fs::read_to_string(&generated.adapter_cc).unwrap();
    assert!(source_out.contains("impl_.get_value()"), "{source_out}");
}
