# WELCOME.EXE (32-bit) -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
CFLAGS="/Ogsy /Gf /Gz /W3"
for f in welcome.c wlcmdlg.c; do
    cc32 "$SRC/$f" $CFLAGS "/I$(winpath "$SRC")"
done
uasm -q -coff -Cp -Fowlcmstr.obj "$SRC/wlcmstr.asm"
uasm -q -coff -Cp -Fowlcmstr2.obj "$SRC/wlcmstr2.asm"
python3 "$ROOT/tools/peicons.py" "$ROOT/original/SYSTEM/welcome.exe" . >/dev/null
python3 "$ROOT/tools/pebitmaps.py" "$ROOT/original/SYSTEM/welcome.exe" . >/dev/null
rc32 "$SRC/welcome.rc"

# Import libraries: SDK libraries with the original's hint values, and an
# ordinal-only SHELL32 library for the undocumented exports.
for l in USER32 KERNEL32 GDI32 ADVAPI32; do
    python3 "$ROOT/tools/patchhints.py" "$ROOT/win32sdk/LIB/I386/$l.LIB" "$l.LIB" \
        "$ROOT/original/SYSTEM/welcome.exe" >/dev/null
done
python3 "$ROOT/tools/ordimplib.py" "$ROOT/win32sdk/LIB/I386/SHELL32.LIB" SHELL32.LIB \
    PathBuildRoot@8@30 PathFindFileName@4@34 PathAppend@8@36 PathIsUNC@4@39 \
    PathFileExists@4@45 ShellMessageBoxA@183 >/dev/null

link32 welcome.exe /NODEFAULTLIB /ENTRY:ModuleEntry@0 /SUBSYSTEM:WINDOWS,4.0 /MERGE:.bss=.data /SECTION:.idata,R \
    wlcmstr.obj welcome.obj wlcmstr2.obj wlcmdlg.obj welcome.rbj \
    USER32.LIB KERNEL32.LIB GDI32.LIB SHELL32.LIB ADVAPI32.LIB

# Post-link, as in the original build: timestamps, bind, checksum.
python3 "$ROOT/tools/pestamp.py" welcome.exe 2ff35982 2ff35982
mkdir -p binddll
for l in user32 kernel32 gdi32 shell32 advapi32; do
    cp "$ROOT/original/SYSTEM/$l.dll" "binddll/$l.dll"
done
wine reg delete 'HKLM\System\CurrentControlSet\Control\Session Manager\KnownDLLs' \
    /v imagehlp /f </dev/null >/dev/null 2>&1 || true
cp "$ROOT/rc4/BIND.EXE" "$ROOT/rc4/IMAGEHLP.DLL" .
WINEDLLOVERRIDES="$WINEDLLOVERRIDES;imagehlp=n" wine ./BIND.EXE -o -u -p binddll welcome.exe \
    </dev/null 2>&1 | tr -d '\r' | grep -v '^$' || true
python3 "$ROOT/tools/pechecksum.py" welcome.exe

compare32 welcome.exe SYSTEM/welcome.exe
