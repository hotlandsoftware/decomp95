# NOTEPAD.EXE (32-bit) -- sourced by tools/build.sh (helpers in tools/buildlib.sh)
CFLAGS="/Ogsy /Gz /W3"
for f in npmain.c npinit.c npsearch.c npprint.c strings.cpp; do
    cc32 "$SRC/$f" $CFLAGS "/I$(winpath "$SRC")"
done
uasm -q -coff -Cp -Fonpbss.obj "$SRC/npbss.asm"
python3 "$ROOT/tools/peicons.py" "$ROOT/original/SYSTEM/notepad.exe" . >/dev/null
rc32 "$SRC/notepad.rc"

# Import libraries: the Win32 SDK libraries with the hint values of the
# (earlier Windows 95 build) libraries the original was linked against.
LIBS="SHELL32 KERNEL32 USER32 GDI32 COMDLG32"
for l in $LIBS; do
    python3 "$ROOT/tools/patchhints.py" "$ROOT/win32sdk/LIB/I386/$l.LIB" "$l.LIB" \
        "$ROOT/original/SYSTEM/notepad.exe" >/dev/null
done

link32 notepad.exe /NODEFAULTLIB /ENTRY:ModuleEntry@0 /SUBSYSTEM:WINDOWS,4.0 \
    /MERGE:.rdata=.text /SECTION:.idata,R \
    npmain.obj npinit.obj npsearch.obj npprint.obj strings.obj npbss.obj notepad.rbj \
    SHELL32.LIB KERNEL32.LIB USER32.LIB GDI32.LIB COMDLG32.LIB

# Post-link, as in the original build: build timestamps (link and CVTRES
# run times), bind imports against the Windows 95 (950) DLLs, checksum.
python3 "$ROOT/tools/pestamp.py" notepad.exe 2ff3548d 2ff3548c
mkdir -p binddll
for l in shell32 kernel32 user32 gdi32 comdlg32; do
    cp "$ROOT/original/SYSTEM/$l.dll" "binddll/$l.dll"
done
# BIND needs the real IMAGEHLP.DLL (Wine's built-in BindImageEx is a stub);
# IMAGEHLP is a "known DLL" in Wine, so drop that entry in our private prefix.
wine reg delete 'HKLM\System\CurrentControlSet\Control\Session Manager\KnownDLLs' \
    /v imagehlp /f </dev/null >/dev/null 2>&1 || true
cp "$ROOT/rc4/BIND.EXE" "$ROOT/rc4/IMAGEHLP.DLL" .
WINEDLLOVERRIDES="$WINEDLLOVERRIDES;imagehlp=n" wine ./BIND.EXE -o -u -p binddll notepad.exe \
    </dev/null 2>&1 | tr -d '\r' | grep -v '^$' || true
python3 "$ROOT/tools/pechecksum.py" notepad.exe

compare32 notepad.exe SYSTEM/notepad.exe
