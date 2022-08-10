# traverse directory structure and xor all .ogg files
# use whenever i'm desperate to extract .ogg files from games when
# i know they're xor'd with a single 1-byte key, and i don't know how to
# find the key properly
# should be easy to make a similar tool for a few more formats

import glob

for filename in glob.iglob('**/*.ogg', recursive=True):
  print(filename)
  d=bytearray(open(filename,'rb').read())
  if len(d)>0:
    key=d[0]^0x4f
    for i in range(0,len(d)): d[i]^=key
    open(filename,'wb').write(d)
