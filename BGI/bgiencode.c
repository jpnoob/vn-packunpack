/* pack bgi/ethornell script files, no codepage conversion happens
   use garbro or something to extract packed script files from .arc files
   use bgidecode to decode script files

   lines that don't start with <int,int,int> are ignored by the encoder and
   can be used for comments

   i have no idea if text in shift-jis (codepage 932) can have bytes containing
   zero in the middle of a string (for example as the second byte of a two-byte
   character). this program can break horribly if that's possible!

   TODO add support for encoding incomplete script files (and not touching
   stuff in the binary that's not in the script)

   v1.0: initial release
   v1.1: added support for format 0x10 (daitoshokan trial). in particular,
         the program should support for encoding of incomplete text files
         (not tested yet).
         though the program is currently bugged, it doesn't find all strings
         in daitoshokan. fortunately, i didn't break the 0x01 format.
         use bgi_script_tools (tlwiki.org) for older formats instead.
         it also doesn't work on main from hajirabu promo, so i'd like to fix it
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void usage() {
	fputs("bgiencode v1.1- by me in 2021\n\n",stderr);
	fputs("usage: bgiencode decoded-script-file\n\n",stderr);
	fputs("kind of assumes that the input file has .txt extension\n",stderr);
	fputs("uses existing encoded file without extension (whether it was .txt or not)\n",stderr);
	fputs("writes to file with .new added to input filename\n",stderr);
	fputs("example: bgiencode 01_prologue.txt\n",stderr);
	fputs("uses 01_prologue and packs to 01_prologue.txt.new\n",stderr);
	exit(0);
}

void error(char *s) {
	fputs(s,stderr);
	exit(1);
}

int opt_format;

int isstrcode(uint32_t c) {
	switch(opt_format) {
	case 0x01:
		// 0x7f is also a string pointer in file format 0x01
		return c==0x03 || c==0x7f;
	case 0x10:
		return c==0x03 || c==0x7f;
	default:
		return 0;
	}
}

#define MAXLEN 10000000
#define MAXS 200
#define MAXL 100000
unsigned char file[MAXLEN];
unsigned char oldfile[MAXLEN];
char line[MAXL];

uint32_t getuint32(unsigned char *p) {
	return p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24);
}

void writeuint32(unsigned char *p,uint32_t val) {
	p[0]=val&255;
	p[1]=(val>>8)&255;
	p[2]=(val>>16)&255;
	p[3]=(val>>24)&255;
}

uint32_t findduplicate(char *s,uint32_t earlieststring,uint32_t endptr) {
	while(earlieststring<endptr) {
		if(!strcmp(s,(char *)file+earlieststring)) return earlieststring;
		earlieststring+=strlen((char *)file+earlieststring)+1;
	}
	return 0;
}

/* insert string into storage and change pointer
   p: string to insert
   earlieststring: start of text area
   curptr: current pointer to end of text area (and end of file)
   newoffs: where to write pointer to string
   offs: header size
*/
void insertstring(char *p,uint32_t earlieststring,uint32_t *curptr,uint32_t newoffs,uint32_t offs) {
	// check if string is duplicate
	uint32_t dup=findduplicate(p,earlieststring,*curptr);
	if(!dup) {
		writeuint32(file+newoffs,*curptr-offs);
		strncpy((char *)file+*curptr,p,strlen(p));
		file[*curptr+strlen(p)]=0;
		(*curptr)+=strlen(p)+1;
	} else writeuint32(file+newoffs,dup-offs);
}

