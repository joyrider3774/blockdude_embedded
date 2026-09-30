#!/usr/bin/env python3
"""Convert the level files in assets/levels to source/blockdude_embedded/levels.h.

Every sub folder of assets/levels is a level pack, every .lev file in it a level.
A .lev file is a list of (type, x, y) byte triplets, it is stored as is followed by
the 0xFF 0xFF 0xFF end marker CWorldParts_Load reads up to.

Packs and levels are sorted naturally, so level2 comes after level1 and not after
level19, as level_data_files[pack][SelectedLevel - 1] is how the game finds a level.

Usage:
  python convert_levels.py            write levels.h
  python convert_levels.py --verify   build levels.h in memory and compare it with the
                                      current one, nothing is written
  --output FILE                       write (or verify) this file instead
"""
import argparse
import difflib
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
LEVELS_DIR = os.path.join(ROOT, "assets", "levels")
OUTPUT = os.path.join(ROOT, "source", "blockdude_embedded", "levels.h")
DEFINES = os.path.join(ROOT, "source", "blockdude_embedded", "defines.h")
#the region of defines.h this tool owns, see build_switches()
BEGIN = "//>>> written by tools/convert_levels.py from assets/levels, do not edit by hand"
END = "//<<<"
BYTES_PER_LINE = 32
EOF_MARKER = b"\xFF\xFF\xFF"


def natural_key(name):
    """level2 < level10: digit runs compare as numbers."""
    return [int(part) if part.isdigit() else part.lower() for part in re.split(r"(\d+)", name)]


def c_name(pack, file_name):
    stem = os.path.splitext(file_name)[0]
    return "level_data_%s_%s_data" % (re.sub(r"\W", "_", pack), re.sub(r"\W", "_", stem))


def read_packs(levels_dir):
    """[(pack, [(file name, bytes), ...]), ...] in natural order."""
    packs = []
    for pack in sorted(os.listdir(levels_dir), key=natural_key):
        folder = os.path.join(levels_dir, pack)
        if not os.path.isdir(folder):
            continue
        levels = []
        for file_name in sorted(os.listdir(folder), key=natural_key):
            if not file_name.lower().endswith(".lev"):
                continue
            with open(os.path.join(folder, file_name), "rb") as f:
                data = f.read()
            if len(data) % 3:
                raise SystemExit("%s/%s: %d bytes is not a list of (type, x, y) triplets" % (pack, file_name, len(data)))
            if 0xFF in data[0::3]:
                raise SystemExit("%s/%s: a part of type 0xFF would be read as the end marker" % (pack, file_name))
            levels.append((file_name, data))
        packs.append((pack, levels))
    return packs


MAX_RUN = 128


def rle(data):
    """The control byte scheme of png2rle565.py, over the bytes of one plane.

        c & 0x80 : a run,     (c & 0x7F) + 1 copies of the byte that follows
        else     : a literal, c + 1 bytes follow

    A run of two is left as a literal: it would cost the same and reading it is more work."""
    out = bytearray()
    literal = []

    def flush():
        while literal:
            chunk = literal[:MAX_RUN]
            del literal[:MAX_RUN]
            out.append(len(chunk) - 1)
            out.extend(chunk)

    i = 0
    while i < len(data):
        run = 1
        while i + run < len(data) and run < MAX_RUN and data[i + run] == data[i]:
            run += 1
        if run >= 3:
            flush()
            out.append(0x80 | (run - 1))
            out.append(data[i])
            i += run
        else:
            literal.append(data[i])
            i += 1
    flush()
    return bytes(out)


def unrle(data, count):
    """The first count bytes the reader in cworldparts.cpp makes of a plane."""
    out = bytearray()
    i = 0
    while i < len(data) and len(out) < count:
        control = data[i]
        i += 1
        if control & 0x80:
            out.extend(bytes([data[i]]) * ((control & 0x7F) + 1))
            i += 1
        else:
            n = control + 1
            out.extend(data[i:i + n])
            i += n
    return bytes(out[:count])


