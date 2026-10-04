//! One SPDX/CSV writer for MSYS2 discovery data and Conan SDK manifests.

use std::collections::BTreeMap;
use std::fs;
use std::path::PathBuf;

use anyhow::{Context, Result};
use serde_json::{json, Value};

#[derive(Debug, clap::Args)]
pub struct SbomCommand {
    /// SDK manifest produced from the locked Conan graph.
    #[arg(long)]
    sdk: Option<PathBuf>,
    /// MSYS2 package inventory (one SPDX package object per line).
    #[arg(long)]
    packages: Option<PathBuf>,
    /// MSYS2 package metadata: name, version, license, description, URL (TSV).
    #[arg(long)]
    msys: Option<PathBuf>,
    /// CMake's FetchContent package inventory.
    #[arg(long)]
    external: Option<PathBuf>,
    #[arg(long)]
    source: PathBuf,
    #[arg(long)]
    output: PathBuf,
}

pub fn run(args: SbomCommand) -> Result<()> {
    let mut packages = BTreeMap::new();
    if let Some(path) = args.packages {
        for line in fs::read_to_string(path)?
            .lines()
            .filter(|line| !line.trim().is_empty())
        {
            let package: Value = serde_json::from_str(line)?;
            packages.insert(
                package["SPDXID"]
                    .as_str()
                    .context("package needs SPDXID")?
                    .to_owned(),
                package,
            );
        }
    }
    if let Some(path) = args.msys {
        for line in fs::read_to_string(path)?.lines() {
            let row: Vec<&str> = line.splitn(5, '\t').collect();
            if row.len() < 5 {
                continue;
            }
            let mut package = package(row[0], row[1], row[2], row[3], row[4]);
            package["downloadLocation"] =
                json!(format!("https://packages.msys2.org/packages/{}", row[0]));
            packages.insert(package["SPDXID"].as_str().unwrap().to_owned(), package);
        }
    }
    if let Some(sdk) = args.sdk {
        let manifest: Value =
            serde_json::from_str(&fs::read_to_string(sdk.join("manifest.json"))?)?;
        for node in manifest["packages"]
            .as_array()
            .context("SDK manifest needs packages")?
        {
            let raw_license = node["license"].as_str().unwrap_or("NOASSERTION");
            let mut package = package(
                node["name"].as_str().unwrap_or_default(),
                node["version"].as_str().unwrap_or_default(),
                raw_license,
                node["description"].as_str().unwrap_or_default(),
                node["homepage"].as_str().unwrap_or("NOASSERTION"),
            );
            // Conan lists do not say whether licenses are alternatives or
            // cumulative obligations. Preserve the metadata without guessing.
            if node["license"].is_array() {
                package["licenseComments"] = json!(node["license"].to_string());
            }
            package["sourceInfo"] = json!(format!(
                "Conan {}; package {}#{}",
                node["ref"].as_str().unwrap_or_default(),
                node["package_id"].as_str().unwrap_or_default(),
                node["prev"].as_str().unwrap_or_default()
            ));
            packages.insert(package["SPDXID"].as_str().unwrap().to_owned(), package);
        }
        let version = manifest["llvm_mingw_release"]
            .as_str()
            .unwrap_or("NOASSERTION");
        let runtime = package(
            "llvm-mingw-runtime",
            version,
            "NOASSERTION",
            "libc++, libunwind and winpthreads; licenses in runtime/LICENSE.TXT",
            "https://github.com/mstorsjo/llvm-mingw",
        );
        packages.insert(runtime["SPDXID"].as_str().unwrap().to_owned(), runtime);
    }
    if let Some(path) = args.external {
        for row in csv_rows(&fs::read_to_string(path)?) {
            if row.len() < 5 {
                continue;
            }
            let package = package(&row[0], &row[1], &row[2], &row[3], &row[4]);
            packages.insert(package["SPDXID"].as_str().unwrap().to_owned(), package);
        }
    }
    let source = fs::read_to_string(args.source.join("CMakeLists.txt"))?;
    let version = regex::Regex::new(r#"set\s*\(WORKRAVE_VERSION\s+"([^"]+)""#)?
        .captures(&source)
        .map(|capture| capture[1].to_owned())
        .unwrap_or_else(|| "UNKNOWN".into());
    let root = json!({"SPDXID":"SPDXRef-Package-Workrave", "name":"Workrave", "versionInfo":version,
        "supplier":"Organization: Workrave", "downloadLocation":"NOASSERTION", "filesAnalyzed":false,
        "homepage":"https://workrave.org", "licenseDeclared":"NOASSERTION", "licenseConcluded":"NOASSERTION",
        "primaryPackagePurpose":"APPLICATION", "summary":"Break reminder and RSI prevention tool"});
    let mut relationships = vec![
        json!({"spdxElementId":"SPDXRef-DOCUMENT", "relationshipType":"DESCRIBES", "relatedSpdxElement":"SPDXRef-Package-Workrave"}),
    ];
    for id in packages.keys() {
        relationships.push(json!({"spdxElementId":"SPDXRef-Package-Workrave", "relationshipType":"DEPENDS_ON", "relatedSpdxElement":id}));
    }
    let now = chrono::Utc::now();
    let mut all = vec![root];
    all.extend(packages.values().cloned());
    let document = json!({"spdxVersion":"SPDX-2.3", "dataLicense":"CC0-1.0", "SPDXID":"SPDXRef-DOCUMENT",
        "name":"workrave-sbom", "documentNamespace":format!("https://workrave.org/spdxdocs/{version}-{}", now.timestamp_nanos_opt().unwrap_or_default()),
        "creationInfo":{"created":now.format("%Y-%m-%dT%H:%M:%SZ").to_string(), "creators":["Tool: ship sbom"]},
        "documentDescribes":["SPDXRef-Package-Workrave"], "packages":all, "relationships":relationships});
    fs::create_dir_all(&args.output)?;
    fs::write(
        args.output.join("sbom.spdx.json"),
        serde_json::to_string_pretty(&document)? + "\n",
    )?;
    let mut csv = String::from("Package Name,Version,License,Description,URL\n");
    for package in packages.values() {
        let columns = [
            "name",
            "versionInfo",
            "licenseDeclared",
            "summary",
            "homepage",
        ];
        csv.push_str(
            &columns
                .iter()
                .map(|key| csv_field(package[key].as_str().unwrap_or_default()))
                .collect::<Vec<_>>()
                .join(","),
        );
        csv.push('\n');
    }
    fs::write(args.output.join("sbom.csv"), csv)?;
    tracing::info!("Generated SBOM: {} dependencies", packages.len());
    Ok(())
}

fn package(name: &str, version: &str, license: &str, description: &str, url: &str) -> Value {
    let id = regex::Regex::new(r"[^A-Za-z0-9.-]")
        .unwrap()
        .replace_all(&format!("{name}-{version}"), "-")
        .into_owned();
    let (declared, comment) = normalize_license(license);
    let mut package = json!({"SPDXID":format!("SPDXRef-Package-{id}"), "name":name, "versionInfo":version,
        "downloadLocation":if url.is_empty() || url == "Unknown" { "NOASSERTION" } else { url },
        "filesAnalyzed":false, "licenseConcluded":"NOASSERTION", "licenseDeclared":declared,
        "copyrightText":"NOASSERTION", "summary":description, "homepage":url});
    if !comment.is_empty() {
        package["licenseComments"] = json!(comment);
    }
    package
}

fn normalize_license(raw: &str) -> (String, String) {
    let license = raw.trim();
    if license.is_empty() {
        return ("NOASSERTION".into(), String::new());
    }
    let normalized = license
        .replace("documentation:spdx:", "")
        .replace("spdx:", "")
        .replace('/', " OR ");
    let normalized = normalized.split_whitespace().collect::<Vec<_>>().join(" ");
    let normalized = match normalized.as_str() {
        "GPL-2.0" => "GPL-2.0-only",
        "GPL-3.0" => "GPL-3.0-only",
        "LGPL-2.1" => "LGPL-2.1-only",
        "LGPL-3.0" => "LGPL-3.0-only",
        other => other,
    };
    let valid =
        regex::Regex::new(r"^[A-Za-z0-9.+()-]+( (AND|OR|WITH) [A-Za-z0-9.+()-]+)*$").unwrap();
    let normalized = if matches!(
        license,
        "Unknown" | "custom" | "LGPL" | "Microsoft-Web-WebView2"
    ) || license.starts_with("custom:")
        || !valid.is_match(normalized)
    {
        "NOASSERTION"
    } else {
        normalized
    };
    (
        normalized.into(),
        if normalized != license {
            license.into()
        } else {
            String::new()
        },
    )
}

fn csv_field(value: &str) -> String {
    format!("\"{}\"", value.replace('"', "\"\""))
}

/// Accept quoted commas, escaped quotes, CRLF and multiline descriptions.
fn csv_rows(text: &str) -> Vec<Vec<String>> {
    let mut rows = Vec::new();
    let mut row = Vec::new();
    let mut field = String::new();
    let mut quoted = false;
    let mut chars = text.chars().peekable();
    while let Some(ch) = chars.next() {
        match ch {
            '"' if quoted && chars.peek() == Some(&'"') => {
                field.push('"');
                chars.next();
            }
            '"' => quoted = !quoted,
            ',' if !quoted => row.push(std::mem::take(&mut field)),
            '\n' if !quoted => {
                row.push(std::mem::take(&mut field));
                rows.push(std::mem::take(&mut row));
            }
            '\r' if !quoted && chars.peek() == Some(&'\n') => {}
            other => field.push(other),
        }
    }
    if !field.is_empty() || !row.is_empty() {
        row.push(field);
        rows.push(row);
    }
    rows
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn csv_preserves_quoted_metadata() {
        assert_eq!(
            csv_rows("name,1,MIT,\"description, with \"\"quotes\"\"\",url\r\n"),
            vec![vec![
                "name",
                "1",
                "MIT",
                "description, with \"quotes\"",
                "url"
            ]]
        );
    }
    #[test]
    fn ambiguous_licenses_remain_visible() {
        assert_eq!(
            normalize_license("Microsoft-Web-WebView2"),
            ("NOASSERTION".into(), "Microsoft-Web-WebView2".into())
        );
        assert_eq!(
            normalize_license("GPL-2.0"),
            ("GPL-2.0-only".into(), "GPL-2.0".into())
        );
        assert_eq!(
            normalize_license("MIT/Apache-2.0"),
            ("MIT OR Apache-2.0".into(), "MIT/Apache-2.0".into())
        );
    }
}