void encode(char *in) {
	// open input file (text file, decoded script)
	FILE *f=fopen(in,"r");
	// read bin file
	char in2[MAXS];
	strncpy(in2,in,MAXS-1);
	for(int i=strlen(in2)-1;i>=0;i--) {
		char c=in2[i];
		in2[i]=0;
		if(c=='.') break;
	}
	FILE *f2=fopen(in2,"rb");
	if(!f2) error("couldn't open file for reading\n");
	fseek(f2,0,SEEK_END);
	size_t filelen=ftell(f2);
	fseek(f2,0,SEEK_SET);
	if(filelen>MAXLEN) error("file too long, increase MAXLEN and recompile\n");
	if(filelen!=fread(file,1,filelen,f2)) error("couldn't read the entire file\n");
	if(fclose(f2)) error("error closing file after reading\n");
	// store an additional, untouched copy of the input file
	memcpy(oldfile,file,filelen);
	// prepare filename for out file
	char out[MAXS];
	strncpy(out,in,MAXS-1);
	strncat(out,".new",MAXS-1-strlen(out));

	// get header length and stuff
	char header[0x1c]="BurikoCompiledScriptVer1.00";
	int hasheader=1;
	for(int i=0;i<0x1c;i++) if(file[i]!=header[i]) {
		hasheader=0;
		break;
	}
	// offs = total header length
	int offs=0;
	if(hasheader) offs=0x1c;
	// skip past framework._bs.function stuff
	offs+=getuint32(file+offs);
	// format thingy, 0x00000001 in all files i've seen so far
	opt_format=getuint32(file+offs);
	// strptr: address where strings start (a bit too low)
	uint32_t strptr=getuint32(file+offs+4);
	if(opt_format!=1 && opt_format!=0x10) fprintf(stderr,"unknown format %u\n",opt_format);

	// go through binary file and find start of text buffer
	// (take minimum of all actual text pointers)
	uint32_t ptr=offs+8;
	uint32_t earlieststring=0xffffffff;
	while(ptr<earlieststring) {
		if(isstrcode(getuint32(file+ptr))) { // next dword is pointer to string
			ptr+=4;
			uint32_t str=getuint32(file+ptr)+offs;
			// only accept strings with pointer > strptr
			if(str>=strptr && earlieststring>str && str>ptr) earlieststring=str;
		}
		ptr+=4;
	}

	uint32_t curptr=earlieststring;
	uint32_t ptr2=8; // pointer minus header size (offs)
	// go through each line in script and insert text
	while(fgets(line,MAXL,f)) {
		int binoffs; // offset into binary file (excludes header)
		uint32_t throw;   // read strptr and throw away since we generate a new one
		int len;      // length of text string
		int r;        // number of chars read
		// ignore any line that doesn't start with <int,int,int>, so literally
    // anything else can be used for comments
		if(3!=sscanf(line,"<%d,%u,%u>%n",&binoffs,&throw,&throw,&r)) continue;
		// dumped script files (for format 0x10) don't contain all texts.
		// go through original binary and add missing texts to storage
		while(ptr2<binoffs-4) {
			// BUG, this routine misses "bss.h" and a ton of other strings before
			// "black" in tos00010 (actually like 0x300 worth of total string length)
			if(isstrcode(getuint32(oldfile+ptr2+offs))) {
				ptr2+=4;
				// ignore pointers to very early in file (within header)
				if(getuint32(oldfile+ptr2+offs)<binoffs) { ptr2+=4; continue; }
//				printf("process bin %u %s\n",ptr2,oldfile+getuint32(oldfile+ptr2+offs)+offs);
				insertstring((char *)oldfile+getuint32(oldfile+ptr2+offs)+offs,earlieststring,&curptr,ptr2+offs,offs);
			}
			ptr2+=4;
		}
		if(ptr2==binoffs-4) ptr2+=8;
		// remove trailing linebreak added by fgets
		len=strlen(line+r);
		while(line[r+len-1]==0x0d || line[r+len-1]==0x0a) line[r+(--len)]=0;
		line[r+len]=0;
//		printf("process txt %u %s\n",binoffs,line+r);
		// insert string
		insertstring(line+r,earlieststring,&curptr,binoffs+offs,offs);
	}
	fclose(f);
	// write new binary file
	f2=fopen(out,"wb");
	if(curptr!=fwrite(file,1,curptr,f2)) error("error writing output file");
	if(fclose(f2)) error("error closing output file");
	printf("copy %s to the game directory and rename the file to %s\n",out,in2);
}

int main(int argc,char **argv) {
	if(argc==1) usage();
	encode(argv[1]);
	return 0;
}
