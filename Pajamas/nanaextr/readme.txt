extract and re-insert text from textdata.dat from games using pajamas engine
(i only know of nanatsuiro drops + trial version)

can add lines with comments in the resulting .txt file, lines not starting
with < are treated as comments and are fully ignored

don't change the numbers in <> at the start of each line, or things may crash
or not work as intended

use garbro to unpack .dat files
don't need to repack, can copy loose files to game directory
(scenario.dat and textdata.dat go in root directory, graphics into graph/)
-------------------------------------------------------------------------------
some details about bytecode language
- it's different from games in softpal adv system

format for text lines (with optional speaker)
- pointer to name of speaker is reused!
- 1 dword (probably opcode), 1 dword (voice id i guess), 1 dword (strptr to
  name of speaker), 1 dword (strptr to text line), 1 dword (increasing index
  for unknown use), then 1 dword (0 or 0x00020000 or 0x00030000), then 1 dword (1 or 2)
- some examples
07 03 00 80 00 00 00 00 00 00 00 00 1E 01 00 00 0C 00 00 00 00 00 03 00 01 00 00 00
07 03 00 80 24 00 00 00 48 01 00 00 50 01 00 00 0D 00 00 00 00 00 03 00 01 00 00 00
07 03 00 80 00 00 00 00 A4 06 00 00 AA 06 00 00 30 00 00 00 00 00 03 00 01 00 00 00
07 03 00 80 00 00 00 00 00 00 00 00 30 05 00 00 28 00 00 00 00 00 00 00 01 00 00 00
07 03 00 80 03 05 00 00 BE 9C 00 00 7C F4 00 00 63 06 00 00 00 00 02 00 01 00 00 00
07 03 00 80 30 32 00 00 D0 6E 02 00 0E 47 0A 00 FC 4D 00 00 00 00 03 00 02 00 00 00

chapter name
- address at last dword
- dwords 0, 1, 2, 3, 5, 6 are fixed
- dunno what dword 4 is, but it's increasing by 1 each time
- dword 7 is pointer to chapter name
03 01 00 02 00 00 00 30 01 00 00 00 03 01 00 02 F4 01 00 20 00 00 00 30 02 0D 00 01 14 00 00 00
03 01 00 02 00 00 00 30 01 00 00 00 03 01 00 02 F5 01 00 20 00 00 00 30 02 0D 00 01 14 67 01 00

unknown instruction with string pointer
- this also matches chapter name
02 0D 00 01 04 67 01 00

unknown instruction with string pointer
03 02 01 01 2A 35 00 00
