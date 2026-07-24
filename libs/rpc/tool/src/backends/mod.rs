//! One directory per codegen target, each implementing [`crate::backend::Backend`].
//! `lib.rs::generate()` is the only place that knows both of these exist —
//! nothing in here references the other.

pub mod dbus;
pub mod grpc;
