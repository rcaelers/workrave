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

"""Drive Workrave's opt-in screenshot interface; capture real Windows UI to RGBA PNGs."""

import argparse
import ctypes
import json
import os
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class PreviewClient:
    def __init__(self, endpoint, proto_dir, timeout=30):
        import grpc
        from grpc_tools import protoc
        from google.protobuf.json_format import ParseDict
        self.generated = tempfile.TemporaryDirectory(prefix="workrave-preview-proto-")
        result = protoc.main(["protoc", "-I" + str(proto_dir),
                              "--python_out=" + self.generated.name,
                              "--grpc_python_out=" + self.generated.name,
                              str(proto_dir / "RpcWindowPreview.proto"),
                              str(proto_dir / "RpcWindowPreviewTypes.proto")])
        if result:
            raise RuntimeError("Could not generate the preview client bindings")
        sys.path.insert(0, self.generated.name)
        import RpcWindowPreview_pb2_grpc
        import RpcWindowPreviewTypes_pb2
        self.types = RpcWindowPreviewTypes_pb2
        self.parse = ParseDict
        self.channel = grpc.insecure_channel(endpoint)
        grpc.channel_ready_future(self.channel).result(timeout=timeout)
        self.stub = RpcWindowPreview_pb2_grpc.WindowPreviewServiceStub(self.channel)
        self.timeout = timeout
        self.has_wait = hasattr(self.types, "WaitForAppliedSequenceRequest")
        self.grpc = grpc

    def submit(self, method, values=None):
        request_type = getattr(self.types, method + "Request")
        request = self.parse(values or {}, request_type())
        return getattr(self.stub, method)(request, timeout=self.timeout).result

    def wait(self, sequence, after_sequence):
        if self.has_wait:
            try:
                error = self.submit("WaitForAppliedSequence", {"sequence": sequence, "after_sequence": after_sequence})
                if error:
                    raise RuntimeError(error)
                return
            except self.grpc.RpcError as error:
                if error.code() != self.grpc.StatusCode.UNIMPLEMENTED:
                    raise
                self.has_wait = False
        # Older preview builds still work, but cannot batch acknowledgements.
        deadline = time.monotonic() + self.timeout
        while self.submit("GetAppliedSequence") < sequence:
            if time.monotonic() > deadline:
                raise TimeoutError("UI did not apply the preview request")
            time.sleep(.002)
        error = self.submit("GetLastError")
        if error:
            raise RuntimeError(error)

    def call(self, method, values=None):
        result = self.submit(method, values)
        if not method.startswith("Get") and method not in ("WaitForAppliedSequence", "Quit"):
            self.wait(result, result - 1)
        return result

    def scene(self, scene):
        self.call("Hide")
        if self.has_wait and scene["calls"]:
            first = None
            for index, (method, arguments) in enumerate(scene["calls"]):
                last = self.submit(method, arguments)
                if first is None:
                    first = last
                if (index + 1) % 8 == 0 or index + 1 == len(scene["calls"]):
                    self.wait(last, first - 1)
                    first = None
        else:
            for method, arguments in scene["calls"]:
                self.call(method, arguments)


