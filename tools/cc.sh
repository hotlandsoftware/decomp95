#!/bin/sh
# Usage: tools/cc.sh <src.c> <outdir> [cl flags...]  -- compile with MSVC 1.52 under Wine.
# Uses a private, display-less Wine prefix so it never touches ~/.wine or the desktop.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=$(realpath "$1"); OUT=$2; shift 2
mkdir -p "$OUT"; cd "$OUT" || exit 1
. "$ROOT/tools/wineenv.sh"
B=$(basename "$SRC" .c)
wine "$ROOT/msvc/BIN/CL.EXE" /nologo /c "$@" /Fc$B.COD /Fo$B.OBJ "$(winpath "$SRC")" </dev/null 2>&1 | tr -d "\r" | grep -v -x -F -e "$B.c" -e "$(winpath "$SRC")" || true
