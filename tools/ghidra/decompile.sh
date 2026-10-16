#!/bin/sh
# Usage: tools/ghidra/decompile.sh <binary> <out.c>
# Runs Ghidra headless (no display) and writes decompiled C for every function.
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=$(realpath "$1"); OUT=$(realpath -m "$2")
PROJ=$(mktemp -d "${TMPDIR:-/tmp}/ghidraproj.XXXXXX")
env -u DISPLAY -u WAYLAND_DISPLAY JAVA_TOOL_OPTIONS=-Djava.awt.headless=true \
    timeout 900 /opt/ghidra/support/analyzeHeadless "$PROJ" tmp -import "$BIN" \
    -scriptPath "$ROOT/tools/ghidra" -postScript DecompileAll.java "$OUT" -deleteProject \
    </dev/null > "$OUT.log" 2>&1
rc=$?
rm -rf "${PROJ:?}"
grep -E 'ERROR|Exception' "$OUT.log" | head -5
exit $rc
