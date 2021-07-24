# python 3
# original taken from chinesize/advhd/pack.py
# modified by me to support parameters and wildcards

import os
import struct
import sys
import glob

def packdir(path,fname):
	files=glob.glob(path)
	files.sort()
	base_size=8
	for f in files:
		base_size+=8+(len(os.path.basename(f))+1)*2
	fs=open(fname,'wb')
	fs.seek(base_size)
	idxes=[]
	curIdx=0
	for f in files:
		nf=open(f,'rb')
		stm=bytearray(nf.read())
		for i in range(len(stm)):
			stm[i]=((stm[i]<<2)|(stm[i]>>6))&255
		fs.write(stm)
		nf.close()
		bs=struct.pack('<II',len(stm),curIdx)
		curIdx+=len(stm)
		idxes.append(bs+os.path.basename(f).encode('utf-16-le')+b'\0\0')
	idxbin=b''.join(idxes)
	if len(idxbin)!=base_size-8:
		print("actual len "+str(len(idxbin))+", expected len"+str(base_size-8))
		raise Exception('idx size error')
	fs.seek(0)
	fs.write(struct.pack('<II',len(files),base_size-8))
	fs.write(idxbin)
	fs.close()

if len(sys.argv)!=3:
	print("usage: py arcpack files-to-be-packed arcfile\n")
	print("examples: py arcpack Rio1\*.ws2 Rio1.arc");
else:
	packdir(sys.argv[1],sys.argv[2])
