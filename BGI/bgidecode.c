/* unpack bgi/ethornell script files, no codepage conversion happens
   use garbro or something to extract packed script files from .arc files
   this program extracts anything that looks like text (or to be exact,
   every pointer into the last half of the file where text resides)

   having to explicitly specify (or redirect) output is a safety measure
   to avoid decoding and overwriting an existing decoded file by accident

   TODO: add support for not extracting non-script text

   v1.0: initial version
   v1.1: added support for format 0x10 (daitoshokan trial).
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

uint32_t opt_format;	// script format, 0x01 (newer games) or 0x10 (older games)

void usage() {
	fputs("bgidecode v1.1 by me in 2021\n\n",stderr);
	fputs("usage: bgidecode script-file\n\n",stderr);
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
	if(hasheader) offs=0x1c;
	// skip past framework._bs.function stuff
	offs+=getuint32(file+offs);
	// format thingy, 0x00000001 in all files i've seen so far
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
	while(ptr<earlieststring) {
		if(isstrcode(getuint32(file+ptr))) {
			ptr+=4;
			uint32_t str=getuint32(file+ptr)+offs;
			// only accept strings with pointer > strptr
			if(str>=strptr && ptr<str) {
				if(earlieststring>str) earlieststring=str;
				printf("<%u,%u,%zd>%s\n",ptr-offs,str-offs,strlen((char *)file+str),file+str);
			}
		}
		ptr+=4;
	}
}

int main(int argc,char **argv) {
	if(argc==1) usage();
	decode(argv[1]);
	return 0;
}
