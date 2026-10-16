# notepad.exe (Windows 95, 4.00.950) — work in progress

Target: `original/SYSTEM/notepad.exe` (34304 bytes, MD5 8198fd52174a2e347c20d5bd79e2af38).

## Status (2026-10-16)
`tools/build.sh notepad` builds a complete, working NOTEPAD.EXE with the same size (34304 bytes) and
section table as the original.

| Part | State |
|---|---|
| .text | 48/57 functions byte-identical (65% of code bytes) |
| .data | identical |
| .bss | identical layout (npbss.asm) |
| .rsrc | identical (timestamps set post-link) |
| .idata | descriptors, hints and bound values identical; entry order follows code and will settle as functions match |
| headers | identical apart from fields that depend on .text/.reloc sizes and the checksum |

| File | Matching | Not yet matching |
|---|---|---|
| npmain.c | 18/19 | InitPageSetupDefaults |
| npinit.c | 7/10 | NPInit, NPRegister, LoadFile |
| npsearch.c | 2/2 | |
| npprint.c | 6/10 | NpPrint, ExpandTabs, PageSetupHookProc (5 bytes), TranslateString |
| strings.cpp | 15/16 | StrRChrIA (register swap) |

InitPageSetupDefaults, NPInit, NPRegister and LoadFile differ only in instruction scheduling or
tail merging, and no source shape, compiler flag, padding or declaration order tried so far has
changed them. They may need a different build of the compiler (see "Open questions").

## Build settings
- CL 9.00 (Visual C++ 2.0) `/Ogsy /Gz /W3`. Not `/O1`, which also pools identical strings.
  CL 9.10 (VC 2.1/2.2) generates the same code.
- Full `<windows.h>` plus `<commdlg.h> <cderr.h> <shellapi.h> <tchar.h>` from the Win32 SDK
  (July 1995), with no WIN32_LEAN_AND_MEAN. The SDK's `commdlg.h` lacks `dllimport`, which is why
  COMDLG32 calls go through `jmp [IAT]` stubs.
- strings.cpp is compiled as C++ (`extern "C"` functions).
- LINK 2.50: `/NODEFAULTLIB /ENTRY:ModuleEntry@0 /SUBSYSTEM:WINDOWS,4.0 /MERGE:.rdata=.text
  /SECTION:.idata,R`, objects in .text order, npbss.obj and the resources last.
- Import libraries: SDK libraries copied with their hints patched to the original's values
  (`tools/patchhints.py`).
- Post-link: link/resource timestamps (`tools/pestamp.py`), `BIND -o` against the 950 retail DLLs
  (native IMAGEHLP under Wine), PE checksum (`tools/pechecksum.py`).
- Resource statements in the original .rc order (icons, menu, accelerators, dialogs, version,
  string blocks 4, 3, 1, 2).
- .bss: LINK 2.50 allocates commons in reverse object order and then in the compiler's (unknown)
  symbol-hash order, so npbss.asm defines the layout.

## How CL 9.00 codegen depends on the rest of the file
Most late mismatches were not source logic but tie-breaks in register allocation, operand order
and stack-slot assignment. Three findings explain them:

1. **Symbol count.** Ties are broken by the compiler's internal symbol numbering, which depends on
   how many symbols were declared before a function (the effect repeats every 32). Names, path and
   symbol kind don't matter, only the count. The original was built with internal Win95 headers we
   don't have, so each file defines `NPPAD` (0–31) before including `nppad.h`, which declares that
   many unused externs. Current offsets: npmain 0, npinit 20, npsearch 29, npprint 21, strings 16.
2. **Locals count.** Unused locals shift the same numbering for one function and everything after
   it. `NPLOCn` (from `nppad.h`) at the top of a function body declares n unused locals, standing
   in for locals the original declared but never used (e.g. debug-only variables). Used in
   SetTitle, StrCmpNA and StrCmpNIA2.
3. **Import declaration order.** Which API pointers get cached in registers, and in which, depends
   on the order the imports were first declared. `notepad.h` pre-declares IsDialogMessage,
   TranslateAccelerator, GetMessage, TranslateMessage and DispatchMessage in the order the original
   headers evidently used. This fixed both WinMain and AbortProc.

A function's code depends only on what precedes it, so tune functions in file order and re-check
the whole file after every edit.

Source-level lessons that fixed mismatches:
- Declaration order of globals and locals (it sets operand and register order).
- `MAKEPOINTS` (takes `&lParam`); signed `int` switch variables; WORD/SHORT globals and parameters
  where the original compares 16 bits.
- Two `++` statements instead of `+= 2`.
- Case order from the jump table.
- Separate calls instead of `?:` arguments, but a `?:` where the original computes a value inline
  (GetPrinterDC).
- A case label inside an `if` block (M_EXIT).
- A dead `if (0)` branch to keep the "intl" literal.
- Explicit end pointers or extra locals where the original hoists a loop bound (GetFileName,
  StrRChrA).
- A redundant outer `if` around a loop (NPInitStrings).
- `for(;;)` with `break` where both exits return the same variable (StrRStrA).
- Intrinsic `strcpy` in TranslateString.

## Open questions
- InitPageSetupDefaults, NPInit, NPRegister and LoadFile: possibly a different C2 build from the one
  in VC++ 2.0. A compiler from the Win95 DDK or an NT 3.51-era SDK would be worth trying.
- PageSetupHookProc, StrRChrIA, ExpandTabs and TranslateString: remaining register or slot swaps
  that the padding knobs haven't fixed yet.

## Tools
- `tools/cc32.sh <src> <outdir> [flags]`: compiles with CL 9.00 and SDK headers (writes .obj/.cod).
- `tools/coffcmp.py <obj> original/SYSTEM/notepad.exe src/notepad/symbols.txt`: per-function compare.
- `tools/fdiff.py <obj> original/SYSTEM/notepad.exe src/notepad/symbols.txt <func> [-a]`: side-by-side
  instruction diff.
- `tools/pedis.py original/SYSTEM/notepad.exe src/notepad/symbols.txt src/notepad/data.txt <func>`:
  original disassembly annotated with our names.
- `tools/variants.py <src> <func> <variants.py>`: compiles function variants in parallel. Supports
  `PADS=a-b`, `LOCS=a-b`, `FDIFF=1` and `BEST=n`.
- `tools/padscan.py <src>`: per-function results at every NPPAD offset.
- `tools/locopt.py <src> [max] [depth] [--write]`: greedy NPLOCn tuning in file order.
- `tools/ghidra/decompile.sh`: headless Ghidra first drafts.
- `tools/res2rc.py`, `tools/peres.py`, `tools/peinfo.py`, `tools/peicons.py`: resources and PE info.
