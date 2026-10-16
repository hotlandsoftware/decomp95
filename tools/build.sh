#!/bin/sh
# Usage: tools/build.sh [module...]   (default: every src/<module>/build.sh)
# Builds each module into build/<module>/ from a clean state and compares it
# with original/.  Exit status is non-zero if any module does not match.
#
# DECOMP95=1 tools/build.sh ...  builds the enhanced Decomp95 variant instead,
# into build/<module>.d95/ (an 8.3 name for DOSBox; not expected to match).
ROOT=$(cd "$(dirname "$0")/.." && pwd)
[ $# -gt 0 ] || set -- $(cd "$ROOT/src" && for d in */; do [ -f "$d/build.sh" ] && echo "${d%/}"; done)
status=0
for MOD in "$@"; do
    echo "=== $MOD"
    SRC="$ROOT/src/$MOD"; OUT="$ROOT/build/$MOD${DECOMP95:+.d95}"
    rm -rf "$OUT"; mkdir -p "$OUT"
    ( set -e; cd "$OUT"; ROOT=$ROOT MOD=$MOD SRC=$SRC OUT=$OUT; . "$ROOT/tools/buildlib.sh"; . "$SRC/build.sh" ) || status=1
done
exit $status