class WindowsCapture:
    """Only inventories and captures windows belonging to the selected Workrave PID."""
    def __init__(self, pid):
        if sys.platform != "win32":
            raise RuntimeError("Native capture runs on the Windows desktop. 'show' and 'call' work on any OS.")
        from ctypes import wintypes as w
        self.w = w
        self.user = ctypes.WinDLL("user32", use_last_error=True)
        self.gdi = ctypes.WinDLL("gdi32", use_last_error=True)
        self.dwm = ctypes.WinDLL("dwmapi", use_last_error=True)
        self.dwm.DwmFlush.restype = ctypes.c_long
        self.dwm.DwmSetWindowAttribute.argtypes = [w.HWND, w.DWORD, ctypes.c_void_p, w.DWORD]
        self.pid = pid
        self.user.SetProcessDpiAwarenessContext.argtypes = [ctypes.c_void_p]
        self.user.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4))
        self.user.GetDC.restype = w.HDC
        self.user.GetDC.argtypes = [w.HWND]
        self.user.GetWindowDC.restype = w.HDC
        self.user.GetWindowDC.argtypes = [w.HWND]
        self.user.ReleaseDC.argtypes = [w.HWND, w.HDC]
        self.user.GetWindowRect.argtypes = [w.HWND, ctypes.POINTER(w.RECT)]
        self.user.GetWindowThreadProcessId.argtypes = [w.HWND, ctypes.POINTER(w.DWORD)]
        self.user.IsWindowVisible.argtypes = [w.HWND]
        self.user.GetWindowTextW.argtypes = [w.HWND, w.LPWSTR, ctypes.c_int]
        self.user.GetClassNameW.argtypes = [w.HWND, w.LPWSTR, ctypes.c_int]
        self.user.GetDpiForWindow.argtypes = [w.HWND]
        self.user.SetWindowPos.argtypes = [w.HWND, w.HWND, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, w.UINT]
        self.user.RedrawWindow.argtypes = [w.HWND, ctypes.c_void_p, w.HRGN, w.UINT]
        self.gdi.CreateCompatibleDC.argtypes = [w.HDC]
        self.gdi.CreateCompatibleDC.restype = w.HDC
        self.gdi.CreateCompatibleBitmap.argtypes = [w.HDC, ctypes.c_int, ctypes.c_int]
        self.gdi.CreateCompatibleBitmap.restype = w.HBITMAP
        self.gdi.SelectObject.argtypes = [w.HDC, w.HGDIOBJ]
        self.gdi.SelectObject.restype = w.HGDIOBJ
        self.gdi.BitBlt.argtypes = [w.HDC, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, w.HDC, ctypes.c_int, ctypes.c_int, w.DWORD]
        self.gdi.GetDIBits.argtypes = [w.HDC, w.HBITMAP, w.UINT, w.UINT, ctypes.c_void_p, ctypes.c_void_p, w.UINT]
        self.gdi.DeleteObject.argtypes = [w.HGDIOBJ]
        self.gdi.DeleteDC.argtypes = [w.HDC]
        self.enum_proc = ctypes.WINFUNCTYPE(w.BOOL, w.HWND, w.LPARAM)
        self.user.EnumWindows.argtypes = [self.enum_proc, w.LPARAM]

    def windows(self, include_backdrop=False):
        result = []
        @self.enum_proc
        def visit(hwnd, parameter):
            owner = self.w.DWORD()
            self.user.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
            if owner.value == self.pid and self.user.IsWindowVisible(hwnd):
                title = ctypes.create_unicode_buffer(512)
                window_class = ctypes.create_unicode_buffer(256)
                self.user.GetWindowTextW(hwnd, title, len(title))
                self.user.GetClassNameW(hwnd, window_class, len(window_class))
                rect = self.w.RECT()
                self.user.GetWindowRect(hwnd, ctypes.byref(rect))
                if (include_backdrop or title.value != "Workrave screenshot backdrop") and rect.right - rect.left > 10 and rect.bottom - rect.top > 10 and rect.right > 0 and rect.bottom > 0:
                    result.append(dict(handle=int(hwnd), title=title.value, window_class=window_class.value,
                                       rect=[rect.left, rect.top, rect.right, rect.bottom],
                                       dpi=int(self.user.GetDpiForWindow(hwnd))))
            return True
        self.user.EnumWindows(visit, 0)
        return result

    def position_main(self, x=400, y=350):
        for window in self.windows():
            if window["title"] == "Workrave":
                self.user.SetWindowPos(window["handle"], None, x, y, 0, 0, 0x0015)  # no resize/z-order/activation

    def arrange(self):
        # Cover unrelated desktop windows with our own backdrop, then restore
        # the z-order of our visible previews. Never activate other applications.
        windows = self.windows(include_backdrop=True)
        disabled = self.w.BOOL(True)
        for window in windows:
            # DWMWA_TRANSITIONS_FORCEDISABLED: only our preview windows change.
            self.dwm.DwmSetWindowAttribute(window["handle"], 3, ctypes.byref(disabled), ctypes.sizeof(disabled))
        for window in reversed(windows):
            if window["title"] == "Workrave screenshot backdrop":
                self.user.SetWindowPos(window["handle"], self.w.HWND(-1), 0, 0, 0, 0, 0x0013)
        for window in reversed(windows):
            if window["title"] != "Workrave screenshot backdrop":
                self.user.SetWindowPos(window["handle"], self.w.HWND(-1), 0, 0, 0, 0, 0x0013)
        # Complete pending native paint messages before the compositor barrier.
        for window in windows:
            self.user.RedrawWindow(window["handle"], None, None, 0x0181)

    def next_frame(self):
        # Wait for the compositor, rather than guessing how long painting takes.
        if self.dwm.DwmFlush() != 0:
            time.sleep(1 / 60)

    def capture(self, rect, hwnd=None):
        from PIL import Image
        left, top, right, bottom = rect
        width, height = right - left, bottom - top
        class Header(ctypes.Structure):
            _fields_ = [("size", self.w.DWORD), ("width", self.w.LONG), ("height", self.w.LONG),
                        ("planes", self.w.WORD), ("bits", self.w.WORD), ("compression", self.w.DWORD),
                        ("image_size", self.w.DWORD), ("xppm", self.w.LONG), ("yppm", self.w.LONG),
                        ("colors", self.w.DWORD), ("important", self.w.DWORD)]
        screen = self.user.GetDC(None) if hwnd is None else self.user.GetWindowDC(hwnd)
        memory = self.gdi.CreateCompatibleDC(screen)
        bitmap = self.gdi.CreateCompatibleBitmap(screen, width, height)
        previous = self.gdi.SelectObject(memory, bitmap)
        try:
            # Include layered screen windows; an opaque native menu has its own DC.
            source_x, source_y = (left, top) if hwnd is None else (0, 0)
            flags = 0x40CC0020 if hwnd is None else 0x00CC0020
            if not self.gdi.BitBlt(memory, 0, 0, width, height, screen, source_x, source_y, flags):
                raise ctypes.WinError(ctypes.get_last_error())
            self.gdi.SelectObject(memory, previous)
            header = Header(ctypes.sizeof(Header), width, -height, 1, 32, 0, width * height * 4, 0, 0, 0, 0)
            buffer = ctypes.create_string_buffer(width * height * 4)
            if self.gdi.GetDIBits(memory, bitmap, 0, height, buffer, ctypes.byref(header), 0) != height:
                raise RuntimeError("Could not read the screenshot bitmap")
            return Image.frombytes("RGB", (width, height), buffer.raw, "raw", "BGRX")
        finally:
            self.gdi.SelectObject(memory, previous)
            self.gdi.DeleteObject(bitmap)
            self.gdi.DeleteDC(memory)
            self.user.ReleaseDC(hwnd, screen)


