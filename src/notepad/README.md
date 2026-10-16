# notepad.exe (Windows 95, 4.00.950) — work in progress

Target: `original/SYSTEM/notepad.exe` (34304 bytes, MD5 8198fd52174a2e347c20d5bd79e2af38).

## Findings
- PE32, LINK 2.50 / C 9.00 (Visual C++ 2.0), compiled with `/O1` (= /Ogsy, functions packed, no padding).
- Headers and import libraries from the Win32 SDK (July 1995): `PAGESETUPDLG` exists there, and its
  `commdlg.h` lacks `dllimport`, which is why COMDLG32 calls go through `jmp [IAT]` stubs.
- `WIN32_LEAN_AND_MEAN` (or equivalent) is needed: VC2's `objbase.h` fails with C4226 otherwise.
- Import hints come from import libraries of an earlier Win95 build (e.g. KERNEL32 LocalAlloc = 399, the
  950 DLL has it at 400). They will need regenerated import libraries with matching export ordering.
- Imports were bound (BIND.EXE) against the 950 retail DLLs in `original/SYSTEM/` (timestamps match),
  and the PE checksum is set.
- No C runtime: own entry point (`ModuleEntry`) at the start of `.text`.
- Globals at 0x4053E0 are a `PAGESETUPDLG` (hDevMode +8, hDevNames +0xC, Flags 0xA006/0xA00A, margins).

## Status (2026-10-15, session 2)
A complete NOTEPAD.EXE now builds (`tools/build.sh notepad`) and runs; same size as the original
(34304 bytes) with an identical section table.

| Part | State |
|---|---|
| .data | identical (1024/1024) |
| .bss | identical layout (npbss.asm + two file statics) |
| .rsrc | identical except CVTRES timestamps |
| .text | 27/57 functions byte-match |
| .idata | same size; order follows code, hints need custom import libs |

Per file: npmain.c 14/19, npinit.c 2/10, npsearch.c 0/2, npprint.c 2/10, strings.cpp 9/16.

## Build settings found
- `/Ogsy /Gz` (not `/O1`: that also pools identical strings, the original does not).
- Full `<windows.h>` + `<commdlg.h> <cderr.h> <shellapi.h> <tchar.h>` (no WIN32_LEAN_AND_MEAN).
- strings.cpp is compiled as C++ (register allocation only matches the C++ front end).
- LINK 2.50: `/MERGE:.rdata=.text /SECTION:.idata,R`, objects in .text order, npbss.obj last.
- Resource statements in the original .rc order (icons, menu, accelerators, dialogs, version,
  string blocks 4,3,1,2).
- "intl" literal: a dead `if (0)` branch in NpWinIniChange keeps the string (original had dead code).
- .bss: LINK 2.50 allocates commons in reverse object order and, within an object, in the
  compiler's symbol-hash order of the (unknown) original names, so npbss.asm fixes the layout.

## Next steps
1. Custom import-library generator (undecorated import names, original hint values); BIND; set
   build timestamps (PE header 0x2FF3548D, resource dirs 0x2FF3548C); checksum.
2. Keep tuning functions in file order (npinit, npsearch, npprint are still first drafts).

## Important: codegen depends on everything before it in the file
CL 9.x breaks register/operand-order ties using state that depends on all symbols seen so far
(headers, earlier functions, declarations).  Padding experiments show it is chaotic, not a simple
count.  Header choice alone moved npmain from 11 to 14 matches.  CL 9.x carries per-file state between functions: changing an earlier function can change register
choice / compare operand order in later ones (NPWndProc, MergeStrings flipped when only NPCommand
changed). Match functions strictly in file order; re-check the whole file after every edit.
Open issues: InitPageSetupDefaults (branch tails merged), NPCommand paste compare (2 bytes),
PFileInPath / WinMain / GetFileName / NPInitStrings register allocation, NPRegister store order.

Source-level lessons that fixed mismatches: declaration order, `MAKEPOINTS` (takes &lParam),
signed `int` switch variables, WORD-typed globals, two `++` instead of `+= 2`, case order (jump table
reveals source order), `switch` with separate calls instead of `?:` arguments, a case label inside an
`if` block (M_EXIT shares Open's PostMessage).

## Tools
- `tools/cc32.sh` – compile with CL 9.00 + SDK headers (writes .obj and .cod)
- `tools/coffcmp.py <obj> original/SYSTEM/notepad.exe src/notepad/symbols.txt -v` – per-function compare
- `tools/ghidra/decompile.sh` – Ghidra headless first-draft decompilation
- `tools/res2rc.py` – PE resources -> RC script (work in progress); `tools/peres.py`, `tools/peinfo.py`
- `tools/fdiff.py <obj> original/SYSTEM/notepad.exe src/notepad/symbols.txt <func>` – side-by-side diff
- `tools/dis.py original/SYSTEM/notepad.exe src/notepad/symbols.txt src/notepad/data.txt <func>` –
  original disassembly annotated with our symbol names

## Symbol-count padding (nppad.h)

CL 9.00 breaks register-allocation and operand-order ties using its internal
symbol numbering, which depends on how many symbols were declared before each
function (it repeats every 32 symbols). The original was built with internal
Win95 headers that we don't have, so each source file defines
`NPPAD` (0-31) and includes `nppad.h`, which declares that many unused
externs. Offsets were found by scanning (`PADS=0-31 tools/variants.py ...`):
npmain 31, npinit 20, npsearch 12, npprint 21, strings 11. This alone fixed
InitPageSetupDefaults, PrintFileCheck and InsertDateTime, and two strings
functions.
