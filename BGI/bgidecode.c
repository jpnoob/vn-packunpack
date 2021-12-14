/* unpack bgi/ethornell script files, no codepage conversion happens
   use garbro or something to extract packed script files from .arc files
   this program extracts anything that looks like text (or to be exact,
   every pointer into the last half of the file where text resides)

   having to explicitly specify (or redirect) output is a safety measure
   to avoid decoding and overwriting an existing decoded file by accident

   v1.0: initial version
   v1.1: added support for format 0x10 (daitoshokan trial).
   v1.2: added very ugly support for not extracting non-script text
   v1.3: added proper support for not extracting non-script text
   v1.4: added script line number in place of the string length parameter
         (third number in <x,y,z> in .txt file), string length was thrown
         away by the encoder anyway. only done in -s mode. character line has
         the previous line number (too lazy to fix it now)
   v1.4a: line numbers are now correct for character names (same as the text
          line it belongs to). fixed -s for v1.72 (newer games) where it
          skipped actual text lines
   v1.4b: forgot to extract text in choices in -s mode in v1.72. this is
          probably still bugged in v1.69 in -s mode
   v1.5: support old v1.69 (edelweiss eiden fantasia)
         TODO convert linebreaks in line to "\n", and the other way in the
         encoder
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

uint32_t opt_format;      // script format, 0x01 (newer games) or 0x10 (older games)
bool opt_nocommand=false; // true: remove commands not part of script

void usage() {
	fputs("bgidecode v1.5 by me in 2021\n\n",stderr);
	fputs("usage: bgidecode [-s] script-file\n\n",stderr);
	fputs("-s: skip extraction of non-script text\n",stderr);
	fputs("output is sent to stdout, redirect it yourself\n",stderr);
	fputs("example: bgidecode 01_prologue > 01_prologue.txt\n",stderr);
	exit(0);
}

void error(char *s) {
	fputs(s,stderr);
	exit(1);
}

#define MAXLEN 10000000
unsigned char file[MAXLEN];

uint32_t name172a[]={0,-1,0x3f,2,0xfe,0,1,0,1,0,0,3};
uint32_t text172a[]={0,-1,0x3f,2,0xfe,0,1,0,1,0,0,-1,-1,3};
uint32_t name172b[]={0,-1,0x3f,2,0xfe,0,0,0,0,0,0,3};
uint32_t text172b[]={0,-1,0x3f,2,0xfe,0,0,0,0,0,0,-1,-1,3};
uint32_t choice172[]={0x10,0,4,0x20,0x11,2,4,3};
//uint32_t choice171[]={0,-1,0x3f,2,0xfe,3}; // not safe to enable without getting a lot of garbage
uint32_t ctext172[]={0x10,0,4,0x20,0x11,2,4,3,-1,9,2,3};
uint32_t name169[]={0,1,0,1,0,0,3};
uint32_t text169[]={0,1,0,1,0,0,-1,-1,3};

enum state {NOTHING,NAME,TEXT,CHOICE};

uint32_t getuint32(unsigned char *p) {
	return p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24);
}

int isstrcode(uint32_t c) {
	switch(opt_format) {
	case 0x01:
		return c==0x03;
	case 0x10:
		// it seems code 0x7f never points to script text
		return c==0x03;
	default:
		return 0;
	}
}

void decode(char *in) {
	FILE *f=fopen(in,"rb");
	if(!f) error("couldn't open file for reading\n");
	fseek(f,0,SEEK_END);
	size_t filelen=ftell(f);
	fseek(f,0,SEEK_SET);
	if(filelen>MAXLEN) error("file too long, increase MAXLEN and recompile\n");
	if(filelen!=fread(file,1,filelen,f)) error("couldn't read the entire file\n");
	if(fclose(f)) error("error closing file after reading\n");

	char header[0x1c]="BurikoCompiledScriptVer1.00";
	int hasheader=1;
	for(int i=0;i<0x1c;i++) if(file[i]!=header[i]) {
		hasheader=0;
		break;
	}

	// calculate offset for start of stuff and do other init stuff
	// offs = total header length
	int offs=0;
	if(hasheader) {
		offs=0x1c;
		// skip past framework._bs.function stuff
		offs+=getuint32(file+offs);
	}
	// format thingy: 0x1 (v1.72), 0x10 (v1.69 with header), 
	// strptr: rough start of strings (underestimate)
	uint32_t strptr=0;
	opt_format=getuint32(file+offs);
	strptr=getuint32(file+offs+4);
	if(opt_format!=0x01 && opt_format!=0x10) fprintf(stderr,"unknown format %u, all bets are off\n",opt_format);

	// loop through script and find strings
	uint32_t ptr=offs+8;
	// we don't know the actual end of script, so take minimum across all string
	// pointers (except pointers that are way too low, i.e. below strptr)
	uint32_t earlieststring=0xffffffff;
	int lineno=0;
	enum state lastthing;
	enum state thisthing=NOTHING;
	while(ptr<earlieststring) {
		if(isstrcode(getuint32(file+ptr))) {
			ptr+=4;
			uint32_t str=getuint32(file+ptr)+offs;
			// only accept strings with pointer > strptr
			if(str>=strptr && ptr<str) {
				if(earlieststring>str) earlieststring=str;
				bool isnametext=true;
				if(opt_nocommand) {
					// only accept strings used for character and text
					int i,j;
					isnametext=false;
					lastthing=thisthing;
					thisthing=NOTHING;
					if(opt_format==1) {
						// check if current string is name in v1.72
						for(i=-0x30,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(name172a[j]<0xffffffffu && getuint32(file+ptr+i)!=name172a[j] && getuint32(file+ptr+i)!=name172b[j]) break;
						}
						if(!i) isnametext=true,thisthing=NAME;
						// check if current string is text in v1.72
						for(i=-0x38,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(text172a[j]<0xffffffffu && getuint32(file+ptr+i)!=text172a[j] && getuint32(file+ptr+i)!=text172b[j]) break;
						}
						if(!i) isnametext=true,thisthing=TEXT;
						// check if current string is a choice in v1.72
						for(i=-0x20,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(choice172[j]<0xffffffffu && getuint32(file+ptr+i)!=choice172[j]) break;
						}
						if(!i) isnametext=true,thisthing=CHOICE;
						// check if current string is text associated with a choice in v1.72
						for(i=-0x30,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(ctext172[j]<0xffffffffu && getuint32(file+ptr+i)!=ctext172[j]) break;
						}
						if(!i) isnametext=true,thisthing=TEXT;
						// check if current string is a choice in v1.71
						// this enables a lot of garbage in general, disable or improve check
/*
						for(i=-0x18,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(choice171[j]<0xffffffffu && getuint32(file+ptr+i)!=choice171[j]) break;
						}*/
						if(!i) isnametext=true,thisthing=CHOICE;
					} else if(opt_format==0x10) {
						// check if current string is name in v1.69
						for(i=-0x1c,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(name169[j]<0xffffffffu && getuint32(file+ptr+i)!=name169[j]) break;
						}
						if(!i) isnametext=true,thisthing=NAME;
						// check if current string is text in v1.69
						for( i=-0x24,j=0;i<0;i+=4,j++) {
							if(ptr+i<0) break;
							if(text169[j]<0xffffffffu && getuint32(file+ptr+i)!=text169[j]) break;
						}
						if(!i) isnametext=true,thisthing=TEXT;
					}
				}
				if(isnametext) {
					// skip line consisting of just 81 40 (japanese space) that slipped through
					if(!(opt_nocommand && file[str]==0x81 && file[str+1]==0x40 && !file[str+2])) {
						// output line number properly in -s mode, otherwise length of string
						if(opt_nocommand) {
							if(lastthing==NOTHING) lineno++;
							else if(lastthing==TEXT) lineno++;
							printf("<%u,%u,%d>%s\n",ptr-offs,str-offs,lineno,file+str);
						} else printf("<%u,%u,%zd>%s\n",ptr-offs,str-offs,strlen((char *)file+str),file+str);
					}
				}
			}
		}
		ptr+=4;
	}
}

int main(int argc,char **argv) {
	if(argc>1 && !strcmp(argv[1],"-s")) {
		opt_nocommand=true;
		argc--; argv++;
	}
	if(argc==1) usage();
	decode(argv[1]);
	return 0;
}
