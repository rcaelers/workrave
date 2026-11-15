#!/usr/bin/env python3
"""Transfer Linux-built Windows binaries to the native test/packaging job."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile


def export(args):
    tests = json.loads(subprocess.check_output(
        ["ctest", "--test-dir", str(args.build), "--show-only=json-v1"], text=True
    ))
    metadata = {
        "source": str(args.source), "build": str(args.build),
        "install": str(args.install), "tests": tests["tests"],
        "package": json.loads((args.build / "windows-package.json").read_text()),
    }
    args.archive.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=args.archive.parent) as temporary:
        pending = Path(temporary) / "handoff.tar.gz"
        # Windows must receive complete files, including versioned .exe aliases
        # whose symlink targets do not themselves have an .exe suffix.
        with tarfile.open(pending, "w:gz", compresslevel=1, dereference=True) as archive:
            archive.add(args.install, arcname="output")
            for path in sorted(args.build.rglob("*")):
                relative = path.relative_to(args.build)
                if not path.is_file() or any(p in ("CMakeFiles", "_deps") for p in relative.parts):
                    continue
                if path.suffix.lower() in (".exe", ".dll", ".sym") or path.name == "external-sbom.csv":
                    archive.add(path, arcname=str(Path("build") / relative))
            manifest = Path(temporary) / "handoff.json"
            manifest.write_text(json.dumps(metadata, indent=2))
            archive.add(manifest, arcname="build/handoff.json")
        pending.replace(args.archive)


def unpack(args):
    with tempfile.TemporaryDirectory(dir=args.build.parent) as temporary:
        temporary = Path(temporary)
        with tarfile.open(args.archive) as archive:
            archive.extractall(temporary, filter="data")
        for source, destination in ((temporary / "build", args.build), (temporary / "output", args.install)):
            shutil.copytree(source, destination, dirs_exist_ok=True)


def bracket(value):
    text = str(value)
    delimiter = "="
    while "]" + delimiter + "]" in text:
        delimiter += "="
    return "[" + delimiter + "[" + text + "]" + delimiter + "]"


def test(args):
    metadata = json.loads((args.build / "handoff.json").read_text())
    replacements = ((metadata["source"], args.source.as_posix()),
                    (metadata["build"], args.build.as_posix()),
                    (metadata["install"], args.install.as_posix()))

    def relocate(value):
        if isinstance(value, list):
            return ";".join(relocate(item) for item in value)
        value = str(value)
        for old, new in replacements:
            value = value.replace(old, new)
        return value

    lines = []
    for entry in metadata["tests"]:
        command = entry.get("command")
        if not command:
            raise RuntimeError(f"Test has no compiled command: {entry['name']}")
        command = [relocate(value) for value in command]
        if Path(command[0]).name == "cmake":
            command[0] = shutil.which("cmake")
        lines.append("add_test(" + " ".join(bracket(v) for v in [entry["name"], *command]) + ")")
        for prop in entry.get("properties", []):
            if prop["name"] == "WORKING_DIRECTORY":
                working = Path(relocate(prop["value"]))
                if working.is_relative_to(args.build):
                    working.mkdir(parents=True, exist_ok=True)
            lines.append("set_tests_properties(" + bracket(entry["name"]) + " PROPERTIES "
                         + bracket(prop["name"]) + " " + bracket(relocate(prop["value"])) + ")")
    (args.build / "CTestTestfile.cmake").write_text("\n".join(lines) + "\n")
    bins = [args.install / "bin", args.build / "bin",
            *sorted((args.sdk / "packages").glob("*/bin")), args.sdk / "runtime"]
    environment = dict(os.environ, PATH=os.pathsep.join(map(str, bins)) + os.pathsep + os.environ["PATH"])
    subprocess.run(["ctest", "--test-dir", str(args.build), "--output-on-failure"], env=environment, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("export", "unpack", "test"))
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--install", type=Path, required=True)
    parser.add_argument("--source", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--sdk", type=Path)
    args = parser.parse_args()
    args.build.mkdir(parents=True, exist_ok=True)
    globals()[args.action](args)


if __name__ == "__main__":
    main()
