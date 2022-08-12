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

working values for some games:
- flyable heart: 1
- kimi no nagori: 1

games that work with no parameter:
- a clockwork leyline 1
- akatsuki yureru koi akari
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

don't change text that's not part of the script, or things may crash
(string pointer finding might work even worse for those strings)

the game engine doesn't do word wrap at all

use garbro to unpack .pac files
use unipack (in the same repo) to repack .pac files
-------------------------------------------------------------------------------
ugly details about the bytecode language where string pointers occur

in the byte sequences below, ## ## ## ## is the string pointer

the extremely lazy way to match:
1f 00 01 00 ## ## ## ##
- this results in a bunch of false hits, but catches nearly everything
- i'd like to improve on this, my current way to handle false hits is dumb

3-way choice (line 151 in leyline 1):
1F 00 01 00 ## ## ## ## 17 00 01 00 00 00 0F 00
00 00 00 00 1F 00 01 00 01 00 00 00 17 00 01 00
01 00 06 00 00 00 00 00
- only difference between choices in leyline 1 is strptr. i guess the
  actual choices (and jump points?) are defined afterwards
- not sure when choice command ends, it could be up to 3 dwords shorter

choices                                                                 v-earliest start                    v-latest start
F9 13 00 00 17 00 01 00 00 00 0F 00 00 00 00 00 1F 00 01 00 01 00 00 00 17 00 01 00 01 00 06 00 00 00 00 00 1F 00 01 00 00 00 00 00 1F 00 01 00 FF 00 00 00 1F 00 01 00 CD 00 00 00
1F 00 01 00 D2 00 00 00 1F 00 01 00 21 0B 00 00 1F 00 01 00 2C 14 00 00 17 00 01 00 02 00 06 00 00 00 00 00 1F 00 01 00 00 00 00 00 1F 00 01 00 FF 00 00 00 1F 00 01 00 04 01 00 00
1F 00 01 00 E1 00 00 00 1F 00 01 00 22 0B 00 00 1F 00 01 00 45 14 00 00 17 00 01 00 02 00 06 00 00 00 00 00 1F 00 01 00 00 00 00 00 1F 00 01 00 FF 00 00 00 1F 00 01 00 3B 01 00 00
1F 00 01 00 F0 00 00 00 1F 00 01 00 23 0B 00 00 1F 00 01 00 52 14 00 00 17 00 01 00 02 00 06 00 00 00 00 00 end

the tricky strings (a couple of latin alphabet names) at the end of the script
of koi x shin ai kanojo:
09 00 01 00 e6 0f 00 00 01 00 01 00 01 00 00 40
## ## ## ## 1f 00 01 00 01 00 00 40 0b 00 01 00
e1 0f 00 00
- hopefully this one will never have false hits since it's so specific

i have yet to identify any actual commands, though i suspect
"1f 00 01 00 ## ## ## ##" means "push ## ## ## ## on stack" or "send that value
to somewhere"

i don't currently have high hopes of matching the early lines of a file
(before the actual text) because low addresses => low numbers
=> more search hits => more false hits
