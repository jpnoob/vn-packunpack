#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage() {
	puts("usage:");
	puts("extract text: nanaextr e textdata.dat > textfile.txt");
	puts("pack text: nanaextr p textfile.txt scenario.dat textdata.dat");
	puts("filenames above can be changed. note that uniextr e prints to stdout.");
	puts("the pack text option writes to scenario.dat.new and textdata.dat.new");
	puts("for packing, cenario.dat and textdata.dat must be the original unchanged files");
	puts("(even if they are renamed)");
	exit(0);
}

#define LARGE 1000

unsigned getint4(char *b,int i) {
	unsigned char *a=(unsigned char *)b;
	return a[i]+(a[i+1]<<8)+(a[i+2]<<16)+(a[i+3]<<24);
}

void writeint4(char *b,int i,unsigned val) {
	unsigned char *a=(unsigned char *)b;
	a[i]=val&255;
	a[i+1]=(val>>8)&255;
	a[i+2]=(val>>16)&255;
	a[i+3]=(val>>24);
}

void readfile(char *name,unsigned *len,char **b) {
	FILE *f=fopen(name,"rb");
	if(!f) printf("file %s couldn't be opened\n",name),exit(0);
	fseek(f,0,SEEK_END);
	*len=ftell(f);
	fseek(f,0,SEEK_SET);
	*b=malloc(*len);
	if(!*b) printf("out of memory\n"),exit(0);
	if(*len!=fread(*b,1,*len,f)) printf("didn't read enough bytes\n"),exit(0);
	fclose(f);
}

void writefile(char *name,unsigned len,char *b) {
	FILE *f=fopen(name,"wb");
	if(!f) printf("file %s couldn't be opened for writing\n",name),exit(0);
	if(len!=fwrite(b,1,len,f)) printf("didn't write enough bytes\n"),exit(0);
	fclose(f);
}

char *a;
char *ascr;
char *atxt;
char *anew;
int anewlen;
unsigned script_len,txt_len;

void extract(int argc,char **argv) {
	if(argc<1) printf("input file expected\n"),exit(0);
	unsigned len;
	readfile(argv[0],&len,&a);
	if(strncmp(a,"PJADV_TF",8)) printf("unknown signature, expected 'PJADV_TF''\n"),exit(0);
	int numstr=getint4(a,12);
	printf("<%d>",numstr);
	for(int i=0;i<12;i++) putchar(a[i]);
	putchar('\n');
	int at=16;
	for(int curstr=0;curstr<numstr;curstr++) {
		if(at>=len) printf("tried to read past end of file at string %d\n",curstr),exit(0);
		int straddr=at;
		printf("<%d,%d>",curstr,straddr);
		while(a[at]) putchar(a[at++]);
		putchar('\n');
		at++;
		if(at&1) at++;
	}
}

unsigned *ptrconv; // convert oldptr -> newptr
char *ptrseen;     // -1: not a ptr, 0: not seen, 1: we've seen oldptr in scenario.dat

int istextline(char *a,int pos) {
	return getint4(a,pos)==0x80000307 && (getint4(a,pos+20)==0x00030000 || getint4(a,pos+20)==0x00020000 || getint4(a,pos+20)==0x00000000) && (getint4(a,pos+24)==0x00000001 || getint4(a,pos+24)==0x00000002);
}

int ischapter(char *a,int pos) {
	return getint4(a,pos)==0x02000103 && getint4(a,pos+4)==0x30000000 && getint4(a,pos+8)==0x00000001 && getint4(a,pos+12)==0x02000103 && getint4(a,pos+20)==0x30000000 && getint4(a,pos+24)==0x01000d02;
}

int isunknownthingwithpointers1(char *a,int pos) {
	// routine is probably wrong, i've deactivated for now
	return 0;
	// first dword of these patterns look too weird, maybe they're part of the previous command
	if(getint4(a,pos)==0x00051c08 && getint4(a,pos+4)==0x01010203) return 1;
	if(getint4(a,pos)==0x01010101 && getint4(a,pos+4)==0x01010203) return 1;
	if(getint4(a,pos)==0x01000001 && getint4(a,pos+4)==0x01000d02) return 1;
	if(getint4(a,pos)==0x30000000 && getint4(a,pos+4)==0x01000d02) return 1;
	return 0;
}

// this matches chapter text (and more unknown text)
int isunknownthingwithpointers2(char *a,int pos) {
	// first dword of these patterns look too weird, maybe they're part of the previous command
	if(getint4(a,pos)==0x01010203) return 1;
	if(getint4(a,pos)==0x01000d02) return 1;
	return 0;
}

