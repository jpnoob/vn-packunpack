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
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void usage() {
	fputs("bgiencode v1.0 by me in 2021\n\n",stderr);
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

#define MAXLEN 10000000
#define MAXS 200
#define MAXL 100000
unsigned char file[MAXLEN];
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
	uint32_t format=getuint32(file+offs);
	// strptr: address where strings start (a bit too low)
	uint32_t strptr=0;
	if(format==1) strptr=getuint32(file+offs+4);
	else fprintf(stderr,"unknown format %u\n",format);

	// go through binary file and find start of text buffer
	// (take minimum of all actual text pointers)
	uint32_t ptr=offs+8;
	uint32_t earlieststring=0xffffffff;
	while(ptr<earlieststring) {
		if(getuint32(file+ptr)==3) { // 3 = next dword is pointer to string
			ptr+=4;
			uint32_t str=getuint32(file+ptr)+offs;
			// only accept strings with pointer > strptr
			if(str>=strptr && earlieststring>str) earlieststring=str;
		}
		ptr+=4;
	}

	uint32_t curptr=earlieststring;
	// go through each line in script and insert text
	while(fgets(line,MAXL,f)) {
		int binoffs; // offset into binary file (excludes header)
		uint32_t throw;   // read strptr and throw away since we generate a new one
		int len;      // length of text string
		int r;        // number of chars read
		// ignore any line that doesn't start with <int,int,int>, so literally
    // anything else can be used for comments
		if(3!=sscanf(line,"<%d,%u,%u>%n",&binoffs,&throw,&throw,&r)) continue;
		// remove trailing linebreak added by fgets
		len=strlen(line+r);
		while(line[r+len-1]==0x0d || line[r+len-1]==0x0a) line[r+(--len)]=0;
		line[r+len]=0;
		// insert string
		int newoffs=binoffs+offs;
		// check if string is duplicate
		uint32_t dup=findduplicate(line+r,earlieststring,curptr);
		if(!dup) {
			writeuint32(file+newoffs,curptr-offs);
			strncpy((char *)file+curptr,line+r,len);
			file[curptr+len]=0;
			curptr+=len+1;
		} else writeuint32(file+newoffs,dup-offs);
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
