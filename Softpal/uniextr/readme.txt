extract and re-insert text from TEXT.DAT from games using softpal adv system engine

can add lines with comments in the resulting .txt file, lines not starting
with < are treated as comments and are fully ignored

don't change the numbers in <>, or things may crash or not work as intended

don't change text that's not part of the script, or things may crash
(string pointer finding might work even worse in those cases)

use garbro to unpack .pac files
use unipack (in the same repo) to repack .pac files
