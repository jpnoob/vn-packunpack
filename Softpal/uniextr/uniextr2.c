#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unimatch.h"

void usage() {
	printf("usage:\n");
	printf("extract text: uniextr2 e TEXT.DAT > textfile.txt\n");
	printf("pack text: uniextr2 p textfile.txt SCRIPT.SRC TEXT.DAT\n");
	printf("filenames above can be changed. note that uniextr e prints to stdout.\n");
	printf("the pack text option writes to SCRIPT.SRC.new and TEXT.DAT.new.\n");
	printf("for packing, SCRIPT.SRC and TEXT.DAT must be the sames one as we originally\n");
	printf("extracted the .txt file from (even if they are renamed.)\n");
	exit(0);
}

void writefile(char *name,unsigned len,char *b) {
	FILE *f=fopen(name,"wb");
	if(!f) printf("file %s couldn't be opened for writing\n",name),exit(0);
	if(len!=fwrite(b,1,len,f)) printf("didn't write enough bytes\n"),exit(0);
	fclose(f);
}

#define LARGE 1000
#define HUGE 1000

char *a;
char *ascr;
char *atxt;
char *anew;
int anewlen;
unsigned script_len,txt_len;

void extract(int argc,char **argv) {
	if(argc<1) printf("in-file expected\n"),exit(0);
	unsigned len;
	readfile(argv[0],&len,&a);
	if(strncmp(a,"$TEXT_LIST__",12) && strncmp(a,"_TEXT_LIST__",12)) {
		printf("unknown signature, expected '$TEXT_LIST__' or '_TEXT_LIST__'\n");
		exit(0);
	}
	int numstr=getint4(a,12);
	printf("<%d>",numstr);
	for(int i=0;i<12;i++) putchar(a[i]);
	putchar('\n');

	int at=16;
	for(int curstr=0;curstr<numstr;curstr++) {
		if(at>=len) printf("tried to read past end of file at string %d\n",curstr),exit(0);
		int straddr=at;
		int curstr2=getint4(a,at);
		at+=4;
		if(curstr!=curstr2) printf("expected string id %d, found %d\n",curstr,curstr2),exit(0);
		printf("<%d,%d>",curstr,straddr);
		while(a[at]) putchar(a[at++]);
		putchar('\n');
		at++;
	}
}

int *ptrconv;   // -1: not a ptr, otherwise line number that has index as address
int *oldptr;    // oldptr[i]: address of line i (0-indexed)
int *newptr;    // newptr[i]: new pointer to  line i (0-indexed)
int *count;     // count[i]: number of times line i is referenced by script

void process(char *a,int at) {
	unsigned v=getint4(ascr,at);
	if(v>=script_len) {
		printf("pointer %x out of bounds at script.src address %x\n",v,at);
		return;
	}
	if(ptrconv[v]<0) {
		printf("%x not a strptr at script.src address %x\n",v,at);
		return;
	}
	int lineno=ptrconv[v];
	if(newptr[lineno]==v) return; // no change
//	printf("changed ptr at line %d at addr %x: %x -> %x\n",lineno,at,v,newptr[lineno]);
	writeint4(ascr,at,newptr[lineno]);
	if(count[ptrconv[v]]<HUGE) count[ptrconv[v]]++;
}

void pack(int argc,char **argv) {
	char s[LARGE+1];
	if(argc<3) printf("infiles (.txt, script.src, text.dat) must be specified\n"),exit(0);
	FILE *f=fopen(argv[0],"rb");
	if(!f) printf("file %s couldn't be opened\n",argv[0]),exit(0);

	readfile(argv[1],&script_len,&ascr);
	readfile(argv[2],&txt_len,&atxt);

	/* prepare text.dat.new */
	anewlen=20000000;
	anew=malloc(anewlen);
	if(!anew) printf("out of memory\n"),exit(0);

	ptrconv=malloc(sizeof(int)*txt_len);
	if(!ptrconv) puts("out of memory"),exit(0);
	int lines=getint4(atxt,12);
	oldptr=malloc(sizeof(int)*lines);
	if(!oldptr) puts("out of memory"),exit(0);
	newptr=malloc(sizeof(int)*lines);
	if(!newptr) puts("out of memory"),exit(0);
	count=malloc(sizeof(int)*lines);
	if(!count) puts("out of memory"),exit(0);
	memset(count,0,sizeof(int)*lines);
	memset(ptrconv,-1,sizeof(int)*txt_len);

	int numlines;
	char signature[100];
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		sscanf(s,"<%d>%99s",&numlines,signature);
		if(strncmp(signature,atxt,12)) printf("signature %s doesn't match the one in %s\n",signature,argv[2]),exit(0);
		if(numlines!=getint4(atxt,12)) printf("wrong total number of lines, text file has %d, dat file has %u\n",numlines,getint4(atxt,12)),exit(0);
		break;
	}
	for(int i=0;i<12;i++) anew[i]=signature[i];
	writeint4(anew,12,numlines);
	/* for each line in f
	   - ignore if it's not starting with <
	   - copy new string to new text.dat
	   - update ptrconv array so that ptrconv[oldptr]=lineno
	   - update newptr array so that newptr[lineno]=newptr
	*/
	int at=16;
	int line=0;
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		int lineno,strptr;
		sscanf(s,"<%d,%d>",&lineno,&strptr);
		if(lineno!=line) printf("sanity error, expected line %d, found %d\n",line,lineno),exit(0);
		oldptr[lineno]=strptr;
		ptrconv[strptr]=lineno;
		newptr[lineno]=at;
		/* copy string to text.dat.new */
		writeint4(anew,at,lineno);
		at+=4;
		int sp=0;
		while(s[sp]!='>') sp++;
		sp++;
		while(s[sp]!=13 && s[sp]!=10 && s[sp]) anew[at++]=s[sp++];
		anew[at++]=0;
		if(at>anewlen) printf("text.dat.new too large\n"),exit(0);
		line++;
	}
	/* stage 2:
	   - scan through script.src, find all string pointers and update them to point to new strings
	   - update count[lineno] for each time pointer to line is changed
	   - warn about changed strings that aren't matched to anything, and matched more than once
	*/
	unsigned newtextlen=at;
	process_script(script_len,ascr);
	for(int i=0;i<lines;i++) if(newptr[i]!=oldptr[i] && count[i]==0) printf("line %d attempted changed, but not matched in script.src\n",i);

	writefile("SCRIPT.SRC.new",script_len,ascr);
	writefile("TEXT.DAT.new",newtextlen,anew);
	printf("inserting text done\n");
}

int main(int argc, char **argv) {
	if(argc<2) usage();
	if(!strcmp(argv[1],"e")) extract(argc-2,argv+2);
	else if(!strcmp(argv[1],"p")) pack(argc-2,argv+2);
	else printf("illegal command %s, use e or p\n",argv[1]);
	return 0;
}
