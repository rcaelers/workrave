//! Both package backends feed one SPDX/CSV inventory with consistent FetchContent metadata.
use serde_json::{json, Value};
use std::{fs, process::Command};

#[test]
fn conan_and_msys_share_the_same_external_package_inventory() {
    let root = std::env::temp_dir().join(format!("ship-sbom-test-{}", std::process::id()));
    fs::create_dir_all(root.join("sdk")).unwrap();
    fs::write(
        root.join("CMakeLists.txt"),
        "set(WORKRAVE_VERSION \"1.12.0\")\n",
    )
    .unwrap();
    fs::write(
        root.join("external.csv"),
        "unfold,1.2.3,MIT,\"A library, with \"\"quoted\"\" metadata\",https://example.org\r\n",
    )
    .unwrap();
    fs::write(
        root.join("msys.tsv"),
        "boost\t1.91.0\tBSL-1.0\tC++ libraries\thttps://boost.org\n",
    )
    .unwrap();
    fs::write(root.join("sdk/manifest.json"),json!({"llvm_mingw_release":"20260922","packages":[{
        "name":"boost","version":"1.91.0","license":"BSL-1.0","description":"C++ libraries","homepage":"https://boost.org",
        "ref":"boost/1.91.0#recipe","package_id":"id","prev":"binary"}]}).to_string()).unwrap();
    for (mode, file) in [("msys", root.join("msys.tsv")), ("sdk", root.join("sdk"))] {
        let status = Command::new(env!("CARGO_BIN_EXE_ship"))
            .arg("sbom")
            .arg(format!("--{mode}"))
            .arg(file)
            .arg("--external")
            .arg(root.join("external.csv"))
            .arg("--source")
            .arg(&root)
            .arg("--output")
            .arg(root.join(mode).join("output"))
            .status()
            .unwrap();
        assert!(status.success());
    }
    let documents: Vec<Value> = ["msys", "sdk"]
        .iter()
        .map(|mode| {
            serde_json::from_str(
                &fs::read_to_string(root.join(mode).join("output/sbom.spdx.json")).unwrap(),
            )
            .unwrap()
        })
        .collect();
    for document in &documents {
        assert_eq!(
            document["documentDescribes"],
            json!(["SPDXRef-Package-Workrave"])
        );
        let packages = document["packages"].as_array().unwrap();
        let external = packages
            .iter()
            .find(|package| package["name"] == "unfold")
            .unwrap();
        assert_eq!(external["summary"], "A library, with \"quoted\" metadata");
        assert!(document["relationships"].as_array().unwrap().iter().any(
            |relationship| relationship["relatedSpdxElement"] == external["SPDXID"]
                && relationship["relationshipType"] == "DEPENDS_ON"
        ));
    }
    assert!(documents[1]["packages"]
        .as_array()
        .unwrap()
        .iter()
        .any(|package| package["name"] == "boost"
            && package["sourceInfo"] == "Conan boost/1.91.0#recipe; package id#binary"));
    assert!(fs::read_to_string(root.join("msys/output/sbom.csv"))
        .unwrap()
        .contains("\"A library, with \"\"quoted\"\" metadata\""));
    fs::remove_dir_all(root).unwrap();
}
