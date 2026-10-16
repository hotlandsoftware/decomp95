#!/bin/sh
# Usage: tools/cc32.sh <src.c> <outdir> [cl flags...]  -- compile with Visual C++ 2.0 (CL 9.00)
# under the headless private Wine prefix.  Writes <name>.obj and a <name>.cod listing.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=$(realpath "$1"); OUT=$2; shift 2
mkdir -p "$OUT"; cd "$OUT" || exit 1
. "$ROOT/tools/wineenv.sh"
VC="$ROOT/${VCDIR:-msvc20}/MSVC20"
SDK="$ROOT/win32sdk"
export WINEPATH="$(winpath "$VC/BIN")" INCLUDE="$(winpath "$SDK/INCLUDE");$(winpath "$VC/INCLUDE")" LIB="$(winpath "$SDK/LIB/I386");$(winpath "$VC/LIB")"
B=$(basename "$SRC"); B=${B%.*}
case "$SRC" in *.cpp) TP=/Tp ;; *) TP= ;; esac
wine "$VC/BIN/CL.EXE" /nologo /c "$@" /FAc /Fa$B.cod /Fo$B.obj $TP "$(winpath "$SRC")" </dev/null 2>&1 \
    | tr -d "\r" | grep -v -x -F -e "$B.c" -e "$B.cpp" -e "$(winpath "$SRC")" || true
