/* convert all normal spaces (0x20) to japanese space (0x81 0x40)
   musica engine doesn't want to display normal space
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage() {
	puts("spaceconv v1.0 by me in 2021");
	puts("usage: spaceconv file");
	exit(0);
}

void error(char *s) {
	puts(s);
	exit(1);
}

char *s;
char *t;

int main(int argc, char **argv) {
	if(argc!=2) usage();
	FILE *f=fopen(argv[1],"rb");
	if(!f) error("Couldn't open input file");
	fseek(f,0,SEEK_END);
	size_t filelen=ftell(f);
	fseek(f,0,SEEK_SET);
	s=malloc(filelen);
	if(!s) error("out of memory");
	/* dumb overestimate, 2x of input file size */
	t=malloc(filelen*2);
	if(!t) error("out of memory");
	if(filelen!=fread(s,1,filelen,f)) error("couldn't read the entire file");
	if(fclose(f)) error("error closing file after reading\n");

	/* find all occurrences of space on lines beginning with ".message" */
	int sp=0,tp=0;
	int startofline=1;
	int ismessageline=0;
	int count=0;
	while(sp<filelen) {
		if(startofline && !strncmp(".message",s+sp,8)) ismessageline=1,startofline=0;
		else if(s[sp]=='\n' || s[sp]=='\r') ismessageline=0,startofline=1;
		else startofline=0;
		if(ismessageline && s[sp]==' ') t[tp++]=0x81,t[tp++]=0x40,sp++,count++;
		else t[tp++]=s[sp++];
	}
	t[tp]=0;

	/* output file, overwrite input file */
	f=fopen(argv[1],"wb");
	if(!f) error("Couldn't open file for writing");
	if(tp!=fwrite(t,1,tp,f)) error("couldn't write the entire file");
	if(fclose(f)) error("error closing file after writing\n");
	printf("done, converted %d spaces\n",count);
	return 0;
}