def encode_level(data):
    """One level's triplets as a header and three run length encoded planes.

        bytes 0-1   how many parts the level has, little endian
        bytes 2-3   where the x plane starts, counted from the start of the array
        bytes 4-5   where the y plane starts
        byte 6 on   the type plane, then the x plane, then the y plane

    The count is what ends the level, so there is no end marker to look for."""
    parts = len(data) // 3
    planes = [rle(bytes(data[i::3])) for i in range(3)]
    x_at = HEADER + len(planes[0])
    y_at = x_at + len(planes[1])
    head = bytes([parts & 0xFF, parts >> 8, x_at & 0xFF, x_at >> 8, y_at & 0xFF, y_at >> 8])
    out = head + b"".join(planes)
    #never write a level that does not come back out of the reader as it went in
    back = bytearray()
    for t, x, y in zip(unrle(planes[0], parts), unrle(planes[1], parts), unrle(planes[2], parts)):
        back += bytes([t, x, y])
    assert bytes(back) == bytes(data), "a level does not decode back to itself"
    return out


def decode_level(stored):
    """What the game reads back out of one of those arrays, as the triplets it started as."""
    parts = stored[0] | (stored[1] << 8)
    x_at = stored[2] | (stored[3] << 8)
    y_at = stored[4] | (stored[5] << 8)
    types = unrle(stored[HEADER:x_at], parts)
    xs = unrle(stored[x_at:y_at], parts)
    ys = unrle(stored[y_at:], parts)
    out = bytearray()
    for t, x, y in zip(types, xs, ys):
        out += bytes([t, x, y])
    return bytes(out)


HEADER = 6


