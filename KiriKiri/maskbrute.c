/* if i can't find mask and offset, but have a bunch of pairs of
   hash and (hash&mask)+offset: use brute force to find mask,offset.
   i guess it might need many pairs in some cases (for example if
   mask has many 1-bits) */

#include <stdio.h>

#define PAIR 6

unsigned pair[PAIR][2]={
{0xA92E7B90u, 0x000006BBu},
{0xA67F9DA6u, 0x000004BFu},
{0xD59976D5u, 0x000006BFu},
{0xCC7B55B7u, 0x000004BFu},
{0x7A2A899Bu, 0x000004BBu},
{0xB2956026u, 0x000004BFu},
};

int main() {
	int poss=0;
	int mask2=-1,offs2=-1;
	for(int mask=0;mask<0x1000;mask++) {
		for(int offset=0;offset<0x1000;offset++) {
			for(int i=0;i<PAIR;i++) {
				unsigned v=(pair[i][0]&mask)+offset;
				if(v!=pair[i][1]) goto fail;
			}
			mask2=mask; offs2=offset;
			poss++;
		fail:;
		}
	}
	printf("%d possible (mask,offset) pairs\n",poss);
	printf("mask %d offset %d\n",mask2,offs2);
	return 0;
}
