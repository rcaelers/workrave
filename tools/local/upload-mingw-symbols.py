#!/usr/bin/env python3
"""Generate Breakpad symbols for the MSYS2/MinGW DLLs we ship, and upload them.

Workrave's own binaries are built with clang and get real symbols from
dump_syms. The GTK stack does not: MSYS2 ships those DLLs stripped of DWARF and
without a CodeView record, so a minidump reports their debug id as 33 zeros and
the crash server has nothing to look them up by. Their export table is the only
symbol source left, and this turns it into a Breakpad .sym file.

The result is function-level attribution and nothing more. An address resolves
to the nearest preceding export, so a static or inlined function is reported
under whatever exported symbol sits in front of it. Treat a large +offset in a
symbolised frame as "this name is wrong", not as a large function.

A nil debug id is shared by every build of a DLL, so it cannot tell one from
another: the 2.88.2 glib in the MSYS2 repo and the 2.88.2 our users run report
different code ids but the same nil debug id. The symbol file is therefore
keyed on the code id -- the PE timestamp and image size -- which the crash
server looks up first whenever a module reports a nil debug id. Pass
--id-source zero for the old behaviour.

  ./upload-mingw-symbols.py /mingw64/bin -o /tmp/syms
  ./upload-mingw-symbols.py /mingw64/bin --upload \\
      --server https://crashes.workrave.org --product-token <token> --token <bearer>
"""

import argparse
import os
import struct
import sys
import urllib.error
import urllib.request
import uuid

MACHINES = {0x8664: "x86_64", 0x14C: "x86", 0xAA64: "arm64"}

# What a minidump reports for a PE with no CodeView record. The symbol file has
# to declare the same or the server will never match it.
NO_DEBUG_ID = "0" * 33


class NotPE(Exception):
    pass


