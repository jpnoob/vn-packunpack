/* test different algorithms against given adler values */
#include <stdio.h>

unsigned adler[]={
	0x176a71bf, 0x26a910ef, 0x9dd4c807, 0x330c6a8b, 0xaeffade1, 0
};

void crypt(unsigned hash) {
	/* hashcrypt */
	unsigned char key=hash&255;
	printf(" %02x",key);

	/* natsupochicrypt */
	key=(hash>>3)&255;
	printf(" %02x",key);

	/* poringsoftcrypt */
	key=(~(hash+1))&255;
	printf(" %02x",key);

	/* sourirecrypt */
	key=(hash^0xcd)&255;
	printf(" %02x",key);

	/* haikuocrypt */
	key=(hash^(hash>>8))&255;
	printf(" %02x",key);

	/* yuzucrypt */
	unsigned h=hash^0x1DDB6E7A;
	key=(h^(h>>8)^(h>>16)^(h>>24))&255;
	printf(" %02x",key);

	/* festivalcrypt */
	key=((hash>>7)^0xff)&255;
	printf(" %02x",key);

	/* hybridcrypt */
	key=(hash>>5)&255;
	printf(" %02x",key);

	printf("\n");
}

int main() {
	for(int i=0;adler[i];i++) {
		printf("%08x =>",adler[i]);
		crypt(adler[i]);
	}
	return 0;
}
