# python 3
# read script files in mashiro iro symphony, check length of lines in script
# and warn about too long lines

import sys
import glob
import codecs

opt_linelen=47
opt_spacewarn=True

def linelength(s):
	l=0
	for i in range(len(s)):
		if 31<ord(s[i]):l+=1
	return l

def wrap(path):
	files=glob.glob(path)
	for filename in files:
		f=codecs.open(filename,'r','cp932')
		lines=f.readlines()
		f.close
		lineno=1
		for line in lines:
			l=line.split('\t')
			if l[0]=='.message' and len(l)==5:
				if(opt_spacewarn and len(l[4].split(' '))>1):
					print("line "+str(lineno)+" has non-japanese space")
				m=l[4].split('\\n')
				for n in m:
					if linelength(n)>opt_linelen:
						print("line "+str(lineno)+" is long ["+n.rstrip('\n\r')+"]")
			lineno+=1

sys.argv=sys.argv[1:]
while len(sys.argv)>0 and sys.argv[0][0]=='-':
	if sys.argv[0]=='-j': opt_spacewarn=False
	elif sys.argv[0][:2]=='-n': opt_linelen=int(sys.argv[0][2:])
	else: print("unsupported parameter "+sys.argv[0])
	sys.argv=sys.argv[1:]
if len(sys.argv)<1:
	print("mashirowrap by me\n")
	print("usage: python mashirowrap.py [-j] [-n<num>] files [more-files]\n")
	print("example: python mashirowrap.py -n40 *.sc")
	print("go through all .sc files and warn about lines longer than 40 chars\n")
	print("-n<num>: max number of chars per line. defaults to 47 if unspecified")
	print("-j: turn off warning about normal space (0x20)")
	print("files can be specified as filenames or wilcards,")
	print("and with one or more parameters")
else:
	for x in sys.argv: wrap(x)
