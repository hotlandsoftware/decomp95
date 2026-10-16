# RSRC16.DLL -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
CFLAGS="/ASw /G3s /GD /Gf /Gy /Ox /W3"
asm "$SRC/dllhdr.asm"
asm "$SRC/rsrc16t.asm"
cc "$SRC/rsrc16.c" $CFLAGS
link16 "/NOD /NOE /NOPACKF /ALIGN:16 /MAP" "dllhdr.obj+rsrc16.obj+rsrc16t.obj" rsrc16.dll "libw.lib" "$SRC/rsrc16.def"
rc4 "$SRC/rsrc16.rc" RSRC16.DLL -40
compare RSRC16.DLL SYSTEM/rsrc16.dll
