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
    pub out_types_proto: Option<PathBuf>,
    pub proto_types_package: Option<String>,
    pub grpc_services_namespace: Option<String>,
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

        let split_types = match (&self.out_types_proto, &self.proto_types_package) {
            (Some(path), Some(package)) => Some((path, package.as_str())),
            (None, None) => None,
            _ => {
                anyhow::bail!("--out-types-proto and --proto-types-package must be given together")
            }
        };
        let types_proto_filename = split_types
            .map(|(path, _)| {
                path.file_name()
                    .context("--out-types-proto has no file name")
                    .map(|name| name.to_string_lossy().to_string())
            })
            .transpose()?;

        let proto_text = proto_gen::render_service_proto(
            model,
            &self.proto_package,
            split_types.map(|(_, package)| package),
            types_proto_filename.as_deref(),
        )?;
        let adapter = cpp_gen::render_adapter(
            model,
            cpp_gen::RenderAdapterOptions {
                package: &self.proto_package,
                proto_types_package: split_types.map(|(_, package)| package),
                grpc_services_namespace: self.grpc_services_namespace.as_deref(),
                adapter_namespace: self.adapter_namespace.as_deref(),
                header_include,
                proto_basename: &proto_basename,
                adapter_header_filename: &adapter_header_filename,
            },
        )?;

        let mut files = vec![
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
        ];
        if let Some((path, package)) = split_types {
            files.push(GeneratedFile {
                path: path.clone(),
                contents: proto_gen::render_types_proto(model, package)?,
            });
        }
        Ok(files)
    }
}
