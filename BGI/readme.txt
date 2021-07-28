this tool has been tested and works with the following versions:

bgi version 1.546.7 - compatibility 1.69 (daitoshokan trial)
bgi version 1.633 - compatibility v1.72 (kanojo step trial)
bgi version 1.654.3 - compatibility v1.72 (azasumi)
bgi version 1.658.5 - compatibility v1.72 (pure x connect)
bgi version 1.659.6 - compatibility v1.72 (hajirabu making lovers promo)
-------------------------------------------------------------------------------
i had to use a dubious hack in order to make this tool work on more files and
versions. the beginning of the text area sometimes contains strings referred
to by 0x7f (command string) as well as strings that no pointers point to
(pointers might be dynamically calculated, dunno). my hack is to leave the
start of the text area alone, and let the start of the text area to be
processed be the lowest address pointed to by 0x03 (text string). this hasn't
broken anything so far.
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
