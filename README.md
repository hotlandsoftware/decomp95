# Decomp95
The goal of this project is to study and create a byte-matched decompilation of the Windows 95 operating system. This is based primarily on Windows 95 Build 950.6 (``en-US_x86chk_950_win95.rtm_chicago.iso``). Some debug symbols are from Build 490 (``4.00.490_x86chk_client_en-us-WIN95_DEBUG.iso``)

# Requirements
Toolchains are not committed; place them as follows:
- `msvc/` – Microsoft Visual C++ 1.52 (`BIN`, `INCLUDE`, `LIB`). CL 8.00c, LINK 5.60.339 and IMPLIB are
  32-bit PE console tools and run under Wine (headless, private prefix in `build/wineprefix`).
- `rc4/BINW16/` – 16-bit Resource Compiler 4.00 from the Win32 SDK (July 1995). It is a DOS program and
  runs in DOSBox (headless).
- Host packages: `wine`, `dosbox`, `uasm`, `python3`

# Building
`tools/build.sh [module...]` builds each module (default: all) into `build/<module>/` from a clean state and
compares it with `original/`. Each module has a `src/<module>/build.sh` using the helpers in `tools/buildlib.sh`.
Artwork (icons etc.) is extracted from `original/` at build time and is never committed.

# Layout
- `original/` – original Windows 95 files, laid out like the Windows directory (not committed)
- `src/crt/` – shared startup code (`crt0w.asm`, medium-model no-CRT Windows startup)
- `src/<module>/` – decompiled sources, `.def`, `.rc` and `build.sh` per module
- `tools/build.sh`, `tools/buildlib.sh` – build driver and helpers
- `tools/cc.sh` – compile one C file with MSVC 1.52 (writes a `.COD` listing)
- `tools/dosrun.sh` – run DOS commands in headless DOSBox; `tools/wineenv.sh` – headless Wine environment
- `tools/codcmp.py` – compare a `.COD` listing instruction-by-instruction with the original
- `tools/necmp.py` – NE executable comparison; `tools/neinfo.py` – NE header/segment/relocation dump
- `tools/neres.py` – NE resource listing/dump; `tools/extract_ico.py` – rebuild an .ico from an NE icon group
- `tools/omfdump.py` – OMF object dump; `tools/symdump.py` – MAPSYM .SYM dump

# Status
| Module | Description | Status |
|---|---|---|
| SYSTEM/tapiexe.exe | Windows Telephony DLL hidden app | **Byte-identical** (1784 bytes) |
| winver.exe | Windows Version program | **Byte-identical** (3632 bytes) |