def remove_window_shadows(image):
    """Keep opaque UI and its one-pixel antialiased outline, excluding cast shadows."""
    from PIL import ImageChops, ImageFilter
    alpha = image.getchannel("A")
    # Retain full-screen translucent blocking backdrops.
    if all(alpha.getpixel(point) >= 128 for point in
           [(0, 0), (image.width - 1, 0), (0, image.height - 1), (image.width - 1, image.height - 1)]):
        return image.copy()
    outline = alpha.point(lambda value: 255 if value >= 250 else 0).filter(ImageFilter.MaxFilter(3))
    result = image.copy()
    result.putalpha(ImageChops.multiply(alpha, outline))
    return result


def pad_content(image):
    from PIL import Image
    bounds = image.getchannel("A").getbbox()
    if bounds is None:
        raise RuntimeError("Capture was empty; check the interactive Windows session")
    cropped = image.crop(bounds)
    padded = Image.new("RGBA", (cropped.width + 4, cropped.height + 4))
    padded.paste(cropped, (2, 2))
    return padded


def recover_alpha(green, magenta, keep_shadows=False):
    """Solve C = alpha*foreground + (1-alpha)*matte, preserving green UI content."""
    import numpy as np
    from PIL import Image, ImageChops
    # Qt uses transparent screen-sized windows. Find the foreground in native
    # Pillow code before allocating arrays, so only the actual card is processed.
    bounds = [ImageChops.difference(im, Image.new("RGB", im.size, color)).getbbox()
              for im, color in [(green, "#00ff00"), (magenta, "#ff00ff")]]
    bounds = [box for box in bounds if box is not None]
    if not bounds:
        raise RuntimeError("Capture was empty; check the interactive Windows session")
    box = (min(b[0] for b in bounds), min(b[1] for b in bounds),
           max(b[2] for b in bounds), max(b[3] for b in bounds))
    a = np.asarray(green.crop(box), dtype=np.int16)
    b = np.asarray(magenta.crop(box), dtype=np.int16)
    r, g, blue = b[..., 0] - a[..., 0], a[..., 1] - b[..., 1], b[..., 2] - a[..., 2]
    median = r + g + blue - np.minimum(np.minimum(r, g), blue) - np.maximum(np.maximum(r, g), blue)
    alpha = np.clip(255 - median, 0, 255).astype(np.uint8)
    # Remove rounding noise only in the background; retain antialiased edges and shadows.
    alpha[alpha < 2] = 0
    foreground = a.astype(np.float32)
    foreground[..., 1] -= 255 - alpha
    foreground *= 255 / np.maximum(alpha[..., None].astype(np.float32), 1)
    rgba = np.dstack([np.rint(np.clip(foreground, 0, 255)).astype(np.uint8), alpha])
    result = Image.fromarray(rgba)
    return pad_content(result if keep_shadows else remove_window_shadows(result))


