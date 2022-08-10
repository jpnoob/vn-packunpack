/* generate data structure for generated programs from several incomplete
   dumps of generated programs

   programdir%d.mem: 0x204 bytes with pointers to routines
   (needs entry 0 which points to the start of routine-mem)
   program%d.mem: 0x4000 bytes with routines
   (should include entry 0 since it can hold a program)
*/

#include <stdio.h>
#include <string.h>

unsigned char fil1[1024];
unsigned char fil2[16384];

unsigned char output[16384];

unsigned read4(unsigned char *a,int i) {
	return a[i]+(a[i+1]<<8)+(a[i+2]<<16)+(a[i+3]<<24);
}

/* now that i've cleaned up garbage and dynamically allocated addresses
   in preprocess(), this routine should (hopefully) never output anything */
void process() {
	unsigned base=read4(fil1,0);
	for(int i=0;i<128;i++) {
		unsigned addr=read4(fil1,i*4+4);
		if(addr) {
			int ix=addr-base;
			int iy=i*128;
			if(output[iy]) {
				/* program exists, do a sanity comparison */
				for(int j=0;j<128;j++) {
					if(output[iy+j]!=fil2[ix+j]) printf("difference in different instances of program %d at index %d\n",i,j);
					if(j>=5 && output[ix+j-5]==0x5a && output[ix+j-4]==0x59 && output[ix+j-3]==0x5b && output[ix+j-2]==0x5e && output[ix+j-1]==0x5f && output[ix+j]==0xc3) break;
				}
			} else for(int j=0;j<128;j++) output[iy+j]=fil2[ix+j];
		}
	}
}

/* zero out the address to the control block in the lookups so that the
   sanity comparison doesn't crap out each time that address (which is often
   dynamically allocated) is different
   be xx xx xx xx     mov esi,xxxxxxxx (x is dword-aligned i think)
   8b 86 yy yy 00 00  mov eax,dword ptr ds:[esi+yyyy] (y<0x1000, divisible by 4)
   and
   be xx xx xx xx     mov esi,xxxxxxxx (x is dword-aligned i think)
   25 ff 03 00 00     and eax,3ff
   8b 04 86           mov eax,dword ptr ds:[esi+eax*4]
   hope that this pattern doesn't occur in the middle of other instructions.
   also, zero out remnants of generated faulty old programs after ret
   5a 59 5b 5e 5f c3 (bunch of pops, then ret)
*/
void preprocess() {
	for(int i=0;i<128;i++) {
		for(int j=0;j<128;j++) {
			int p=i*128+j;
			if(j<128-11 && fil2[p]==0xbe && (fil2[p+1]&3)==0 && fil2[p+5]==0x8b && fil2[p+6]==0x86 && (fil2[p+7]&3)==0 && fil2[p+8]<0x10 && fil2[p+9]==0x00 && fil2[p+10]==0x00) {
				fil2[p+1]=fil2[p+2]=fil2[p+3]=fil2[p+4]=0;
			}
			if(j<128-13 && fil2[p]==0xbe && (fil2[p+1]&3)==0 && fil2[p+5]==0x25 && fil2[p+6]==0xff && fil2[p+7]==0x03 && fil2[p+8]==0x00 && fil2[p+9]==0x00 && fil2[p+10]==0x8b && fil2[p+11]==0x04 && fil2[p+12]==0x86) {
				fil2[p+1]=fil2[p+2]=fil2[p+3]=fil2[p+4]=0;
			}
			if(j<128-6 && fil2[p]==0x5a && fil2[p+1]==0x59 && fil2[p+2]==0x5b && fil2[p+3]==0x5e && fil2[p+4]==0x5f && fil2[p+5]==0xc3) {
				j+=6;
				for(;j<128;j++) fil2[i*128+j]=0;
			}
		}
	}
}

int main() {
	memset(output,0,sizeof(output));
	for(int i=1;;i++) {
		char filename[200];
		sprintf(filename,"programdir%d.mem",i);
		FILE *f=fopen(filename,"rb");
		if(!f) {
			printf("processed %d dumps\n",i-1);
			break;
		}
		fread(fil1,1,1024,f);
		fclose(f);
		sprintf(filename,"program%d.mem",i);
		f=fopen(filename,"rb");
		if(!f) {
			puts("error");
			return 1;
		}
		fread(fil2,1,16384,f);
		fclose(f);
		printf("processing dump %d\n",i);
		preprocess();
		process();
	}
	int missing=0;
	for(int i=0;i<128;i++) if(!output[i*128]) {
		missing=1;
		printf("missing program %d\n",i);
	}
	if(missing) return 0;
	printf("[");
  for(int i=0;i<128;i++) {
		unsigned addr=i*128;
		printf("[");
		for(int j=0;j<128;j++) {
			printf("%d",output[addr+j]);
			if(j>=5 && output[addr+j-5]==0x5a && output[addr+j-4]==0x59 && output[addr+j-3]==0x5b && output[addr+j-2]==0x5e && output[addr+j-1]==0x5f && output[addr+j]==0xc3) break;
			printf(",");
		}
		printf("]");
		if(i<127) printf(",");
	}
	printf("]\n");
	return 0;
}
