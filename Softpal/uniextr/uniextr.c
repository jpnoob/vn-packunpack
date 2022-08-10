#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage() {
	printf("usage:\n");
	printf("extract text: uniextr e TEXT.DAT > outfile\n");
	printf("pack text: uniextr p textfile.txt SCRIPT.SRC TEXT.DAT\n");
	printf("filenames above can be changed. note that uniextr e prints to stdout.\n");
	printf("the pack text option writes to SCRIPT.SRC.new and TEXT.DAT.new\n");
	printf("for packing, SCRIPT.SRC and TEXT.DAT must be the original unchanged files\n");
	printf("(even if they are renamed)\n");
	exit(0);
}

#define LARGE 1000

char *a;
char *ascr;
char *atxt;
char *anew;
int anewlen;

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

int isptr(char *a,int pos,int ptr) {
	if(pos<4) return 0;
	unsigned x=getint4(a-4,pos);
	unsigned y=getint4(a,pos);
	return (x&0xffffffff)==0x1f+0x10000 && y==ptr;
	return x==ptr;
}

void pack(int argc,char **argv) {
	char s[LARGE+1];
	if(argc<3) printf("infiles (.txt, script.src, text.dat) must be specified\n"),exit(0);
	FILE *f=fopen(argv[0],"rb");
	if(!f) printf("file %s couldn't be opened\n",argv[0]),exit(0);

	unsigned script_len,txt_len;
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
	/* for each line in f
	   - ignore if it's not starting with <
	   - copy new string to new text.dat
	   - if string pointer has changed, copy string pointer to script.src
	   - do some magic to find the correct pointer in script.src
	*/
	int at=16;
	int atscr=4; /* position in script file */
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		int lineno,strptr;
		sscanf(s,"<%d,%d>",&lineno,&strptr);
		if(strptr!=at) {
			while(1) {
				// TODO this part is not very robust
				if(atscr+8>=script_len) printf("reached end of script file without finding text %d\n",lineno),exit(0);
				if(isptr(ascr,atscr,strptr)) break;
				atscr+=4;
			}
			/* string pointer has changed, we must change the corresponding pointer in script.src */
			writeint4(ascr,atscr,at);
		}
		/* copy string to text.dat.new */
		writeint4(anew,at,lineno);
		at+=4;
		int sp=0;
		while(s[sp]!='>') sp++;
		sp++;
		while(s[sp]!=13 && s[sp]!=10 && s[sp]) anew[at++]=s[sp++];
		anew[at++]=0;
		if(at>anewlen) printf("text.dat.new too large\n"),exit(0);
	}
	/* write back files */
	writefile("SCRIPT.SRC.new",script_len,ascr);
	writefile("TEXT.DAT.new",at,anew);
	printf("inserting text done\n");
}

int main(int argc, char **argv) {
	if(argc<2) usage();
	if(!strcmp(argv[1],"e")) extract(argc-2,argv+2);
	else if(!strcmp(argv[1],"p")) pack(argc-2,argv+2);
	else printf("illegal command %s, use e or p\n",argv[1]);
	return 0;
}
