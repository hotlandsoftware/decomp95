# Shared build helpers, sourced by src/<module>/build.sh via tools/build.sh.
# Expects ROOT, MOD, SRC (src/<module>) and OUT (build/<module>); runs in $OUT.
# Toolchains: MSVC 1.52 CL/LINK/IMPLIB under headless Wine, UASM, RC 4.0 in headless DOSBox.

. "$ROOT/tools/wineenv.sh"

# cc <file.c> [cl flags...]  -- compile with MSVC 1.52 (also writes a .COD listing)
cc() { f=$1; shift; "$ROOT/tools/cc.sh" "$f" "$OUT" "$@"; }

# asm <file.asm>  -- assemble with UASM into <name>.obj
asm() { uasm -q -omf -Cp -Fo"$(basename "$1" .asm).obj" "$1"; }

# implib <file.def>  -- make <name>.lib import library
implib() { cp "$1" .; wine "$ROOT/msvc/BIN/IMPLIB.EXE" /NOLOGO "$(basename "$1" .def).lib" "$(basename "$1")" </dev/null 2>&1 | tr -d '\r'; }

# link "<options>" "<objs joined by +>" <exe> "<libs>" <file.def>
link16() {
    cp "$5" .
    # Unquoted on purpose: LINK parses its own command line (spaces/commas/+).
    wine "$ROOT/msvc/BIN/LINK.EXE" /NOLOGO $1 $2, $3, $(basename "$3" .exe).map, $4, $(basename "$5")\; \
        </dev/null 2>&1 | tr -d '\r'
    [ -f "$3" ] || { echo "LINK failed: $3 not produced" >&2; return 1; }
}

# rc4 <file.rc> <EXE> [rc switches...]  -- bind resources with the 16-bit RC 4.0 (in DOSBox)
rc4() {
    rcfile=$1; exe=$2; shift 2
    cp "$rcfile" "$(basename "$rcfile" | tr a-z A-Z)"
    "$ROOT/tools/dosrun.sh" "build/$MOD" 'set PATH=C:\RC4\BINW16' "set INCLUDE=C:\\MSVC\\INCLUDE;C:\\SRC\\$(echo "$MOD" | tr a-z A-Z)" \
        "RC $* $(basename "$rcfile" | tr a-z A-Z) $exe" | grep -v -e '^Microsoft' -e '^Copyright' -e '^$' || true
}

# compare <built> <original>
compare() { python3 "$ROOT/tools/necmp.py" "$ROOT/original/$2" "$1"; }
