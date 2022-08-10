/* pack softpal adv system format
   based on:
   https://github.com/Yggdrasill-Moe/Niflheim/tree/master/SOFTPAL_ADV_SYSTEM/fuckpac
   and garbro
   with the following functionality:
   - support for flyable heart (old format) and new format (signature "PAC ")
   - can update existing archive with only a few files

   doesn't support extracting from archives (use garbro or fuckpac), or creating new
   archives from scratch, or adding (not replacing) a new file to an existing archive

   in other words, the set of files inside an archive never changes (just the contents)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LARGE 1000

void usage() {
	puts("usage: unipack pac-file directory-with-files-to-pack");
	puts("example: unipack data.pac data\\");
	puts("writes to {outfile}.new, so data.pac => data.pac.new");
	exit(0);
}

char *a;		// infile (.pac)
char *anew;	// outfile (.pac)

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

unsigned char ror(int val,int n) {
	n&=7;
	return (val >> n) | (val << (8 - n));
}

void encrypt(char *a,int len) {
	int count=(len-0x10)/4;
	unsigned char *p=(unsigned char *)a+0x10;
	unsigned key1=0x084DF873;
	unsigned key2=0xFF987DEE;
	unsigned c=0x04;
	for(int i=0;i<count;i++) {
		unsigned *dp=(unsigned *)p;
		*dp^=key1^key2;
		p=(unsigned char*)dp;
		*p=ror(*p,c++);
		c&=0xff;
		p+=4;
	}
}

int main(int argc,char **argv) {
	if(argc<3) usage();
	if(strlen(argv[2])>LARGE-40) printf("directory too long\n"),exit(0);
	unsigned alen,alen2;
	readfile(argv[1],&alen,&a);
	alen2=alen+10000000; // magic constant, increase if we run out of memory
	anew=malloc(alen2);
	if(!anew) printf("out of memory\n"),exit(0);
	unsigned filesaddr,diraddr;
	if(a[0]=='P' && a[1]=='A' && a[2]=='C' && a[3]==' ') {
		filesaddr=0x8;
		diraddr=0x804;
	} else {
		filesaddr=0;
		diraddr=0x3fe;
	}
	int numfiles=getint4(a,filesaddr);
	unsigned at=diraddr+0x28*numfiles;
	// i have no idea what the stuff before the directory entries is, just copy it verbatim
	// it looks like a number that gradually increases towards the number of files
	memcpy(anew,a,at);

	char s[LARGE];
	// loop over each file in archive directory, try to read file, update archive if it exists
	// at is the address where the next file is to be written
	for(int fileno=0;fileno<numfiles;fileno++) {
		char *curfile;
		unsigned curlen;
		// check if file exists
		strcpy(s,argv[2]);
		if(s[strlen(s)-1]!='\\') strcat(s,"\\");
		strcat(s,a+diraddr);
		FILE *f=fopen(s,"rb");
		if(!f) {
			// file doesn't exist, keep file in old .pac file
			memcpy(anew+at,a+getint4(a,diraddr+0x24),getint4(a,diraddr+0x20));
			at+=getint4(a,diraddr+0x20);
		} else {
			// file exists, read it and add to new .pac file
			printf("update archive with %s\n",a+diraddr);
			fclose(f);
			readfile(s,&curlen,&curfile);
			// encrypt
			if(curfile[0]=='$') encrypt(curfile,curlen);
			memcpy(anew+at,curfile,curlen);
			writeint4(anew,diraddr+0x20,curlen);
			writeint4(anew,diraddr+0x24,at);
			at+=curlen;
			free(curfile);
		}
		diraddr+=0x28;
	}
	if(strlen(argv[1])>LARGE-5) printf("archive filename too long\n"),exit(0);
	strcpy(s,argv[1]);
	strcat(s,".new");
	writefile(s,at,anew);
	puts("archive updated");
	return 0;
}
