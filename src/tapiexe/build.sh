# TAPIEXE.EXE -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
CFLAGS="/AM /G3s /GA /GEfs /Gy /Ox /W3"
asm "$ROOT/src/crt/crt0w.asm"
asm "$SRC/telestr.asm"
cc "$SRC/tapistr.c" $CFLAGS
cc "$SRC/tapiexe.c" $CFLAGS
implib "$SRC/tapi.def"
link16 "/NOD /NOE /NON /PACKC /ALIGN:8 /MAP" "tapistr.obj+tapiexe.obj+crt0w.obj+telestr.obj" \
    tapiexe.exe "mnocrtw.lib libw.lib tapi.lib" "$SRC/tapiexe.def"
rc4 "$SRC/tapiexe.rc" TAPIEXE.EXE -40 -k
compare TAPIEXE.EXE SYSTEM/tapiexe.exe
