#!/bin/sh
# Build TAPIEXE.EXE and compare with original/tapiexe.exe.
# MSVC 1.52 CL/LINK/IMPLIB (32-bit PE tools) under a headless private Wine prefix,
# UASM for the startup code, and the 16-bit RC 4.0 (Win32 SDK BINW16) in headless DOSBox.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC="$ROOT/src/tapiexe"
OUT="$ROOT/build/tapiexe"
CFLAGS="/AM /G3s /GA /GEfs /Gy /Ox /W3"
mkdir -p "$OUT"; cd "$OUT"
find . -maxdepth 1 -type f -iregex '.*\.\(obj\|cod\|exe\|map\|res\|lib\|rc\|def\|log\|pdb\)' -delete

cp "$SRC/tapiexe.def" .
uasm -q -omf -Cp -Fostart.obj "$SRC/start.asm"
"$ROOT/tools/cc.sh" "$SRC/tapistr.c" "$OUT" $CFLAGS
"$ROOT/tools/cc.sh" "$SRC/tapiexe.c" "$OUT" $CFLAGS

. "$ROOT/tools/wineenv.sh"
cp "$SRC/tapi.def" .
wine "$ROOT/msvc/BIN/IMPLIB.EXE" /NOLOGO tapi.lib tapi.def 2>&1 | tr -d '\r'
wine "$ROOT/msvc/BIN/LINK.EXE" /NOLOGO /NOD /NOE /NON /PACKC /ALIGN:8 /MAP \
    tapistr.obj+tapiexe.obj+start.obj, tapiexe.exe, tapiexe.map, mnocrtw.lib libw.lib tapi.lib, tapiexe.def\; \
    2>&1 | tr -d '\r'

# Bind the version resource and mark the module as a 4.0 app (-k: no fast-load area).
cp "$SRC/tapiexe.rc" TAPIEXE.RC
"$ROOT/tools/dosrun.sh" build/tapiexe 'set PATH=C:\RC4\BINW16' 'set INCLUDE=C:\MSVC\INCLUDE' \
    'RC -40 -k TAPIEXE.RC TAPIEXE.EXE' | grep -v -e '^Microsoft' -e '^Copyright' -e '^$' || true

python3 "$ROOT/tools/necmp.py" "$ROOT/original/tapiexe.exe" TAPIEXE.EXE
