/* pack individual files into d.o. format archive file */
/* problem: unpacking and repacking ISF results in a file that the game
   doesn't like (can't find w_close.inf). maybe the game cares about
   how the filenames are sorted? */

#include <stdio.h>
#include <stdlib.h>

/* given a directory path, read all file entries
   (including file name, date, other flags).
   this mess is supposed to work on most platforms.
*/
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#endif

typedef long long ll;
typedef unsigned long long ull;

typedef struct {
#ifdef _WIN32
	HANDLE hfind;
	WIN32_FIND_DATA f;
#else
	DIR *d;
	struct dirent *f;
#endif
	int dir;	/* 1: dir, 0: no dir, -1: not supported */
	ull len;
	int nolen;	/* 1 if no support for filelen */
	char *s;
} dir_t;

#ifdef _WIN32
int dirwin(dir_t *h) {
	h->dir=(h->f.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?1:0;
	h->s=h->f.cFileName;
	h->len=h->f.nFileSizeHigh*(1ULL<<32)+h->f.nFileSizeLow;
	h->nolen=0;
	return 1;
}
#else
int dirunix(dir_t *h) {
	h->f=readdir(h->d);
	if(!h->f) return 0;
	h->s=h->f->d_name;
#ifdef _DIRENT_HAVE_D_TYPE
	h->dir=h->f->d_type==DT_DIR;
#else
	h->dir=-1;
#endif
	h->nolen=1;
	return 1;
}
#endif
/* return 1 if ok, 0 if error */
int findfirst(char *s,dir_t *h) {
#ifdef _WIN32
	h->hfind=FindFirstFile(s,&h->f);
	if(INVALID_HANDLE_VALUE==h->hfind) return 0;
	return dirwin(h);
#else
	h->d=opendir(s);
	if(!h->d) return 0;
	return dirunix(h);
#endif
}

/* return 0 if no more files in directory */
int findnext(dir_t *h) {
#ifdef _WIN32
	if(!FindNextFile(h->hfind,&h->f)) return 0;
	return dirwin(h);
#else
	return dirunix(h);
#endif
}

void findclose(dir_t *h) {
#ifdef _WIN32
	FindClose(h->hfind);
#else
	closedir(h->d);
#endif
}

void usage() {
	puts("dopack v1.0 by me in 2021\n");
	puts("usage: dopack destfile\n");
	puts("destfile becomes an archive of all files in current dir");
	puts("(if destfile already exists it will be overwritten)");
	exit(0);
}

unsigned char *buf;

void writeuint32(unsigned char *b,unsigned int x) {
	b[0]=x&255;
	b[1]=(x>>8)&255;
	b[2]=(x>>16)&255;
	b[3]=x>>24;
}

void pack(char *dest) {
	dir_t t;
	unsigned int numfiles=0;
	unsigned long long totlen=0;
	unsigned int resstart;
#ifdef _WIN32
	if(!findfirst("*",&t)) { puts("no files"); exit(1); }
#else
	if(!findfirst(".",&t)) { puts("no files"); exit(1); }
#endif
	do {
		if(t.dir==0) {
			numfiles++;
			if(t.nolen) { puts("file must have file length"); exit(1); }
			if(t.len>=(1ULL<<32)) { puts("individual file can't exceed 4gb"); exit(1); }
			totlen+=t.len;
		}
	} while(findnext(&t));
	findclose(&t);
	printf("found %d files with total filesize %I64d\n",numfiles,totlen);
	if(totlen>=(1ULL<<32)) { puts("total filesize can't be more than 4gb"); exit(1); }

	/* allocate 32 bytes for header, 20*files for dir listing,
	   totlen for all files, 16*(files+1) for overhead */
	buf=calloc(32+totlen+20*numfiles+16*(numfiles+1),1);
	if(!buf) { puts("out of memory"); exit(1); }

	/* make header (first 0x20 bytes) */
	/* first 8 bytes: id */
	sprintf((char *)buf,"SM2MPX10");
	/* write number of files at 0x8 */
	writeuint32(buf+8,numfiles);
	resstart=32+20*numfiles;
	/* write end of dir entries at 0xc */
	writeuint32(buf+0xc,resstart);
	/* write name of archive file at 0x10 (max 12 chars) */
	for(int i=0;i<12 && dest[i];i++) buf[i+0x10]=dest[i];
	/* write offset to dir list at 0x1c */
	writeuint32(buf+0x1c,0x20);

	/* read all files and gradually build archive file in memory */
#ifdef _WIN32
	findfirst("*",&t);
#else
	findfirst(".",&t);
#endif
	unsigned int atdir=0x20;
	/* write contents to 16-aligned start address */
	resstart=(resstart+15)/16*16;
	do {
		if(t.dir==0) {
			/* write filename (max 12 chars) */
			for(int i=0;i<12 && t.s[i];i++) buf[atdir+i]=t.s[i];
			/* write start of file contents to directory entry */
			writeuint32(buf+atdir+0xc,resstart);
			/* write file length to directory entry */
			writeuint32(buf+atdir+0x10,t.len);
			/* read contents of file */
			FILE *f=fopen(t.s,"rb");
			if(!f) { puts("file open for reading error"); exit(1); }
			if(fread(buf+resstart,1,t.len,f)!=t.len) { puts("file read error"); exit(1); }
			if(fclose(f)) { puts("error closing file for reading"); exit(1); }
			/* set start of next file */
			resstart=(resstart+t.len+15)/16*16;
			/* advance dir pointer */
			atdir+=20;
		}
	} while(findnext(&t));

	/* write archive file */
	FILE *f=fopen(dest,"wb");
	if(!f) { puts("file open for writing error"); exit(1); }
	if(fwrite(buf,1,resstart,f)!=resstart) { puts("file write error"); exit(1); }
	if(fclose(f)) { puts("error closing file for writing"); exit(1); }
	free(buf);
}

int main(int argc,char **argv) {
	if(argc<2) usage();
	pack(argv[1]);
}