def capture_scene(client, capture, name, scene, output, settle=0, keep_shadows=False):
    started = time.perf_counter()
    client.scene(scene)
    if any(method == "ShowWindow" and arguments.get("window", 0) in (0, "PREVIEW_WINDOW_TYPE_MAIN") for method, arguments in scene["calls"]):
        capture.position_main()
    # Keep the pointer outside the target windows so hover effects stay unchanged.
    capture.user.SetCursorPos(100, 100)
    images = []
    windows = None
    rect = None
    # Windows' native tray popup is opaque and paints through GDI. Read its
    # own window DC once, avoiding both desktop readbacks and backdrop occlusion.
    current = capture.windows()
    deadline = time.monotonic() + 3
    while not current and time.monotonic() < deadline:
        capture.next_frame()
        current = capture.windows()
    if len(current) == 1 and current[0].get("window_class") == "#32768":
        window = current[0]
        capture.user.RedrawWindow(window["handle"], None, None, 0x0181)
        rgb = capture.capture(window["rect"], window["handle"])
        result = pad_content(rgb.convert("RGBA"))
        mattes = []
        windows, rect = current, window["rect"]
    else:
        result = None
    for color in ["#00ff00", "#ff00ff"]:
        if result is not None:
            break
        client.call("SetBackdrop", {"color": color})
        # Native Windows tray popups close when the backdrop request is dispatched.
        # Reopen exactly the same menu for each matte.
        for method, arguments in scene["calls"]:
            if method == "ShowMenu":
                client.call(method, arguments)
        # An optional extra delay is available for slow third-party themes.
        if settle:
            time.sleep(settle)
        capture.arrange()
        capture.next_frame()
        current = capture.windows()
        if not current:
            raise RuntimeError("No Workrave preview windows visible for " + name)
        if windows is None:
            windows = current
            rect = [min(w["rect"][0] for w in windows) - 16,
                    min(w["rect"][1] for w in windows) - 16,
                    max(w["rect"][2] for w in windows) + 16,
                    max(w["rect"][3] for w in windows) + 16]
            rect[0], rect[1] = max(0, rect[0]), max(0, rect[1])
            rect[2] = min(capture.user.GetSystemMetrics(0), rect[2])
            rect[3] = min(capture.user.GetSystemMetrics(1), rect[3])
            # Qt break cards use transparent, screen-sized native windows.
            # Their capture must exclude the OS-owned taskbar.
            screen = [0, 0, capture.user.GetSystemMetrics(0), capture.user.GetSystemMetrics(1)]
            if any(w["rect"] == screen for w in windows):
                work_area = capture.w.RECT()
                if capture.user.SystemParametersInfoW(0x0030, 0, ctypes.byref(work_area), 0):
                    rect[0], rect[1] = max(rect[0], work_area.left), max(rect[1], work_area.top)
                    rect[2], rect[3] = min(rect[2], work_area.right), min(rect[3], work_area.bottom)
        elif sorted((w["title"], w["rect"]) for w in current) != sorted((w["title"], w["rect"]) for w in windows):
            raise RuntimeError("Window geometry changed between mattes for " + name)
        images.append(capture.capture(rect))
    if result is None:
        result = recover_alpha(*images, keep_shadows=keep_shadows)
        mattes = ["#00ff00", "#ff00ff"]
    result.save(output / (name + ".png"), compress_level=1)
    elapsed = time.perf_counter() - started
    (output / (name + ".json")).write_text(json.dumps(dict(scene=scene, windows=windows, bounds=rect,
                                                           size=result.size, mattes=mattes,
                                                           shadows=keep_shadows, seconds=elapsed), indent=2))
    print("Captured", name, result.size, f"{elapsed:.3f}s", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--endpoint", default="127.0.0.1:50055")
    parser.add_argument("--proto-dir", type=Path, default=ROOT / "ui/app/devel/gen")
    parser.add_argument("--scenes", type=Path, default=Path(__file__).with_name("window-preview-scenes.json"))
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("list")
    commands.add_parser("quit", help="Close the development driver cleanly")
    show = commands.add_parser("show"); show.add_argument("scene")
    call = commands.add_parser("call"); call.add_argument("method"); call.add_argument("json", nargs="?", default="{}")
    screenshot = commands.add_parser("capture"); screenshot.add_argument("scene", help="Preset name or all")
    screenshot.add_argument("--pid", type=int, required=True)
    screenshot.add_argument("--output", type=Path, required=True)
    screenshot.add_argument("--settle", type=float, default=0, help="Optional extra delay per matte, in seconds")
    screenshot.add_argument("--keep-shadows", action="store_true", help="Retain window drop shadows")
    args = parser.parse_args()
    scenes = json.loads(args.scenes.read_text())
    if args.command == "list":
        for name, scene in scenes.items(): print(name + ": " + scene["description"])
        return
    client = PreviewClient(args.endpoint, args.proto_dir)
    if args.command == "quit":
        client.call("Quit")
    elif args.command == "call":
        print(client.call(args.method, json.loads(args.json)))
    elif args.command == "show":
        client.scene(scenes[args.scene]); print("Showing", args.scene)
    else:
        args.output.mkdir(parents=True, exist_ok=True)
        capture = WindowsCapture(args.pid)
        selected = scenes if args.scene == "all" else {args.scene: scenes[args.scene]}
        try:
            for name, scene in selected.items():
                try:
                    capture_scene(client, capture, name, scene, args.output, args.settle, args.keep_shadows)
                except RuntimeError as error:
                    if args.scene != "all" or not scene.get("optional"):
                        raise
                    print("Unavailable optional scene", name, str(error), flush=True)
        finally:
            client.call("Hide")
            client.call("SetBackdrop", {"color": ""})

if __name__ == "__main__":
    main()
