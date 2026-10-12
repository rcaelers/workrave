#!/usr/bin/env python3
# Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""Checks for transparent capture processing; needs Pillow and NumPy."""
import importlib.util
import unittest
from pathlib import Path

from PIL import Image

spec = importlib.util.spec_from_file_location("preview", Path(__file__).with_name("window-preview.py"))
preview = importlib.util.module_from_spec(spec)
spec.loader.exec_module(preview)


class TransparentCaptureTests(unittest.TestCase):
    def test_green_controls_and_antialiased_outline_survive(self):
        foreground = Image.new("RGBA", (16, 16))
        for x in range(5, 11):
            for y in range(5, 11):
                foreground.putpixel((x, y), (0, 255, 0, 255))
        foreground.putpixel((4, 5), (0, 255, 0, 128))
        foreground.putpixel((12, 8), (0, 0, 0, 32))
        mattes = [Image.alpha_composite(Image.new("RGBA", foreground.size, color), foreground).convert("RGB")
                  for color in ["#00ff00", "#ff00ff"]]
        result = preview.recover_alpha(*mattes)
        self.assertEqual(result.size, (11, 10))
        self.assertEqual(result.getpixel((3, 2)), (0, 255, 0, 255))
        self.assertEqual(result.getpixel((2, 2)), (0, 255, 0, 128))
        self.assertEqual(result.getchannel("A").getbbox(), (2, 2, 9, 8))

    def test_shadow_removal_preserves_native_rgb(self):
        image = Image.new("RGBA", (12, 12), (30, 40, 50, 32))
        image.paste((60, 70, 80, 255), (4, 4, 8, 8))
        result = preview.remove_window_shadows(image)
        self.assertEqual(result.convert("RGB").tobytes(), image.convert("RGB").tobytes())
        self.assertEqual(result.getpixel((1, 1))[3], 0)
        self.assertEqual(result.getpixel((4, 4))[3], 255)

    def test_fullscreen_translucent_blocking_backdrop_survives(self):
        image = Image.new("RGBA", (12, 12), (0, 0, 0, 128))
        image.paste((240, 240, 240, 255), (4, 4, 8, 8))
        self.assertEqual(preview.remove_window_shadows(image).tobytes(), image.tobytes())

    def test_empty_capture_is_reported(self):
        with self.assertRaisesRegex(RuntimeError, "Capture was empty"):
            preview.recover_alpha(Image.new("RGB", (8, 8), "#00ff00"),
                                  Image.new("RGB", (8, 8), "#ff00ff"))


if __name__ == "__main__":
    unittest.main()
