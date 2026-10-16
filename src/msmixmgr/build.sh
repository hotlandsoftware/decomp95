# MSMIXMGR.DLL -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
asm "$SRC/msmixmgr.asm"
link16 "/NOD /NOE /ALIGN:16 /MAP" "msmixmgr.obj" msmixmgr.dll "" "$SRC/msmixmgr.def"
rc4 "$SRC/msmixmgr.rc" MSMIXMGR.DLL -40 -t
compare MSMIXMGR.DLL SYSTEM/msmixmgr.dll
