# Shared build helpers, sourced by src/<module>/build.sh via tools/build.sh.
# Expects ROOT, MOD, SRC (src/<module>) and OUT (build/<module>[.d95]); runs in $OUT.
# When DECOMP95 is set, C and resource code is built with DECOMP95 defined.
# Toolchains: MSVC 1.52 CL/LINK/IMPLIB under headless Wine, UASM, RC 4.0 in headless DOSBox.

. "$ROOT/tools/wineenv.sh"

# cc <file.c> [cl flags...]  -- compile with MSVC 1.52 (also writes a .COD listing)
cc() { f=$1; shift; "$ROOT/tools/cc.sh" "$f" "$OUT" "$@" ${DECOMP95:+/DDECOMP95}; }

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
    "$ROOT/tools/dosrun.sh" "build/$(basename "$OUT")" 'set PATH=C:\RC4\BINW16' "set INCLUDE=C:\\MSVC\\INCLUDE;C:\\SRC\\$(echo "$MOD" | tr a-z A-Z)" \
        "RC ${DECOMP95:+-dDECOMP95} $* $(basename "$rcfile" | tr a-z A-Z) $exe" | grep -v -e '^Microsoft' -e '^Copyright' -e '^$' || true
}

# compare <built> <original>
compare() {
    if [ -n "$DECOMP95" ]; then echo "Decomp95 variant built: $OUT/$1 (not compared)"; return 0; fi
    python3 "$ROOT/tools/necmp.py" "$ROOT/original/$2" "$1"
}

# ---- 32-bit (Win32) helpers: Visual C++ 2.0 + Win32 SDK headers/libs ----
VC32="$ROOT/${VCDIR:-msvc20}/MSVC20"

# cc32 <file.c> [cl flags...]  -- compile with CL 9.x (also writes a .cod listing)
cc32() { f=$1; shift; "$ROOT/tools/cc32.sh" "$f" "$OUT" "$@" ${DECOMP95:+/DDECOMP95}; }

# rc32 <file.rc>  -- Win32 RC (rc4/RC.EXE) + CVTRES -> <name>.rbj (read-only .rsrc)
rc32() {
    b=$(basename "$1" .rc)
    cp "$1" "$b.rc"
    wine "$ROOT/rc4/RC.EXE" /r ${DECOMP95:+/dDECOMP95} /fo"$b.res" "$b.rc" </dev/null 2>&1 | tr -d '\r' \
        | grep -v -e '^Microsoft' -e '^Copyright' -e '^$' || true
    wine "$VC32/BIN/CVTRES.EXE" -i386 -r -o "$b.rbj" "$b.res" </dev/null 2>&1 | tr -d '\r' \
        | grep -v -e '^Microsoft' -e '^Copyright' -e '^$' || true
    [ -f "$b.rbj" ] || { echo "rc32 failed: $b.rbj not produced" >&2; return 1; }
}

# link32 <out.exe> <link options...> -- objects/libs are part of the options
link32() {
    exe=$1; shift
    wine "$VC32/BIN/LINK.EXE" /NOLOGO /OUT:"$exe" /MAP:"$(basename "$exe" .exe).map" "$@" \
        </dev/null 2>&1 | tr -d '\r'
    [ -f "$exe" ] || { echo "LINK failed: $exe not produced" >&2; return 1; }
}

# compare32 <built> <original>  -- whole-file compare with per-section detail
compare32() {
    if [ -n "$DECOMP95" ]; then echo "Decomp95 variant built: $OUT/$1 (not compared)"; return 0; fi
    python3 "$ROOT/tools/pecmp.py" "$ROOT/original/$2" "$1"
}