def build_header(packs, levels_dir):
    max_items = max(len(levels) for _, levels in packs)
    lines = [
        "// Auto-generated by tools/convert_levels.py",
        "// Source directory: %s" % os.path.relpath(levels_dir, ROOT).replace(os.sep, "/"),
        "// Access as: level_data_files[group_index][item_index]  (const uint8_t*)",
        "// A row holds only the levels that are built in, so the slots after them stay null.",
        "// A level is kept as three run length encoded planes, the types then the x's then the",
        "// y's, behind a header of the part count and where the second and third plane start.",
        "// See encode_level in the tool and the reader in cworldparts.cpp",
        "",
        "#pragma once",
        "#include <stdint.h>",
        "#include <stddef.h>",
        "//LEVELPACKS, MAXLEVELSPERPACK and what they work out to, which pick what is below",
        '#include "defines.h"',
        "",
        "// Index map:",
    ]
    for g, (pack, levels) in enumerate(packs):
        lines.append("//   [%d] %s/" % (g, pack))
        for i, (file_name, _) in enumerate(levels):
            lines.append("//       [%d] %s" % (i, file_name))
    lines.append("")

    for pack, levels in packs:
        for file_name, data in levels:
            body = encode_level(data)
            lines.append("// %s/%s (%d parts, %d bytes packed into %d)"
                         % (pack, file_name, len(data) // 3, len(data), len(body)))
            lines.append("const uint8_t %s[] PLATFORM_PROGMEM = {" % c_name(pack, file_name))
            rows = ["    " + ", ".join("0x%02X" % b for b in body[off:off + BYTES_PER_LINE])
                    for off in range(0, len(body), BYTES_PER_LINE)]
            lines.append(",\n".join(rows))
            lines.append("};")
            lines.append("")

    lines.append("// 2D lookup table: level_data_files[group_index][item_index]")
    lines.append("// Only the packs and levels the switches in defines.h ask for are named here:")
    lines.append("// an array that is named nowhere is one the compiler leaves out.")
    lines.append("const uint8_t* const level_data_files[LEVELPACKCOUNT][LEVELSPERPACK] = {")
    for pack, levels in packs:
        lines.append("#if LEVELPACKS & LP_%s" % pack)
        lines.append("    {")
        for i, (file_name, _) in enumerate(levels):
            lines.append("#if LEVELBUILT_%s(%d)" % (pack, i))
            lines.append("        %s," % c_name(pack, file_name))
            lines.append("#endif")
        lines.append("    },")
        lines.append("#endif")
    lines.append("};")
    lines.append("")
    lines.append("// how many levels each pack that is built in holds")
    lines.append("const uint8_t level_data_counts[LEVELPACKCOUNT] = {")
    for pack, levels in packs:
        lines.append("#if LEVELPACKS & LP_%s" % pack)
        lines.append("    LEVELSKEPT_%s," % pack)
        lines.append("#endif")
    lines.append("};")
    lines.append("")
    return "\n".join(lines) + "\n"


def level_geometry(data):
    """A level's entries and the size of its bounding box, in tiles.

    CWorldParts_Load makes one part per entry and then fills the rows between the bottom of the
    level and the bottom of the screen with earth, so how many parts a level really makes depends
    on how many rows the device shows. The header states the sizes and lets the device work the
    rest out, see LP_FILL."""
    entries = len(data) // 3
    xs = [data[i + 1] for i in range(0, len(data) - 2, 3)]
    ys = [data[i + 2] for i in range(0, len(data) - 2, 3)]
    if not xs:
        return 0, 0, 0
    return entries, max(xs) - min(xs) + 1, max(ys) - min(ys) + 1


def build_switches(packs):
    """The LEVELPACKS and MAXLEVELSPERPACK switches: what of the levels a build takes.

    A pack, or a level, that is left out is named nowhere, so the compiler drops its array and
    the game does not offer it. What packs and levels there are is what lies in assets/levels,
    the same list the arrays are written from, so this is written from there and not by hand."""
    pad = max(len(pack) for pack, _ in packs)
    most = max(len(levels) for _, levels in packs)
    lines = [
        "//LEVELPACKS: the level packs that are built in, an LP_ bit each (the size is what the",
        "//pack takes in flash). All of them unless the device header or the build picks fewer; a",
        "//pack that is left out takes no flash and is not offered in the game",
    ]
    for i, (pack, levels) in enumerate(packs):
        #what it costs in flash, which is the packed size and not the size of the level files
        size = sum(len(encode_level(d)) for _, d in levels)
        lines.append("#define LP_%-*s (1ul << %2d)    //%2d levels, %6d bytes"
                     % (pad, pack, i, len(levels), size))
    lines += [
        "#define LP_ALL ((1ul << %d) - 1)" % len(packs),
        "#ifndef LEVELPACKS",
        "#define LEVELPACKS LP_ALL",
        "#endif",
        "#if (LEVELPACKS & LP_ALL) == 0",
        '#error "LEVELPACKS has to leave at least one level pack in"',
        "#endif",
        "//how many that comes to, which is how many the game offers",
        "#define LEVELPACKCOUNT ("
        + " + ".join("((LEVELPACKS & LP_%s) != 0)" % pack for pack, _ in packs) + ")",
        "",
        "//FIRSTLEVELPERPACK and MAXLEVELSPERPACK: the run of levels a pack keeps, for a device",
        "//that has not the flash for a whole one. The levels of a pack are in the order they are",
        "//played, so a build takes MAXLEVELSPERPACK of them starting at FIRSTLEVELPERPACK and the",
        "//rest are left out; 0 levels means all of them from the first one on. Several builds with",
        "//runs that follow one another cover a whole pack between them",
        "#ifndef FIRSTLEVELPERPACK",
        "#define FIRSTLEVELPERPACK 0",
        "#endif",
        "#ifndef MAXLEVELSPERPACK",
        "#define MAXLEVELSPERPACK 0",
        "#endif",
        "//A pack may be given a run of its own, and takes the one above when it is not. That is what",
        "//lets one build hold the end of a long pack and the whole of a short one",
    ]
    for pack, _ in packs:
        lines += [
            "#ifndef FIRSTLEVEL_%s" % pack,
            "#define FIRSTLEVEL_%s FIRSTLEVELPERPACK" % pack,
            "#endif",
            "#ifndef MAXLEVELS_%s" % pack,
            "#define MAXLEVELS_%s MAXLEVELSPERPACK" % pack,
            "#endif",
        ]
    lines += [
        "//1 while level n of a pack is in the build, counted from 0",
        "#define LEVELBUILT_AT(n, first, most) (((n) >= (first)) && \\",
        "                                      (((most) == 0) || ((n) < (first) + (most))))",
        "//of a pack of n levels, how many are kept",
        "#define LEVELSKEPT_AT(n, first, most) (((n) <= (first)) ? 0 : \\",
        "                                      ((((most) == 0) || ((n) - (first) <= (most))) \\",
        "                                       ? (n) - (first) : (most)))",
        "//the longest of the runs, which is what the lookup table in levels.h is sized by. It is",
        "//written after the packs below, since it is the packs it is the largest of",
        "//A pack the run leaves nothing of would be offered with no levels in it, so this says",
        "//so at build time instead: a build starting past the end of a pack leaves it out too",
    ]
    for pack, levels in packs:
        lines += [
            "#define LEVELBUILT_%s(n) LEVELBUILT_AT(n, FIRSTLEVEL_%s, MAXLEVELS_%s)" % (pack, pack, pack),
            "#define LEVELSKEPT_%s LEVELSKEPT_AT(%d, FIRSTLEVEL_%s, MAXLEVELS_%s)"
            % (pack, len(levels), pack, pack),
            "#if (LEVELPACKS & LP_%s) && (LEVELSKEPT_%s == 0)" % (pack, pack),
            '#error "the run leaves nothing of %s, take that pack out of LEVELPACKS"' % pack,
            "#endif",
        ]
    longest = "LEVELSKEPT_%s" % packs[0][0]
    for pack, _ in packs[1:]:
        longest = "((%s) > LEVELSKEPT_%s ? (%s) : LEVELSKEPT_%s)" % (longest, pack, longest, pack)
    lines += [
        "//the longest run of all, which is how wide the lookup table in levels.h has to be",
        "#define LEVELSPERPACK %s" % longest,
        "",
        "//The busiest level each pack has, counted the same way encode_level does. The pool of world",
        "//parts is the largest single thing the game asks the heap for and no level fills the grid, so",
        "//a build wants no more slots than the packs it holds can fill, see MAXWORLDPARTS",
    ]
    lines += [
        "//A level is padded with earth from its bottom row down to the bottom of the screen, see",
        "//CWorldParts_Load, so the parts it makes depend on how many rows the device shows. This is",
        "//that padding for a level whose bounding box is w by h tiles",
        "#define LP_ROWSSHOWN ((NrOfRowsVisible < NrOfRows) ? NrOfRowsVisible : NrOfRows)",
        "#define LP_FILL(w, h) ((PADLEVELROWS && (LP_ROWSSHOWN > (h))) \\",
        "                       ? ((LP_ROWSSHOWN - (h)) * (((w) < NrOfCols) ? (w) : NrOfCols)) : 0)",
        "//the entries of each level plus that padding, the largest of them per pack",
    ]
    for pack, levels in packs:
        most_of_pack = "0"
        for _, data in levels:
            entries, w, h = level_geometry(data)
            most_of_pack = ("LP_PARTS_MAX(%s, %d + LP_FILL(%d, %d))"
                            % (most_of_pack, entries, w, h))
        lines.append("#define LP_PARTS_%-*s %s" % (pad, pack, most_of_pack))
    most = "0"
    for pack, _ in packs:
        most = "LP_PARTS_MAX(%s, LP_PARTS_OF(%s))" % (most, pack)
    lines += [
        "",
        "//How many parts the busiest level of the packs this build holds has. A pack that is left out",
        "//counts for nothing. LP_PARTS_MAX is a function and not a macro on purpose: a macro naming",
        "//its first argument twice doubles the text at every step, which with many packs puts the",
        "//compiler out of memory. constexpr keeps it usable where a constant is wanted",
        "static inline constexpr int LP_PARTS_MAX(int a, int b) { return (a > b) ? a : b; }",
        "#define LP_PARTS_OF(p) (((LEVELPACKS & LP_##p) != 0) ? LP_PARTS_##p : 0)",
        "#define LEVELPACKMAXPARTS %s" % most,
    ]
    return "\n".join(lines)


def splice(text, block):
    """Puts block between the markers, which is the part of defines.h this tool writes."""
    start = text.find(BEGIN)
    end = text.find(END, start + 1) if start >= 0 else -1
    if start < 0 or end < 0:
        raise SystemExit("the markers are gone from %s, put them back" % DEFINES)
    return text[:start] + BEGIN + "\n" + block + "\n" + text[end:]


def parse_header(text):
    """What the game gets out of a levels.h: the arrays (text and bytes) and the lookup table."""
    arrays = {}
    for m in re.finditer(r"(// [^\n]+\n)const uint8_t (\w+)\[\] PLATFORM_PROGMEM = \{\n(.*?)\n\};", text, re.S):
        stored = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(3)))
        arrays[m.group(2)] = {
            "text": m.group(0),
            #what the game reads is the triplets, so that is what is compared
            "bytes": decode_level(stored),
        }
    m = re.search(r"level_data_files\[([^\]]+)\]\[([^\]]+)\] = \{\n(.*?)\n\};", text, re.S)
    table = None
    if m:
        #a row is what lies between a pack's guard and the brace that closes it, and holds the
        #name of every level of that pack whatever the switches would leave out
        rows = [re.findall(r"level_data_\w+", block)
                for block in re.findall(r"\{\n(.*?)\n    \},", m.group(3), re.S)]
        table = {"dims": (m.group(1), m.group(2)), "rows": rows}
    return arrays, table


