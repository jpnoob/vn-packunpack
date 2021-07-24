# python 3
# original taken from chinesize/advhd/pack.py
# modified by me to support parameters and wildcards

import os
import struct
import sys

def packdir(files,fname):
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

if len(sys.argv)<3:
	print("usage: arcpack files-to-be-packed arcfile")
	print("files-to-be-packed is a list of 1 or more files/wildcards.\n")
	print("examples: arcpack Rio1\*.ws2 Rio1.arc");
	print("          arcpack dir\* archive.arc");
	print("          arcpack file1 file2 file3 packed.arc");
	print("          arcpack dir1\*.txt dir2\*.png package.arc");
else:
	packdir(sys.argv[1:len(sys.argv)-1],sys.argv[len(sys.argv)-1])
