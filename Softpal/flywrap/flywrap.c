/* wordwrap, made specifically for flyable heart
   warn about lines that are longer than textbox width
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// max font size, increase if needed
#define MAX 1000
// max string size
#define LARGE 10000

void usage() {
	puts("usage: flywrap TEXT.DAT text.txt [linelen] [sN=linelen] [etc]");
	puts("linelen: line length in characters for default font size");
	puts("sN=linelen: line length for lines starting with <sN> for an integer N");
	puts("s30=linelen: line length for lines starting with <s30> (example)");
	puts("if no line lengths are specified, good defaults for flyable heart are used");
	exit(0);
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

char *a;

int fontlen[MAX];

int main(int argc,char **argv) {
	if(argc<3) usage();
	for(int i=0;i<MAX;i++) fontlen[i]=-1;
	// good presets for flyable heart
	fontlen[0]=44; // 0: standard font size
	fontlen[18]=48;
	fontlen[26]=35;
	fontlen[28]=33;
	fontlen[30]=31;
	for(int i=3;i<argc;i++) {
		if(argv[i][0]=='s') {
			int a,b;
			sscanf(argv[i],"s%d=%d",&a,&b);
			if(a<0 || a>=MAX) printf("bad font size %d, keep between 1 and %d\n",a,LARGE-1),exit(0);
			fontlen[a]=b;
		} else sscanf(argv[i],"%d",&fontlen[0]);
	}

	unsigned len;
	readfile(argv[1],&len,&a);

	FILE *f=fopen(argv[2],"r");
	if(!f) printf("couldn't open %s\n",argv[2]),exit(0);

	char s[LARGE+1],t[LARGE+1];
	int numlines;
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		sscanf(s,"<%d>",&numlines);
		break;
	}

	int at=16;
	while(fgets(s,LARGE,f)) {
		if(s[0]!='<') continue;
		int lineno,dummy;
		sscanf(s,"<%d,%d>",&lineno,&dummy);
		at+=4;
		// a totally irrelevant part that checks for tags i don't know about
		for(int i=0;a[at+i] && at+i<len;i++) {
			if(a[at+i]=='<' && a[at+i+1]!='c' && a[at+i+1]!='d' && a[at+i+1]!='b' && a[at+i+1]!='r' && a[at+i+1]!='s' && a[at+i+1]!='/') printf("unknown tag %c at line %d\n",a[at+i+1],lineno);
		}

		int k=0;
		while(s[k]!='>') k++;
		k++;
		for(int i=0;s[k+i] && a[at+i];i++) if(s[k+i]!=a[at+i]) goto check;
		while(a[at]) at++;
		at++;
		// line is unchanged from original script, we don't care
		continue;
	check:
		while(a[at]) at++;
		at++;
		int fontsize=0;
		// find the line's fint size (find <sN>)
		for(int i=0;s[i] && i<LARGE;i++) if(s[i]=='<' && s[i+1]=='s') {
			sscanf(s+i+2,"%d",&fontsize);
			if(fontlen[fontsize]<0) printf("error, no line length defined for font size %d\n",fontsize);
			break;
		}
		// strip away all tags except <br>
		int j=0;
		for(int i=0;s[i] && i<LARGE;i++) {
			if(!strncmp(s+i,"<br>",4)) {
				t[j++]=s[i];
				continue;
			}
			if(s[i]=='<') {
				while(s[i] && s[i]!='>') i++;
				continue;
			}
			t[j++]=s[i];
		}
		t[j]=0;
		// warn about lines that has substring such that len(substr) >= fontlen[fontsize]
		int len=0;
		for(int i=0;t[i];i++) {
			if(len>=fontlen[fontsize]) {
				for(int j=0;j<i-1;j++) putchar(t[j]);
				printf("____");
				for(int j=i-1;t[j] && t[j]!=13 && t[j]!=10 && t[j]!='<';j++) putchar(t[j]);
				printf("\nline %d too long\n",lineno);
				goto checked;
			}
			if(!strncmp(t+i,"<br>",4)) {
				i+=3;
				len=-1;
			}
			len++;
		}
	checked:
		;
	}
	puts("linewrap check done");
	return 0;
}
