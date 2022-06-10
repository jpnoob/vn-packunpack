/* if i can't find mask and offset, but have a bunch of pairs of
   hash and (hash&mask)+offset: use brute force to find mask,offset.
   i guess it might need many pairs in some cases (for example if
   mask has many 1-bits) */

#include <stdio.h>

#define PAIR 7

unsigned pair[PAIR][2]={
{0xD853B109,0x167},
{0x748E7362,0x3a7},
{0x8125C6A2,0x367},
{0xF19AF2AA,0x367},
{0x797647C2,0x3a7},
{0xDFA7B3CC,0x3ab},
{0x5373A963,0x1a7},
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