void updateptr(unsigned oldptr,char *ascr,int atscr,char *s) {
	if(oldptr>=txt_len) printf("error at %x: too large %s address %x\n",atscr,s,oldptr),exit(0);
	if(ptrseen[oldptr]<0) printf("error at %x: no %s string at address %x\n",atscr,s,oldptr),exit(0);
	ptrseen[oldptr]=1;
	writeint4(ascr,atscr,ptrconv[oldptr]);
}

void pack(int argc,char **argv) {
	char s[LARGE+1];
	if(argc<3) printf("infiles (.txt, scenario.dat, textdata.dat) must be specified\n"),exit(0);
	FILE *f=fopen(argv[0],"rb");
	if(!f) printf("file %s couldn't be opened\n",argv[0]),exit(0);

	readfile(argv[1],&script_len,&ascr);
	readfile(argv[2],&txt_len,&atxt);

	/* prepare text.dat.new */
	anewlen=20000000;
	anew=malloc(anewlen);
	if(!anew) printf("out of memory\n"),exit(0);

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

	ptrconv=malloc(sizeof(unsigned)*txt_len);
	if(!ptrconv) puts("out of memory while allocating ptrconv table"),exit(0);
	memset(ptrconv,-1,sizeof(unsigned)*txt_len);
	ptrseen=malloc(txt_len);
	if(!ptrseen) puts("out of memory while allocating ptrseen table"),exit(0);
	memset(ptrseen,-1,txt_len);
	/* for each line in input file:
	   - ignore if it's not starting with <
	   - copy new string to new textdata.dat
     - populate table of old strptr -> new strptr
	*/
	int at=16; // position in new textdata.dat
	int line=0;
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		int lineno,strptr;
		sscanf(s,"<%d,%d>",&lineno,&strptr);
		if(lineno!=line) printf("sanity error, expected line %d, found %d\n",line,lineno),exit(0);
		ptrconv[strptr]=at;
		ptrseen[strptr]=0;
		/* copy string to textdata.dat.new */
		int sp=0;
		while(s[sp]!='>') sp++;
		sp++;
		while(s[sp]!=13 && s[sp]!=10 && s[sp]) anew[at++]=s[sp++];
		// i'm not sure exactly how to avoid showing two strings after one another
		// but at last 2 terminating zeroes + even address seems to work
		anew[at++]=0;
		anew[at++]=0;
		if(at&1) anew[at++]=0;
		if(at>anewlen) printf("text.dat.new too large\n"),exit(0);
		line++;
	}

	// update all string pointers in scenario.dat
	for(int atscr=0;atscr<=script_len-0x1c;atscr+=4) {
		if(istextline(ascr,atscr)) {
			// speaker's name (optional)
			unsigned oldptr=getint4(ascr,atscr+8);
			if(oldptr) updateptr(oldptr,ascr,atscr+8,"speaker name");
			// text line (mandatory)
			updateptr(getint4(ascr,atscr+12),ascr,atscr+12,"text line");
/*
		} else if(ischapter(ascr,atscr)) {
			// chapter name
			updateptr(getint4(ascr,atscr+28),ascr,atscr+28,"chapter name");
*/
		} else if(isunknownthingwithpointers1(ascr,atscr)) {
			// pointer to unknown something (mandatory)
			updateptr(getint4(ascr,atscr+8),ascr,atscr+8,"unknown1");
			// pointer to another unknown something (mandatory)
			unsigned oldptr=getint4(ascr,atscr+20);
			if(oldptr && oldptr!=0x01000302 && oldptr!=0x02000103) updateptr(oldptr,ascr,atscr+20,"unknown2");
		} else if(isunknownthingwithpointers2(ascr,atscr)) {
			// pointer to unknown something (mandatory)
			// also matches chapter name
			updateptr(getint4(ascr,atscr+4),ascr,atscr+4,"unknown1");
		}
	}
	// sanity check (unseen text => unchanged pointers)
	// because wrong (unchanged) pointers is likely a very bad thing
	for(int i=0;i<txt_len;i++) if(!ptrseen[i]) {
		printf("strptr %x not seen\n",i);
	}

	/* write back files */
	writefile("scenario.dat.new",script_len,ascr);
	writefile("textdata.dat.new",at,anew);
	printf("inserting text done\n");
}

int main(int argc,char **argv) {
	if(argc<2) usage();
	if(!strcmp(argv[1],"e")) extract(argc-2,argv+2);
	if(!strcmp(argv[1],"p")) pack(argc-2,argv+2);
	else printf("illegal option %s\n",argv[1]);
	return 0;
}