def verify(new_text, path):
    with open(path, "r", newline="") as f:
        old_text = f.read()
    new_arrays, new_table = parse_header(new_text)
    old_arrays, old_table = parse_header(old_text)
    ok = True

    if old_table is None:
        print("  no level_data_files table found in %s" % path)
        return False
    if new_table["dims"] != old_table["dims"]:
        ok = False
        print("  DIFFERS  table size %s, current file has %s" % (new_table["dims"], old_table["dims"]))
    for g, (new_row, old_row) in enumerate(zip(new_table["rows"], old_table["rows"])):
        for i, (new_name, old_name) in enumerate(zip(new_row, old_row)):
            if new_name != old_name:
                ok = False
                print("  DIFFERS  level_data_files[%d][%d] = %s, current file has %s" % (g, i, new_name, old_name))
    print("  lookup table %s by %s: %s" % (new_table["dims"] + ("identical" if ok else "differs",)))

    same = 0
    for name in sorted(set(new_arrays) | set(old_arrays), key=natural_key):
        if name not in old_arrays:
            ok = False
            print("  MISSING  %s is not in the current file" % name)
        elif name not in new_arrays:
            ok = False
            print("  EXTRA    %s is in the current file but made from no .lev file" % name)
        elif new_arrays[name]["bytes"] != old_arrays[name]["bytes"]:
            ok = False
            print("  DIFFERS  %s has other level data" % name)
        elif new_arrays[name]["text"] != old_arrays[name]["text"]:
            ok = False
            print("  DIFFERS  %s has the same bytes but is formatted differently" % name)
        else:
            same += 1
    print("  level arrays: %d of %d identical (data and text)" % (same, len(new_arrays)))

    if ok and new_text != old_text:
        print("  note: the file text differs outside the arrays and the table (generator line, index map")
        print("        comment or the order the arrays are defined in), this has no effect on the game")
    return ok


