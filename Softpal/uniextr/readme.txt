uniextr:    extract and re-insert text from TEXT.DAT from games using softpal
            adv system engine. this version has unreliable string pointer
            replacing
uniextr2:   same as uniextr, but with reliable string pointer replacing. might
            not work with games that aren't explicitly supported
unisanity:  check if script.src and text.dat are parsable and reports all
            text lines that can't be found
-------------------------------------------------------------------------------
uniextr

extract and re-insert text from TEXT.DAT from games using softpal adv system engine

can add lines with comments in the resulting .txt file, lines not starting
with < are treated as comments and are fully ignored

don't change the numbers in <> at the start of each line, or things may crash
or not work as intended

string pointer search is not very reliable. if the game crashes after the
first changed line, add a parameter "1" to the uniextr p command. if the game
still crashes, increase the number in the parameter, then repeat until it
works. if no values work (e.g. the game crashes later than right after the
first changed line), i really need to improve the pointer finding routine

games that need extra parameter to avoid crash (uniextr only)
- flyable heart: 1
- kimi no nagori: 1

games that don't need extra parameter (uniextr only)
- akatsuki yureru koi akari trial
- a clockwork leyline 1
- flyable candyheart
- flyable heart trial
- kizuna kirameku koi iroha
- kizuna kirameku koi iroha trial
- kizuna kirameku koi iroha -tsubaki renka-
- koi suru kokoro to mahou no kotoba
- koi x shin ai kanojo
- koi x shin ai kanojo trial
- majo koi nikki trial 1
- natsuiro kokoro log
- re:d cherish trial
-------------------------------------------------------------------------------
uniextr2

like uniextr, this tool also rebuilds the entire text.dat file. so every line
after the first line in the script need to be matched correctly in script.src,
otherwise script insertion will most likely be wrong. if this tool doesn't
work, try arcusmaximus/vntranslationtools

this tool will warn if we try to change a non-script line that isn't matched
with script.src

games that should work with uniextr2
* all strings occurring later than the first script line are matched
* only verified with unisanity, haven't tried script insertion on all games
- akatsuki yureru koi akari trial (0-867 left, all are non-script)
- a clockwork leyline 1 (0-51 left, all are non-script)
- a clockwork leyline 1 (english)
- a clockwork leyline 2 (english)
- anata ni koi suru ren'ai recette
- anata ni koi suru ren'ai recette trial
- flyable candyheart (0-9 left, all are non-script)
- flyable heart (0-37 left, all are non-script)
- flyable heart trial (0-23 left, all are non-script)
- kimi no nagori (0, 2-6 left, all are non-script)
- kimi to boku to no kishi no hibi -rakuen no chevalier- trial
- kizuna kirameku koi iroha (0-920 left, all are non-script)
- kizuna kirameku koi iroha trial (0-852 left, all are non-script)
- kizuna kirameku koi iroha -tsubaki renka- (0-1028 left, all are non-script)
- kizuna kirameku koi iroha -tsubaki renka- trial (0-852 left, all are non-script)
- koi saku miyako ni ai no yakusoku o ~annaffiare~ (0-330 left, all are non-script)
- koi saku miyako ni ai no yakusoku o ~annaffiare~ trial (0-301 left, all are non-script)
- koi suru kokoro to mahou no kotoba (0-1042 left, all are non-script)
- koi suru kokoro to mahou no kotoba trial (0-874 left, all are non-script)
- koi x shin ai kanojo (0-387 left, all are non-script)
- koi x shin ai kanojo trial (0-366 left, all are non-script)
- majo koi nikki trial 1 (0-335 left, all are non-script)
- natsuiro kokoro log (0-416 left, all are non-script)
- natsuiro kokoro log -happy summer-
- natsuiro kokoro log -happy summer- log
- re:d cherish trial (0-942 left, all are non-script)
- shikotama slave ~aruji de shimai na tenshi to akuma~ trial (0-196 left, all are non-script i think)
- shiraha kirameku koi shirabe
- shiraha kirameku koi shirabe trial
- the witch's love diary (english)
- unity marriage trial
-------------------------------------------------------------------------------
common stuff for both tools

don't change text that's not part of the script, or things may crash. string
pointer finding might work even worse for those strings

the game engine doesn't do word wrap at all

use garbro to unpack .pac files
use unipack (in the same repo) to repack .pac files
