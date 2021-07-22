spaceconv

utility that converts spaces in text boxes from 0x20 to 0x81 0x40
(two-byte sequence in japanese code page). aka all spaces on lines that
begin with ".message". to avoid trouble, use tab to separate fields on those
lines instead of space (or separators will be converted and the game will
surely complain).

mashiro iro symphony doesn't show text if a line contains normal space,
but japanese space works fine
