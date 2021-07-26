this tool has been tested and works with the following versions:

bgi version 1.633 - compatibility v1.72 (kanojo step trial)
bgi version 1.654.3 - compatibility v1.72 (azasumi)
bgi version 1.658.5 - compatibility v1.72 (pure x connect)
bgi version 1.659.6 - compatibility v1.72 (hajirabu making lovers promo)

this tool doesn't currently work with:

bgi version 1.546.7 - compatibility 1.69 (daitoshokan trial)
- misses strings due to dynamic pointer calculation?
-------------------------------------------------------------------------------
singular cases this tool doesn't work with:

hajirabu promo/data01000.arc/main

same problem that keeps the tool from working with daitoshokan: some kind of
code that probably calculates string pointers dynamically and my tool misses
some strings, messing up the encoding
-------------------------------------------------------------------------------
file structure in bgi/ethornell engine games TODO

sysgrp.arc
- engine graphics (different format from data02xxx)
sysprg.arc
system.arc

data01000.arc
- game startup or something
data015??.arc
- game script
data02???.arc
- game graphics
data03000.arc
- sound effect files
data03400.arc
- bgm
data03800.arc
- system sounds
data04???.arc
- voice files
- files with higher numbers can be title screen or config voices
