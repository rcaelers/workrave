//! Clients for the services a release talks to: the signing service,
//! GitHub, and the snapshots bucket (S3 storage, the artifact catalog and
//! the auto-update appcast generated from it).

pub mod appcast;
pub mod catalog;
pub mod github;
pub mod s3;
pub mod signing;
