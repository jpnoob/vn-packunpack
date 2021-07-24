# python 3
# pack files into (unencrypted) .int archive (catsystem 2 archive format)

import os
import sys
import glob

def writeuint32(buf,offset,val):
	buf[offset+0]=val&255
	buf[offset+1]=(val>>8)&255
	buf[offset+2]=(val>>16)&255
	buf[offset+3]=(val>>24)

def pack(path,out):
	files=glob.glob(path)
	outbuf=[ord('K'),ord('I'),ord('F')]+[0]*(5+0x48*len(files))
	index=0
	writeuint32(outbuf,4,len(files));
	for filename in glob.glob(path):
		# write filename to 8+0x48*index
		i=8+0x48*index
		s=os.path.basename(filename)
		for j in range(len(s)):
			outbuf[i+j]=ord(s[j])
		# read file
		f=open(filename,"rb")
		b=f.read()
		f.close()
		writeuint32(outbuf,i+0x44,len(b))
		writeuint32(outbuf,i+0x40,len(outbuf))
		outbuf+=b
		index+=1
	f=open(out,"wb")
	f.write(bytes(outbuf))
	f.close()
	print("done")

if len(sys.argv)!=3:
	print("cat2pack by me in 2021\n")
	print("usage: cat2pack <files> <outfile>\n")
	print("example: cat2pack script/*.cst scene.int,")
	print("packs all *.cst files in script/ into scene.int")
else:
	pack(sys.argv[1],sys.argv[2])
