#!/bin/sh
# Build TAPIEXE.EXE and compare with original/tapiexe.exe.
# MSVC 1.52 CL/LINK (32-bit PE tools) under a headless private Wine prefix.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC="$ROOT/src/tapiexe"
OUT="$ROOT/build/tapiexe"
CFLAGS="/AM /G3s /GA /GEfs /Gy /Ox /W3"
mkdir -p "$OUT"; cd "$OUT"
find . -maxdepth 1 -type f -iregex '.*\.\(obj\|cod\|exe\|map\|res\)' -delete

cp "$SRC/tapiexe.def" .
nasm -f obj -o start.obj "$SRC/start.asm"
"$ROOT/tools/cc.sh" "$SRC/tapistr.c" "$OUT" $CFLAGS
"$ROOT/tools/cc.sh" "$SRC/tapiexe.c" "$OUT" $CFLAGS

. "$ROOT/tools/wineenv.sh"
wine "$ROOT/msvc/BIN/LINK.EXE" /NOLOGO /NOD /NOE /NON /PACKC /ALIGN:8 /MAP \
    tapistr.obj+tapiexe.obj+start.obj, tapiexe.exe, tapiexe.map, mnocrtw.lib libw.lib, tapiexe.def\; \
    2>&1 | tr -d '\r'

python3 "$ROOT/tools/necmp.py" "$ROOT/original/tapiexe.exe" tapiexe.exe
