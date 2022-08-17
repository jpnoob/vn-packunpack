#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unimatch.h"

#define HUGE 1000

void usage() {
	puts("usage: script.src text.dat");
	puts("checks if we found a surefire string pointer match for everything in text.dat");
	puts("into script.src.");
	exit(0);
}

unsigned scrlen,txtlen;
char *ascr;
char *atxt;

int *ptrconv;   // -1: not a ptr, otherwise line number that has index as address
int *strptr;    // strptr[i]: address of line i (0-indexed)
int *count;     // count[i]: number of times line i is referenced by script

void process(char *a,int at) {
	unsigned v=getint4(ascr,at);
	if(v>=scrlen) {
		printf("pointer %x out of bounds at script.src address %x\n",v,at);
		return;
	}
	if(ptrconv[v]<0) {
		printf("%x not a strptr at script.src address %x\n",v,at);
		return;
	}
	if(count[ptrconv[v]]<HUGE) count[ptrconv[v]]++;
}

int main(int argc,char **argv) {
	if(argc<3) usage();
	readfile(argv[1],&scrlen,&ascr);
	readfile(argv[2],&txtlen,&atxt);

	ptrconv=malloc(sizeof(int)*txtlen);
	if(!ptrconv) puts("out of memory"),exit(0);
	int lines=getint4(atxt,12);
	strptr=malloc(sizeof(int)*lines);
	if(!strptr) puts("out of memory"),exit(0);
	count=malloc(sizeof(int)*lines);
	if(!count) puts("out of memory"),exit(0);

	memset(count,0,sizeof(int)*lines);
	memset(ptrconv,-1,sizeof(int)*txtlen);
	// traverse text.dat to build strptr[]
	int at=16;
	for(int i=0;i<lines;i++) {
		if(at>=txtlen) puts("text.dat has too few lines"),exit(0);
		strptr[i]=at;
		ptrconv[at]=i;
		int lineno=getint4(atxt,at);
		at+=4;
		if(i!=lineno) printf("expected line number %d, found %d\n",i,lineno),exit(0);
		while(at<txtlen && atxt[at]) at++;
		at++;
	}
	if(at<txtlen) printf("text.data has too many lines (at pos %d, len %d)\n",at,txtlen),exit(0);

	// traverse script.src and find all string references
	process_script(scrlen,ascr);

	// output results:
	// strings that were never referenced (could be true negative hit)
	// strings that where referenced more than once (could be false positive hit)
	for(int i=0;i<lines;i++) {
		if(count[i]!=1) printf("line %d at pos %x referenced %d times\n",i,strptr[i],count[i]);
	}
	return 0;
}
