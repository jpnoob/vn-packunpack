/* if i can't find mask and offset, but have a bunch of pairs of
   hash and (hash&mask)+offset: use brute force to find mask,offset.
   i guess it might need many pairs in some cases (for example if
   mask has many 1-bits) */

#include <stdio.h>

#define PAIR 5

unsigned pair[PAIR][2]={
{0x7EF5B12D,0x272},
{0x1FA10C6C,0x1b1},
{0xA630664A,0x18d},
{0x0899D2ED,0x1b2},
{0x98609247,0x18a},
};

int main() {
	int poss=0;
	for(unsigned mask=0;mask<0x1000;mask++) {
		for(unsigned offset=0;offset<0x1000;offset++) {
			for(int i=0;i<PAIR;i++) {
				unsigned v=(pair[i][0]&mask)+offset;
				if(v!=pair[i][1]) goto fail;
			}
			if(poss<20) printf("mask 0x%x offset 0x%x\n",mask,offset);
			else if(poss==20) puts("(and more)");
			poss++;
		fail:;
		}
	}
	printf("%d possible (mask,offset) pairs\n",poss);
	return 0;
}
