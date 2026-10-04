#!/usr/bin/env python3
"""Store Breakpad symbols for the native build's PDBs before its container exits."""

import argparse
import json
from pathlib import Path
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path)
    parser.add_argument("--sdk", type=Path, help="Include the Conan dependencies' matching PDBs")
    args = parser.parse_args()
    tool = args.build / "_deps/dump_syms-src/dump_syms.exe"
    if not tool.is_file():
        raise RuntimeError("Crashpad build has no dump_syms executable")
    pdbs = sorted(args.build.rglob("*.pdb"))
    if not pdbs:
        raise RuntimeError("Crashpad build produced no PDBs")
    for pdb in pdbs:
        # Ignore configure-time compiler probes and third-party tool downloads.
        if "CMakeFiles" in pdb.parts or "dump_syms-src" in pdb.parts:
            continue
        subprocess.run([str(tool), "--store", str(args.build / "symbols"), str(pdb)], check=True)
    # Verify both shipping executables resolve their exact PDB and retain unwind data.
    for binary in ("workrave.exe", "WorkraveCrashHandler.exe"):
        candidates = [p for p in args.build.rglob(binary) if "CMakeFiles" not in p.parts]
        if not candidates:
            raise RuntimeError(f"Missing {binary} in Crashpad build")
        subprocess.run([str(tool), "--check-cfi", "--store", str(args.build / "symbols"),
                        str(candidates[0])], check=True)
    if args.sdk:
        for manifest in sorted((args.sdk / "packages").glob("*/symbols/manifest.json")):
            package = manifest.parent.parent
            for entry in json.loads(manifest.read_text()):
                subprocess.run([str(tool), "--store", str(args.build / "symbols"),
                                str(package / entry["pdb"])], check=True)


if __name__ == "__main__":
    main()
