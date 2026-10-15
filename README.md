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
`tools/build.sh` builds `build/tapiexe/TAPIEXE.EXE` from a clean state and compares it with `original/tapiexe.exe`.

# Layout
- `original/` – original binaries (not committed)
- `src/<module>/` – decompiled sources
- `tools/build.sh` – builds into `build/` and compares against `original/`
- `tools/cc.sh` – compile one C file with MSVC 1.52 (writes a `.COD` listing)
- `tools/dosrun.sh` – run DOS commands in headless DOSBox; `tools/wineenv.sh` – headless Wine environment
- `tools/codcmp.py` – compare a `.COD` listing instruction-by-instruction with the original
- `tools/necmp.py` – NE executable comparison; `tools/omfdump.py` – OMF object dump; `tools/symdump.py` – MAPSYM .SYM dump

# Status
| Module | Description | Status |
|---|---|---|
| tapiexe.exe | Windows Telephony DLL hidden app | **Byte-identical** (1784 bytes, MD5 c2b65024347742655abe184ce8ac9094) |
