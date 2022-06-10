#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char *file[2];

int getlen(char *s) {
	FILE *f=fopen(s,"rb");
	if(!f) { printf("can't open file\n"); exit(1); }
	fseek(f,0,SEEK_END);
	int len=ftell(f);
	fclose(f);
	return len;
}

void loadfile(int ix,char *s,int len) {
	FILE *f=fopen(s,"rb");
	if(!f) { printf("can't open file\n"); exit(1); }
	fread(file[ix],1,len,f);
	fclose(f);
}

int main(int argc,char **argv) {
	if(argc<3) {
		printf("usage: xor file1 file2 [outfile]\n");
		printf("xor 2 files (of equal length), output to outfile or stdout\n");
	}
	int len1=getlen(argv[1]);
	int len2=getlen(argv[2]);
	if(len1!=len2) {
		printf("input files must have same length\n");
		return 0;
	}
	if(!(file[0]=malloc(len1))) { printf("out of memory\n"); return 0; }
	if(!(file[1]=malloc(len2))) { printf("out of memory\n"); return 0; }
	loadfile(0,argv[1],len1);
	loadfile(1,argv[2],len2);
	for(int i=0;i<len1;i++) file[1][i]^=file[0][i];
	char freq[256];
	for(int i=0;i<256;i++) freq[i]=0;
	for(int i=0;i<len1;i++) freq[file[1][i]]=1;
	int sum=0;
	for(int i=0;i<256;i++) sum+=freq[i];
	if(sum==1) printf("every byte xor'd with %02x\n",file[1][0]);
	else printf("%d different xor values\n",sum);
	if(argc==4) {
		FILE *f=fopen(argv[3],"wb");
		if(!f) { printf("can't save file\n"); exit(1); }
		fwrite(file[1],1,len1,f);
		fclose(f);
	} else {
		for(int i=0;i<len1;i++) printf("%02x ",file[1][i]);
	}
	return 0;
}
