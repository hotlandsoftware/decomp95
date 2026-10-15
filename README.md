# Decomp95
The goal of this project is to study and create a byte-matched decompilation of the Windows 95 operating system. This is based primarily on Windows 95 Build 950.6 (``en-US_x86chk_950_win95.rtm_chicago.iso``). Some debug symbols are from Build 490 (``4.00.490_x86chk_client_en-us-WIN95_DEBUG.iso``)

# Requirements
- Microsoft Visual C++ 1.52 (CL 8.00c, LINK 5.60.339) in `msvc/` (`msvc/BIN`, `msvc/INCLUDE`, `msvc/LIB`).
  CL and LINK are 32-bit PE console tools and run under Wine (headless, private prefix in `build/wineprefix`).
- Windows 95 SDK/DDK 16-bit `RC.EXE` 4.0 (supports `-40`) for resources — not yet available
- `wine`, `nasm`, `python3` (`uasm` planned for MASM-compatible fixups in startup code)

# Layout
- `original/` – original binaries (not committed)
- `src/<module>/` – decompiled sources
- `tools/build.sh` – builds into `build/` and compares against `original/`
- `tools/cc.sh` – compile one C file with MSVC 1.52 (writes a `.COD` listing)
- `tools/codcmp.py` – compare a `.COD` listing instruction-by-instruction with the original
- `tools/necmp.py` – NE executable comparison; `tools/omfdump.py` – OMF object dump

# Status
| Module | Description | Status |
|---|---|---|
| tapiexe.exe | Windows Telephony DLL hidden app | Code: all C matches; segment 1 at 599/609 bytes (remaining = NASM fixup encoding). Data segment matches. Version resource pending RC 4.0 |
