#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Wrap the GTK menu's original PNG artwork in Windows ICO containers.

The PNG pixels are preserved unchanged, including their alpha channel.
Adwaita artwork attribution and license are in ../adwaita/.
"""

from pathlib import Path
import struct


def main():
    destination = Path(__file__).resolve().parent
    images = destination.parent
    sources = {
        "open": "adwaita/32x32/document-open.png",
        "preferences": "adwaita/32x32/preferences-system.png",
        "rest-break": "timer-rest-break-large.png",
        "about": "adwaita/32x32/help-about.png",
        "quit": "adwaita/32x32/application-exit.png",
    }
    for name, source in sources.items():
        png = (images / source).read_bytes()
        if png[:8] != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"Not a PNG: {source}")
        width, height = struct.unpack_from(">II", png, 16)
        if not (1 <= width <= 256 and 1 <= height <= 256):
            raise ValueError(f"Invalid icon dimensions: {source}")
        header = struct.pack("<HHH", 0, 1, 1)
        entry = struct.pack("<BBBBHHII", width % 256, height % 256, 0, 0, 1, 32, len(png), 22)
        (destination / f"menu-{name}.ico").write_bytes(header + entry + png)


if __name__ == "__main__":
    main()
