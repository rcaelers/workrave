"""Exercise the handoff with CTest, relocation and paths containing spaces."""

import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest


SCRIPT = Path(__file__).resolve().parents[1] / "windows-qt-handoff.py"
spec = importlib.util.spec_from_file_location("handoff", SCRIPT)
handoff = importlib.util.module_from_spec(spec)
spec.loader.exec_module(handoff)


@unittest.skipUnless(os.name != "nt" and shutil.which("cmake"), "POSIX executable fixture and CMake required")
class SbomComponentTest(unittest.TestCase):
    def test_qt_deployment_scans_application_source_instead_of_packaging_project(self):
        template = SCRIPT.parents[2] / "ui/app/toolkits/qt/dist/windows/WindeployQt.cmake.in"
        with tempfile.TemporaryDirectory(prefix="qt deployment ") as temporary:
            root = Path(temporary)
            recorded = root / "arguments.json"
            tool = root / "windeployqt"
            tool.write_text("#!" + sys.executable + "\nimport json,sys\n"
                            + "from pathlib import Path\nPath(" + repr(str(recorded))
                            + ").write_text(json.dumps(sys.argv[1:]))\n")
            tool.chmod(0o755)
            source = root / "application source"
            generated = root / "deploy.cmake"
            configure = root / "configure.cmake"
            variables = {"WINDEPLOYQT_EXECUTABLE": str(tool), "WORKRAVE_SOURCE_DIR": str(source),
                         "CMAKE_SOURCE_DIR": str(root / "packaging project"), "qtconf_dest_dir": "bin"}
            configure.write_text("\n".join("set(" + name + " " + handoff.bracket(value) + ")"
                                             for name, value in variables.items())
                                 + "\nconfigure_file(" + handoff.bracket(str(template)) + " "
                                 + handoff.bracket(str(generated)) + " @ONLY)\n")
            subprocess.run(["cmake", "-P", str(configure)], check=True)
            subprocess.run(["cmake", "-DCMAKE_INSTALL_PREFIX=" + str(root / "output"),
                            "-P", str(generated)], check=True)
            arguments = json.loads(recorded.read_text())
            self.assertEqual(arguments[arguments.index("--qmldir") + 1],
                             str(source / "ui/app/toolkits/qt/qml"))

    def test_separate_component_uses_configured_source_with_legacy_fallback(self):
        template = SCRIPT.parents[2] / "cmake/win32/GenerateSBOM.cmake.in"
        with tempfile.TemporaryDirectory(prefix="sbom component ") as temporary:
            root = Path(temporary)
            recorded = root / "arguments.json"
            tool = root / "ship"
            tool.write_text("#!" + sys.executable + "\nimport json,sys\n"
                            + "from pathlib import Path\nPath(" + repr(str(recorded))
                            + ").write_text(json.dumps(sys.argv[1:]))\n")
            tool.chmod(0o755)
            for override in ("", str(root / "application source")):
                with self.subTest(source_override=override):
                    legacy = str(root / "original source")
                    generated = root / "sbom.cmake"
                    configure = root / "configure.cmake"
                    variables = {"HAVE_SBOM": "ON", "WORKRAVE_SDK_DIR": str(root / "sdk"),
                                 "SHIP_EXECUTABLE": str(tool), "WORKRAVE_SOURCE_DIR": override,
                                 "CMAKE_SOURCE_DIR": legacy, "CMAKE_BINARY_DIR": str(root / "build")}
                    configure.write_text("\n".join("set(" + name + " " + handoff.bracket(value) + ")"
                                                     for name, value in variables.items())
                                         + "\nconfigure_file(" + handoff.bracket(str(template)) + " "
                                         + handoff.bracket(str(generated)) + " @ONLY)\n")
                    subprocess.run(["cmake", "-P", str(configure)], check=True)
                    # SOURCE_DIR is deliberately absent from this new process.
                    subprocess.run(["cmake", "-DCMAKE_INSTALL_PREFIX=" + str(root / "output"),
                                    "-P", str(generated)], check=True)
                    arguments = json.loads(recorded.read_text())
                    self.assertEqual(arguments[arguments.index("--source") + 1], override or legacy)


@unittest.skipUnless(shutil.which("cmake") and shutil.which("ctest"), "CMake/CTest required")
class HandoffTest(unittest.TestCase):
    def test_export_unpack_and_native_ctest_relocate_paths(self):
        with tempfile.TemporaryDirectory(prefix="qt handoff ") as temporary:
            root = Path(temporary)
            source, build, install = [root / ("original " + part) for part in ("source", "build", "output")]
            for path in (source, build, install):
                path.mkdir()
            working = build / "empty test directory"
            working.mkdir()
            (source / "value.txt").write_text("handoff")
            (install / "value.txt").write_text("handoff")
            if os.name != "nt":
                (build / "fixture.exe-1.0").write_bytes(b"staged executable")
                (build / "fixture.exe").symlink_to("fixture.exe-1.0")
            else:
                (build / "fixture.exe").write_bytes(b"staged executable")
            (build / "unused.obj").write_bytes(b"compiler object")
            (build / "windows-package.json").write_text(json.dumps({"version": "1.12.0-alpha.1", "channel": "alpha"}))
            command = [shutil.which("cmake"), "-E", "compare_files",
                       str(source / "value.txt"), str(install / "value.txt")]
            (build / "CTestTestfile.cmake").write_text(
                "add_test(" + " ".join(handoff.bracket(v) for v in ["relocated test", *command]) + ")\n"
                + "set_tests_properties(" + handoff.bracket("relocated test")
                + " PROPERTIES WORKING_DIRECTORY " + handoff.bracket(str(working)) + ")\n"
            )
            archive = root / "handoff.tar.gz"
            handoff.export(SimpleNamespace(source=source, build=build, install=install, archive=archive))
            relocated = root / "relocated"
            relocated.mkdir()
            destination = SimpleNamespace(source=relocated / "source", build=relocated / "build",
                                          install=relocated / "output", archive=archive, sdk=relocated / "sdk")
            destination.build.mkdir()
            shutil.copytree(source, destination.source)
            handoff.unpack(destination)
            self.assertTrue((destination.build / "fixture.exe").is_file())
            self.assertFalse((destination.build / "fixture.exe").is_symlink())
            self.assertEqual((destination.build / "fixture.exe").read_bytes(), b"staged executable")
            self.assertFalse((destination.build / "unused.obj").exists())
            shutil.rmtree(source)
            shutil.rmtree(build)
            shutil.rmtree(install)
            # The test must succeed after all original paths are gone.
            handoff.test(destination)
            self.assertEqual(len(json.loads((destination.build / "handoff.json").read_text())["tests"]), 1)


if __name__ == "__main__":
    unittest.main()
