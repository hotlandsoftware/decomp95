#!/bin/sh
# Usage: tools/build.sh [module...]   (default: every src/<module>/build.sh)
# Builds each module into build/<module>/ from a clean state and compares it
# with original/.  Exit status is non-zero if any module does not match.
ROOT=$(cd "$(dirname "$0")/.." && pwd)
[ $# -gt 0 ] || set -- $(cd "$ROOT/src" && for d in */; do [ -f "$d/build.sh" ] && echo "${d%/}"; done)
status=0
for MOD in "$@"; do
    echo "=== $MOD"
    SRC="$ROOT/src/$MOD"; OUT="$ROOT/build/$MOD"
    rm -rf "$OUT"; mkdir -p "$OUT"
    ( set -e; cd "$OUT"; ROOT=$ROOT MOD=$MOD SRC=$SRC OUT=$OUT; . "$ROOT/tools/buildlib.sh"; . "$SRC/build.sh" ) || status=1
done
exit $status
