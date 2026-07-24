use anyhow::{Context, Result};
use aws_credential_types::Credentials;
use aws_sdk_s3::config::{BehaviorVersion, Region};
use aws_sdk_s3::types::Object;
use aws_sdk_s3::Client;
use serde_json::Value;

pub struct S3Store {
    bucket: String,
    client: Client,
}

impl S3Store {
    pub fn new(endpoint: &str, bucket: &str, access_key_id: &str, secret_access_key: &str) -> Self {
        let creds = Credentials::new(access_key_id, secret_access_key, None, None, "ship");
        let config = aws_sdk_s3::Config::builder()
            .behavior_version(BehaviorVersion::latest())
            .region(Region::new("us-east-1"))
            .endpoint_url(endpoint)
            .credentials_provider(creds)
            .force_path_style(true)
            .build();
        let client = Client::from_conf(config);
        Self {
            bucket: bucket.to_string(),
            client,
        }
    }

    pub async fn list(&self, prefix: &str) -> Result<Vec<Object>> {
        let mut items = Vec::new();
        let mut marker: Option<String> = None;
        loop {
            let mut req = self.client.list_objects().bucket(&self.bucket);
            if !prefix.is_empty() {
                req = req.prefix(prefix);
            }
            if let Some(m) = marker.as_ref() {
                req = req.marker(m);
            }
            let resp = req
                .send()
                .await
                .with_context(|| format!("listing {prefix}"))?;
            if let Some(contents) = resp.contents {
                let last_key = contents.last().and_then(|o| o.key().map(|s| s.to_string()));
                items.extend(contents);
                if resp.is_truncated.unwrap_or(false) {
                    marker = last_key;
                    continue;
                }
            }
            break;
        }
        Ok(items)
    }

    pub async fn file_exists(&self, key: &str) -> Result<bool> {
        match self
            .client
            .head_object()
            .bucket(&self.bucket)
            .key(key)
            .send()
            .await
        {
            Ok(_) => Ok(true),
            Err(e) => {
                let svc = e.into_service_error();
                if svc.is_not_found() {
                    return Ok(false);
                }
                Err(anyhow::anyhow!("head_object failed: {svc}"))
            }
        }
    }

    pub async fn write(&self, key: &str, body: Vec<u8>, content_type: &str) -> Result<()> {
        self.client
            .put_object()
            .bucket(&self.bucket)
            .key(key)
            .body(body.into())
            .content_type(content_type)
            .send()
            .await
            .with_context(|| format!("put_object {key}"))?;
        Ok(())
    }

    pub async fn write_json(&self, key: &str, value: &Value) -> Result<()> {
        let body = serde_json::to_vec(value)?;
        self.write(key, body, "application/json").await
    }

    pub async fn read_json(&self, key: &str) -> Result<Value> {
        let resp = self
            .client
            .get_object()
            .bucket(&self.bucket)
            .key(key)
            .send()
            .await
            .with_context(|| format!("get_object {key}"))?;
        let bytes = resp.body.collect().await?.into_bytes();
        let v = serde_json::from_slice(&bytes).with_context(|| format!("parse JSON {key}"))?;
        Ok(v)
    }

    pub async fn delete_object(&self, key: &str) -> Result<()> {
        self.client
            .delete_object()
            .bucket(&self.bucket)
            .key(key)
            .send()
            .await
            .with_context(|| format!("delete_object {key}"))?;
        Ok(())
    }
}