def main():
    parser = argparse.ArgumentParser(description="Convert assets/levels to levels.h")
    parser.add_argument("--verify", action="store_true", help="compare with the current levels.h, write nothing")
    parser.add_argument("--output", default=OUTPUT, help="levels.h to write or verify")
    args = parser.parse_args()

    packs = read_packs(LEVELS_DIR)
    text = build_header(packs, LEVELS_DIR)
    summary = ", ".join("%s %d" % (pack, len(levels)) for pack, levels in packs)

    switches = build_switches(packs)
    with open(DEFINES, "r", newline="") as f:
        defines_raw = f.read()
    defines_old = defines_raw.replace("\r\n", "\n")
    defines_new = splice(defines_old, switches)

    if args.verify:
        print("verifying %s against assets/levels (%s)" % (os.path.relpath(args.output, ROOT), summary))
        ok = verify(text, args.output)
        if defines_new == defines_old:
            print("  the level switches in %s match" % os.path.basename(DEFINES))
        else:
            ok = False
            print("  the level switches in %s differ:" % os.path.basename(DEFINES))
            for line in difflib.unified_diff(defines_old.split("\n"), defines_new.split("\n"),
                                             "current", "generated", lineterm="", n=1):
                print("      " + line)
        print("everything matches" if ok else "there are differences")
        return 0 if ok else 1

    with open(args.output, "w", newline="\n") as f:
        f.write(text)
    print("wrote %s (%s)" % (os.path.relpath(args.output, ROOT), summary))
    with open(DEFINES, "w", newline="\r\n") as f:
        f.write(defines_new)
    print("wrote the level switches into %s" % os.path.relpath(DEFINES, ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
