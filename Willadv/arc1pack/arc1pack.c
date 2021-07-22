/* pack old arc format
   works on Anata no Koto o Suki to Iwasete trial version
   bleh, i should probably write stuff like this in python
*/

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
/* end of hideous directory code */

void usage() {
	puts("arc1pack by me in 2021\n");
	puts("usage: arc1pack destfile\n");
	puts("packs all files in current directory");
	puts("if destfile already exists it will be overwritten");
	puts("destination file is allowed to be in current directory, it won't be included");
	exit(0);
}

void error(char *s) {
	puts(s);
	exit(1);
}

unsigned char *buf;

void writeuint32(unsigned char *b,unsigned int x) {
	b[0]=x&255;
	b[1]=(x>>8)&255;
	b[2]=(x>>16)&255;
	b[3]=x>>24;
}

unsigned int readuint32(unsigned char *b) {
	return b[0]+(b[1]<<8)+(b[2]<<16)+(b[3]<<24);
}

int conv(int c) {
//	if(c=='.') return 0;
//	if(c=='_') return 1;
	return c;
}

int compdir(const void *A,const void *B) {
	const char *a=A,*b=B;
	for(int i=0;i<12;i++) {
		int aa=conv(*(a+i));
		int bb=conv(*(b+i));
		if(aa<bb) return -1;
		if(aa>bb) return 1;
	}
	return 0;
}

/* unique file extensions seen */
char ext[1000][4];
int extnum;

void addextension(char *s) {
	char ex[10];
	int i,j;
	for(i=0;s[i];i++) if(s[i]=='.') break;
	i++;
	for(j=0;j<3;j++,i++) {
		if(!s[i]) printf("file %s: ",s),error("file extension must exist and have 3 chars");
		ex[j]=s[i];
	}
	if(s[i]) printf("file %s: ",s),error("file extension must exist and have 3 chars");
	ex[3]=0;
	for(i=0;i<extnum;i++) if(!strcmp(ex,ext[i])) return;
	strcpy(ext[extnum++],ex);
}

/* check if extension in filename s matches e (extension) */
int equalext(char *s,char *e) {
	while(*s && *s!='.') s++;
	if(*s) s++;
	while(*e && *s) {
		if(toupper(*e)!=toupper(*s)) return 0;
		e++;
		s++;
	}
	return 1;
}

void pack(char *dest) {
	dir_t t;
	unsigned int numfiles=0;
	unsigned long long totlen=0;
	extnum=0;
	/* find total file length */
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
			addextension(t.s);
		}
	} while(findnext(&t));
	findclose(&t);
	printf("found %d files with total filesize %lld\n",numfiles,totlen);
	if(totlen>=(1ULL<<32)) { puts("total filesize can't be more than 4gb"); exit(1); }
	if(!numfiles) error("there must be at least one file");

	/* header: 4 bytes
	   extnum*12 bytes for file extension headers
	   numfiles*21 bytes for directory listings
	   plus total file length */
	buf=calloc(totlen+4+12*extnum+21*numfiles+totlen,1);
	if(!buf) error("out of memory");

	/* make header (number of extensions) */
	writeuint32(buf,extnum);
	/* start of actual files */
	unsigned int resstart=4+12*extnum+21*numfiles;
	/* start of directory */
	unsigned int atdir=4+12*extnum;

	/* for each extension, do stuff */
	for(int i=0;i<extnum;i++) {
		unsigned int extstart=4+i*12;
		unsigned int numfileswithext=0;
		for(int j=0;j<4;j++) buf[extstart+j]=toupper(ext[i][j]);
		writeuint32(buf+extstart+8,atdir);
		/* find all files with current extension */
#ifdef _WIN32
		findfirst("*",&t);
#else
		findfirst(".",&t);
#endif
		do {
			printf("%s %s\n",t.s,ext[i]);
			if(t.dir==0 && equalext(t.s,ext[i])) {
				printf("read file %s\n",t.s);
				/* write filename (max 12 chars) */
				for(int j=0;j<12 && t.s[j]!='.' && t.s[j];j++) buf[atdir+j]=toupper(t.s[j]);
				writeuint32(buf+atdir+0x0d,t.len);
				writeuint32(buf+atdir+0x11,resstart);
				/* read file and build archive file in memory */
				FILE *f=fopen(t.s,"rb");
				if(!f) error("error while opening file for reading");
				if(fread(buf+resstart,1,t.len,f)!=t.len) error("file read error");
				if(fclose(f)) error("error closing file for reading");
				/* cyclic shift 2 bits left */
				for(int j=0;j<t.len;j++) buf[resstart+j]=(buf[resstart+j]<<2)|(buf[resstart+j]>>6);
				/* advance stuff */
				resstart+=t.len;
				atdir+=21;
				numfileswithext++;
			}
		} while(findnext(&t));
		findclose(&t);
		/* number of files with this extension */
		writeuint32(buf+extstart+4,numfileswithext);
	}

	/* write archive file */
	FILE *f=fopen(dest,"wb");
	if(!f) error("file open for writing error");
	if(fwrite(buf,1,resstart,f)!=resstart) error("file write error");
	if(fclose(f)) error("error closing file for writing");
	free(buf);
}

int main(int argc,char **argv) {
	if(argc!=2) usage();
	pack(argv[1]);
	return 0;
}