class PE:
    def __init__(self, data):
        self.d = data
        if len(data) < 0x40 or data[:2] != b"MZ":
            raise NotPE("no MZ header")
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        if pe + 24 > len(data) or data[pe : pe + 4] != b"PE\0\0":
            raise NotPE("no PE header")
        self.machine, nsec, self.timestamp = struct.unpack_from("<HHI", data, pe + 4)
        opt_size = struct.unpack_from("<H", data, pe + 20)[0]
        opt = pe + 24
        magic = struct.unpack_from("<H", data, opt)[0]
        pe32plus = magic == 0x20B
        self.image_size = struct.unpack_from("<I", data, opt + 56)[0]
        dd = opt + (112 if pe32plus else 96)
        self.dirs = [struct.unpack_from("<II", data, dd + 8 * i) for i in range(16)]
        self.sections = []
        for i in range(nsec):
            b = opt + opt_size + 40 * i
            vsz, vaddr, rsz, raddr = struct.unpack_from("<IIII", data, b + 8)
            self.sections.append((vaddr, vsz, raddr, rsz))

    def rva_to_off(self, rva):
        for vaddr, vsz, raddr, rsz in self.sections:
            if vaddr <= rva < vaddr + max(vsz, rsz):
                off = rva - vaddr + raddr
                return off if off < len(self.d) else None
        return None

    def cstr(self, rva):
        off = self.rva_to_off(rva)
        if off is None:
            return None
        end = self.d.find(b"\0", off)
        return self.d[off:end].decode("ascii", "replace")

    @property
    def arch(self):
        return MACHINES.get(self.machine)

    @property
    def code_id(self):
        return "%08X%X" % (self.timestamp, self.image_size)

    def debug_id(self):
        """(debug_id, debug_file) as the minidump will report them."""
        rva, size = self.dirs[6]
        off = self.rva_to_off(rva) if size else None
        if off is not None:
            for i in range(size // 28):
                e = off + 28 * i
                if e + 28 > len(self.d):
                    break
                dtype = struct.unpack_from("<I", self.d, e + 12)[0]
                dsize, _, draw = struct.unpack_from("<III", self.d, e + 16)
                if dtype == 2 and self.d[draw : draw + 4] == b"RSDS":
                    g1, g2, g3 = struct.unpack_from("<IHH", self.d, draw + 4)
                    g4 = self.d[draw + 12 : draw + 20]
                    age = struct.unpack_from("<I", self.d, draw + 20)[0]
                    pdb = self.d[draw + 24 : draw + dsize].split(b"\0")[0]
                    guid = "%08X%04X%04X%s" % (g1, g2, g3, g4.hex().upper())
                    return guid + ("%X" % age), pdb.decode("ascii", "replace")
        return NO_DEBUG_ID, None

    def version(self):
        """FileVersion from the VS_FIXEDFILEINFO resource, if there is one."""
        sig = b"\xbd\x04\xef\xfe"
        i = self.d.find(sig)
        if i < 0 or i + 16 > len(self.d):
            return None
        ms, ls = struct.unpack_from("<II", self.d, i + 8)
        return "%d.%d.%d.%d" % (ms >> 16, ms & 0xFFFF, ls >> 16, ls & 0xFFFF)

    def exports(self):
        rva, size = self.dirs[0]
        if not size:
            return []
        off = self.rva_to_off(rva)
        if off is None:
            return []
        nfunc, nname = struct.unpack_from("<II", self.d, off + 20)
        afunc, aname, aord = struct.unpack_from("<III", self.d, off + 28)
        fo, no, oo = (self.rva_to_off(x) for x in (afunc, aname, aord))
        if None in (fo, no, oo):
            return []
        out = set()
        for i in range(nname):
            name_rva = struct.unpack_from("<I", self.d, no + 4 * i)[0]
            ordinal = struct.unpack_from("<H", self.d, oo + 2 * i)[0]
            if ordinal >= nfunc:
                continue
            func_rva = struct.unpack_from("<I", self.d, fo + 4 * ordinal)[0]
            # An RVA inside the export directory is a forwarder, not code.
            if rva <= func_rva < rva + size or not func_rva:
                continue
            name = self.cstr(name_rva)
            if name:
                out.add((func_rva, name))
        return sorted(out)


def make_sym(pe, name, id_source="code"):
    debug_id, pdb = pe.debug_id()
    # A nil debug id is shared by every build of the DLL. The crash server falls
    # back to the code id for those, so key the file on it instead.
    if debug_id == NO_DEBUG_ID and id_source == "code":
        debug_id = pe.code_id
    debug_file = pdb or name
    exports = pe.exports()
    lines = ["MODULE windows %s %s %s" % (pe.arch, debug_id, debug_file)]
    lines.append("INFO CODE_ID %s %s" % (pe.code_id, name))
    lines += ["PUBLIC %x 0 %s" % (rva, sym) for rva, sym in exports]
    return "\n".join(lines) + "\n", debug_file, debug_id, len(exports)


def post_multipart(url, bearer, fields, filename, payload):
    boundary = uuid.uuid4().hex
    body = bytearray()
    for key, value in fields.items():
        body += ("--%s\r\n" % boundary).encode()
        body += ('Content-Disposition: form-data; name="%s"\r\n\r\n' % key).encode()
        body += value.encode() + b"\r\n"
    body += ("--%s\r\n" % boundary).encode()
    body += (
        'Content-Disposition: form-data; name="upload_file_symbols"; filename="%s"\r\n'
        % filename
    ).encode()
    body += b"Content-Type: application/octet-stream\r\n\r\n"
    body += payload + b"\r\n"
    body += ("--%s--\r\n" % boundary).encode()

    req = urllib.request.Request(url, data=bytes(body), method="POST")
    req.add_header("Content-Type", "multipart/form-data; boundary=%s" % boundary)
    req.add_header("Authorization", "Bearer %s" % bearer)
    with urllib.request.urlopen(req, timeout=120) as r:
        return r.status, r.read().decode("utf-8", "replace")[:200]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("path", help="a DLL/EXE, or a directory to scan")
    ap.add_argument("-o", "--output", help="write .sym files here")
    ap.add_argument("--upload", action="store_true")
    ap.add_argument("--server", help="e.g. https://crashes.workrave.org")
    ap.add_argument("--product-token", help="product token in the upload URL")
    ap.add_argument("--token", help="bearer token; also read from GUARDRAIL_SYMBOL_TOKEN")
    ap.add_argument("--channel", default="stable")
    ap.add_argument("--commit", default="msys2", help="provenance recorded with the upload")
    ap.add_argument(
        "--id-source",
        choices=("code", "zero"),
        default="code",
        help="what to key a DLL with no CodeView record on: its code id (default) "
        "or the nil debug id the minidump reports",
    )
    ap.add_argument("--insecure", action="store_true", help="skip TLS verification")
    ap.add_argument("-n", "--dry-run", action="store_true")
    args = ap.parse_args()

    bearer = args.token or os.environ.get("GUARDRAIL_SYMBOL_TOKEN")
    if args.upload:
        missing = [
            n
            for n, v in (("--server", args.server), ("--product-token", args.product_token), ("--token", bearer))
            if not v
        ]
        if missing:
            sys.exit("--upload needs %s" % ", ".join(missing))

    if args.insecure:
        import ssl

        ctx = ssl._create_unverified_context()
        urllib.request.install_opener(
            urllib.request.build_opener(urllib.request.HTTPSHandler(context=ctx))
        )

    if os.path.isdir(args.path):
        files = sorted(
            os.path.join(args.path, f)
            for f in os.listdir(args.path)
            if f.lower().endswith((".dll", ".exe"))
        )
    else:
        files = [args.path]
    if not files:
        sys.exit("no .dll or .exe found in %s" % args.path)

    if args.output:
        os.makedirs(args.output, exist_ok=True)

    generated = uploaded = skipped = failed = 0
    for path in files:
        name = os.path.basename(path)
        try:
            pe = PE(open(path, "rb").read())
        except (NotPE, struct.error, OSError) as e:
            print("  skip  %-32s %s" % (name, e))
            skipped += 1
            continue
        if pe.arch is None:
            print("  skip  %-32s unsupported machine 0x%x" % (name, pe.machine))
            skipped += 1
            continue

        sym, debug_file, debug_id, n = make_sym(pe, name, args.id_source)
        if n == 0:
            print("  skip  %-32s no exports" % name)
            skipped += 1
            continue
        if debug_id == NO_DEBUG_ID:
            note = "  (nil id: every build collides -- use --id-source code)"
        elif debug_id == pe.code_id:
            note = "  (code id)"
        else:
            note = "  (CodeView; prefer dump_syms)"
        print("  sym   %-32s %5d symbols  id=%-33s%s" % (name, n, debug_id, note))
        generated += 1

        if args.output:
            with open(os.path.join(args.output, debug_file + ".sym"), "w") as f:
                f.write(sym)

        if not args.upload:
            continue
        url = "%s/api/symbols/%s/upload" % (args.server.rstrip("/"), args.product_token)
        fields = {
            "version": pe.version() or "0.0.0.0",
            "channel": args.channel,
            "commit": args.commit,
            "build_id": pe.code_id,
        }
        if args.dry_run:
            print("        would POST %s %s" % (url, fields))
            continue
        try:
            status, body = post_multipart(url, bearer, fields, debug_file + ".sym", sym.encode())
            print("        uploaded http=%s %s" % (status, body.strip()))
            uploaded += 1
        except urllib.error.HTTPError as e:
            print("        FAILED http=%s %s" % (e.code, e.read().decode("utf-8", "replace")[:200]))
            failed += 1
        except OSError as e:
            print("        FAILED %s" % e)
            failed += 1

    print(
        "\n%d generated, %d uploaded, %d skipped, %d failed" % (generated, uploaded, skipped, failed)
    )
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
