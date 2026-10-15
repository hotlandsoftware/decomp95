# WINVER.EXE -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
CFLAGS="/AM /G3s /GA /Oxs /W3"
python3 "$ROOT/tools/extract_ico.py" "$ROOT/original/winver.exe" WINVER WINVER.ICO
asm "$SRC/winstub.asm"
wine "$ROOT/msvc/BIN/LINK.EXE" /NOLOGO winstub.obj, winstub.exe, winstub.map\; </dev/null 2>&1 | tr -d '\r'
asm "$ROOT/src/crt/crt0w.asm"
cc "$SRC/winver.c" $CFLAGS
link16 "/NOD /NOE /NOPACKC /ALIGN:16 /MAP" "winver.obj+crt0w.obj" winver.exe "mnocrtw.lib libw.lib" "$SRC/winver.def"
rc4 "$SRC/winver.rc" WINVER.EXE -40
compare WINVER.EXE winver.exe
