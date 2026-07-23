use std::path::PathBuf;

use anyhow::Result;
use clap::Parser;

use clang_rpc_gen::{generate, GenerateOptions};

/// Generates a gRPC C++ service adapter from an annotated C++ header.
#[derive(Parser)]
#[command(name = "clang-rpc-gen")]
struct Cli {
    /// The annotated C++ header to parse.
    #[arg(long)]
    header: PathBuf,

    /// A .cc/.cpp translation unit that #includes `--header`, used to
    /// resolve its real compiler flags via `--compile-commands`.
    #[arg(long)]
    anchor_source: PathBuf,

    /// Path to a CMAKE_EXPORT_COMPILE_COMMANDS-generated compile_commands.json.
    #[arg(long)]
    compile_commands: PathBuf,

    /// Where to write the generated (disposable) .proto schema.
    #[arg(long)]
    out_proto: PathBuf,

    /// Where to write the generated ServiceImpl header.
    #[arg(long)]
    out_adapter_hh: PathBuf,

    /// Where to write the generated ServiceImpl source.
    #[arg(long)]
    out_adapter_cc: PathBuf,

    /// The proto package (dotted), e.g. "workrave.rpc".
    #[arg(long)]
    proto_package: String,

    /// Literal text for the generated `#include "..."` of the annotated
    /// header. Defaults to the header's file name.
    #[arg(long)]
    header_include: Option<String>,

    /// A file supplying `@rpc` tags by fully-qualified name, for
    /// declarations in `--header` that can't carry an annotation comment of
    /// their own. See external_annotations.rs for the file format.
    #[arg(long)]
    annotations: Option<PathBuf>,
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    let opts = GenerateOptions {
        header: cli.header,
        anchor_source: cli.anchor_source,
        compile_commands: cli.compile_commands,
        out_proto: cli.out_proto,
        out_adapter_hh: cli.out_adapter_hh,
        out_adapter_cc: cli.out_adapter_cc,
        proto_package: cli.proto_package,
        header_include: cli.header_include,
        external_annotations: cli.annotations,
    };

    let generated = generate(&opts)?;
    println!("generated {}", generated.proto.display());
    println!("generated {}", generated.adapter_hh.display());
    println!("generated {}", generated.adapter_cc.display());

    Ok(())
}
