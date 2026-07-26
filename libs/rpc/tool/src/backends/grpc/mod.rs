//! The gRPC target: renders a `.proto` schema (later fed to real `protoc` +
//! `grpc_cpp_plugin`, not by this crate) plus the `<Service>ServiceImpl`
//! adapter that calls straight into the real, unmodified, annotated C++.
//! Always active — unlike DBus, there's no opt-in flag for it.

mod cpp_gen;
mod proto_gen;

use std::path::PathBuf;

use anyhow::{Context, Result};

use crate::backend::{Backend, GeneratedFile};
use crate::template_model::GenerationModel;

pub struct GrpcBackend {
    pub out_proto: PathBuf,
    pub out_adapter_hh: PathBuf,
    pub out_adapter_cc: PathBuf,
    pub proto_package: String,
    pub adapter_namespace: Option<String>,
}

impl Backend for GrpcBackend {
    fn name(&self) -> &'static str {
        "grpc"
    }

    fn generate(
        &self,
        model: &GenerationModel,
        header_include: &str,
    ) -> Result<Vec<GeneratedFile>> {
        let proto_basename = self
            .out_proto
            .file_stem()
            .context("--out-proto has no file stem")?
            .to_string_lossy()
            .to_string();
        let adapter_header_filename = self
            .out_adapter_hh
            .file_name()
            .context("--out-adapter-hh has no file name")?
            .to_string_lossy()
            .to_string();

        let proto_text = proto_gen::render_proto(model, &self.proto_package)?;
        let adapter = cpp_gen::render_adapter(
            model,
            &self.proto_package,
            self.adapter_namespace.as_deref(),
            header_include,
            &proto_basename,
            &adapter_header_filename,
        )?;

        Ok(vec![
            GeneratedFile {
                path: self.out_proto.clone(),
                contents: proto_text,
            },
            GeneratedFile {
                path: self.out_adapter_hh.clone(),
                contents: adapter.header,
            },
            GeneratedFile {
                path: self.out_adapter_cc.clone(),
                contents: adapter.source,
            },
        ])
    }
}
