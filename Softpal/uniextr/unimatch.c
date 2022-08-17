#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unimatch.h"

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

// syscall can be a bunch of values
int istextline(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+8)==0x0001001f && getint4(a,at+16)==0x0001001f && getint4(a,at+24)==0x00010017 && (getint4(a,at+28)==0x00020002 || getint4(a,at+28)==0x0002000f || getint4(a,at+28)==0x00020010 || getint4(a,at+28)==0x00020011 || getint4(a,at+28)==0x00020012 || getint4(a,at+28)==0x00020013 || getint4(a,at+28)==0x00020014) && getint4(a,at+32)==0;
}

// text line from unity princess (slightly different format)
int isunitymarriagetextline(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x30000000 && getint4(a,at+12)==0x00010001 && getint4(a,at+16)==0x30000001 && getint4(a,at+24)==0x00010001 && getint4(a,at+28)==0x30000002 && getint4(a,at+36)==0x00010008 && getint4(a,at+40)==0x40000000 && getint4(a,at+44)==0x40000000 && getint4(a,at+48)==0x00010001 && getint4(a,at+52)==0x40000000 && getint4(a,at+60)==0x00010001 && getint4(a,at+64)==0x30000003 && getint4(a,at+68)==0x40000000 && getint4(a,at+72)==0x00010017 && (getint4(a,at+76)==0x00020002 || getint4(a,at+76)==0x0002000f || getint4(a,at+76)==0x00020011 || getint4(a,at+76)==0x00020012 || getint4(a,at+76)==0x00020013);
}

// text line from unity princess that appears gradually
int isunitymarriagegradualtext(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x30000000 && getint4(a,at+12)==0x00010001 && getint4(a,at+16)==0x30000001 && getint4(a,at+24)==0x00010001 && getint4(a,at+28)==0x30000002 && getint4(a,at+36)==0x00010017 && getint4(a,at+40)==0x00020014;
}

// warning, pattern can occur without strptr
int isnewchapterorscene(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)!=0x40000000 && getint4(a,at+8)==0x00010017 && getint4(a,at+12)==0x000f0002 && getint4(a,at+16)==0;
}

// a special case from akatsuki yureru trial
int isakatsukispecialtextline(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+8)==0x0001001f && getint4(a,at+16)==0x0001001f && getint4(a,at+24)==0x00010017 && getint4(a,at+28)==0x00030016 && getint4(a,at+32)==0;
}

// choice
int ischoice(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+8)==0x0001001f && getint4(a,at+16)==0x0001001f && getint4(a,at+24)==0x00010017 && getint4(a,at+28)==0x00060002 && (getint4(a,at+32)==0 || getint4(a,at+32)==0x00000001);
}

// a special case from flyable heart with the strptr in a different spot
// it's pretty similar to flyable candyheart, maybe merge them
int isflyableheartspecialtextline(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+8)==0x00010009 && getint4(a,at+16)==0x0001001f && getint4(a,at+24)==0x00010017 && getint4(a,at+28)==0x00060002 && getint4(a,at+32)==0;
}

// matches selection
// warning, this pattern can occur without strptr
int isselection(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)!=0x40000000 && getint4(a,at+8)==0x00010017 && getint4(a,at+12)==0x000a0015 && getint4(a,at+16)==0;
}

// matches roll and eyecatch commands (and maybe more)
int isrolleyecatch(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x40000000 && getint4(a,at+12)==0x0001001f && getint4(a,at+16)==0x40000000 && getint4(a,at+20)==0x0001001f && getint4(a,at+28)==0x00010008 && getint4(a,at+32)==0x40000000 && getint4(a,at+36)==0x40000000 && getint4(a,at+40)==0x00010001 && getint4(a,at+44)==0x40000000 && (getint4(a,at+48)==0x50800000 || getint4(a,at+48)==0x50810000) && getint4(a,at+52)==0x0001001f && getint4(a,at+56)==0x40000000 && getint4(a,at+60)==0x00010017 && getint4(a,at+64)==0x00120009 && getint4(a,at+68)==0;
}

// stuff that one normally wouldn't change, but whose pointers must be changed if i'm doing
// the current approach where i change every pointer after the first changed line

int isunitymarriagemysterioustext(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x30000000 && getint4(a,at+12)==0x00010009 && getint4(a,at+20)==0x00010008 && getint4(a,at+24)==0x40000000 && getint4(a,at+28)==0x40000000
 && getint4(a,at+32)==0x00010001 && getint4(a,at+36)==0x40000000 && getint4(a,at+40)==0 && getint4(a,at+44)==0x00010001 && getint4(a,at+48)==0x30000001 && getint4(a,at+52)==0x40000000 && getint4(a,at+56)==0x00010017 && getint4(a,at+60)==0x00060002;
}

// matches the two staff roll commands at the very end of the script in akatsuki yureru trial
int isakatsukistaffroll(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x00010001 && getint4(a,at+12)==0x40000000 && getint4(a,at+16)==0x80000001 && getint4(a,at+20)==0x0001001f && getint4(a,at+24)==0x40000000 && getint4(a,at+28)==0x0001001f && getint4(a,at+36)==0x00010017 && getint4(a,at+40)==0x0012005c && getint4(a,at+44)==0;
}

// matches the two staff roll commands at the very end of the script in natsuiro kokoro log
int isrorologstaffroll(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x00010001 && getint4(a,at+12)==0x40000000 && getint4(a,at+16)==0x80000001 && getint4(a,at+20)==0x0001001f && getint4(a,at+24)==0x40000000 && getint4(a,at+28)==0x0001001f && getint4(a,at+36)==0x00010017 && getint4(a,at+40)==0x000f0004 && getint4(a,at+44)==0;
}

// matches the 3 non-script things at the end of flyable heart trial
int isflyabletrialendstuff(char *a,int at) {
	return getint4(a,at)==0x00010017 && getint4(a,at+4)==0x0012000b && getint4(a,at+8)==0 && getint4(a,at+12)==0x0001000a && getint4(a,at+20)==0x40000000 && getint4(a,at+24)==0x0001001f && getint4(a,at+32)==0x00010017 && getint4(a,at+36)==0x000f0001 && getint4(a,at+40)==0;	
}

// matches the backslash at the end of kizuna kirameku koi iroha -tsubaki renka-
int iskizunafandiscbackslash(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x40000000 && getint4(a,at+8)==0x00010017 && getint4(a,at+12)==0x00120032 && getint4(a,at+16)==0 && getint4(a,at+20)==0x0001001f && getint4(a,at+28)==0x00010008 && getint4(a,at+32)==0x40000000 && getint4(a,at+36)==0x40000000 && getint4(a,at+40)==0x00010001 && getint4(a,at+44)==0x40000000 && getint4(a,at+48)==0x50800000 && getint4(a,at+52)==0x0001001f && getint4(a,at+56)==0x40000000 && getint4(a,at+60)==0x00010017 && getint4(a,at+64)==0x0012003b && getint4(a,at+68)==0;
}

// matches play command at the end of koi suru kokoro to mahou no kotoba
int isplaykoisurukokoro(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x00010001 && getint4(a,at+12)==0x40000000 && getint4(a,at+16)==0x80000001 && getint4(a,at+20)==0x0001001f && getint4(a,at+24)==0x40000000 && getint4(a,at+28)==0x0001001f && getint4(a,at+36)==0x00010017 && getint4(a,at+40)==0x000f0004 && getint4(a,at+44)==0 && getint4(a,at+48)==0x0001001f && getint4(a,at+52)==0x00000001 && getint4(a,at+56)==0x0001001f && getint4(a,at+60)==0x000005dc && getint4(a,at+64)==0x00010017 && getint4(a,at+68)==0x00070000 && getint4(a,at+72)==0;
}

// matches proper names near the end of koi x shin ai kanojo
int ispropernamekoishin(char *a,int at) {
	return getint4(a,at)==0x00010009 && getint4(a,at+4)==0x00000fe6 && getint4(a,at+8)==0x00010001 && getint4(a,at+12)==0x40000001 && getint4(a,at+20)==0x0001001f && getint4(a,at+24)==0x40000001 && getint4(a,at+28)==0x0001000b && getint4(a,at+32)==0x00000fe1;
}

// matches the choice summary thing in a clockwork leyline 1
// warning, pattern can occur without string pointer
int isleylinethingbeforechoice(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)!=0x40000000 && getint4(a,at+8)==0x00010017 && getint4(a,at+12)==0x000f0000 && getint4(a,at+16)==0;
}

// increase flag in leyline
int isincreaseflag(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x40000000 && getint4(a,at+8)==0x0001001f && getint4(a,at+16)==0x00010017 && getint4(a,at+20)==0x000f0004 && getint4(a,at+24)==0;
}

// website in leyline
int isleylinewebsite(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x0001001f && getint4(a,at+12)==0x0fffffff && getint4(a,at+16)==0x0001001f && getint4(a,at+20)==0x0fffffff && getint4(a,at+24)==0x0001001f && getint4(a,at+28)==0x0fffffff && getint4(a,at+32)==0x0001001f && getint4(a,at+36)==0x0fffffff && getint4(a,at+40)==0x0001001f && getint4(a,at+44)==0x0fffffff && getint4(a,at+48)==0x0001001f && getint4(a,at+52)==0x0fffffff && getint4(a,at+56)==0x0001001f && getint4(a,at+60)==0x0fffffff && getint4(a,at+64)==0x0001001f && getint4(a,at+72)==0x00010008 && getint4(a,at+76)==0x40000000 && getint4(a,at+80)==0x40000000 && getint4(a,at+84)==0x00010001 && getint4(a,at+88)==0x40000000 && getint4(a,at+92)==0x50800000 && getint4(a,at+96)==0x0001001f && getint4(a,at+100)==0x40000000 && getint4(a,at+104)==0x00010017 && getint4(a,at+108)==0x00120009 && getint4(a,at+112)==0;
}

// koi suru kokoro trial is particularly bad with unique matches
// save folder, "%d %d", various stuff in koi suru kokoro trial
int iskoisurukokorotrialsavefolder(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x40000000 && (getint4(a,at+8)==0x50000000 || getint4(a,at+8)==0x50800000 || getint4(a,at+8)==0x40000003) && getint4(a,at+12)==0x0001001f && getint4(a,at+16)==0x40000000 && getint4(a,at+20)==0x0001001f && getint4(a,at+28)==0x00010017 && getint4(a,at+32)==0x0012005a && getint4(a,at+36)==0;
}

// **debug in koi suru kokoro trial
int iskoisurukokorotrialdebug(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x0001001f && getint4(a,at+12)==0x0fffffff && getint4(a,at+16)==0x0001001f && getint4(a,at+20)==0x0fffffff && getint4(a,at+24)==0x0001001f && getint4(a,at+32)==0x00010017 && getint4(a,at+36)==0x0012005a && getint4(a,at+40)==0;
}

// "1,2,3,4" and "rarara" (in hiragana) in koi suru kokoro trial
int iskoisurukokorotrial1234rarara(char *a,int at) {
	return getint4(a,at)==0x00010001 && getint4(a,at+4)==0x40000002 && getint4(a,at+12)==0x00010001 && getint4(a,at+16)==0x40000000 && getint4(a,at+20)==0x40000002 && getint4(a,at+24)==0x0001001f && getint4(a,at+28)==0x40000000 && getint4(a,at+32)==0x00010017 && getint4(a,at+36)==0x00120073 && getint4(a,at+40)==0;
}

// match "press the button" in koi suru kokoro trial
int iskosurukokorotrialpressthebutton(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+4)==0x0fffffff && getint4(a,at+8)==0x0001001f && getint4(a,at+12)==0x0fffffff && getint4(a,at+16)==0x0001001f && getint4(a,at+20)==0x0fffffff && getint4(a,at+24)==0x0001001f && getint4(a,at+28)==0x0fffffff && getint4(a,at+32)==0x0001001f && getint4(a,at+36)==0x0fffffff && getint4(a,at+40)==0x0001001f && getint4(a,at+44)==0x0fffffff && getint4(a,at+48)==0x0001001f && getint4(a,at+52)==0x0fffffff && getint4(a,at+56)==0x0001001f && getint4(a,at+64)==0x00010017 && getint4(a,at+68)==0x000f0004 && getint4(a,at+72)==0;
}

// match music playlist in koi suru kokoro trial
int iskosurukokorotrialmusicplaylist(char *a,int at) {
	return getint4(a,at)==0x0001001f && getint4(a,at+8)==0x00010008 && getint4(a,at+12)==0x40000000 && getint4(a,at+16)==0x40000000 && getint4(a,at+20)==0x00010001 && getint4(a,at+24)==0x40000000 && getint4(a,at+28)==0x50800000 && getint4(a,at+32)==0x0001001f && getint4(a,at+36)==0x40000000 && getint4(a,at+40)==0x00010017 && getint4(a,at+44)==0x0012003b && getint4(a,at+48)==0;
}

void process_script(unsigned scrlen,char *ascr) {
	int at=16;
	while(at<scrlen) {
		if(istextline(ascr,at)) {
			// text line (mandatory)
			process(ascr,at+4);
			// speaker (optional)
			if(getint4(ascr,at+12)!=0x0fffffff) process(ascr,at+12);
			at+=36;
		} else if(isunitymarriagetextline(ascr,at)) {
			// speaker (optional)
			if(getint4(ascr,at+20)!=0x0fffffff) process(ascr,at+20);
			// text line (mandatory)
			process(ascr,at+32);
			at+=80;
		} else if(isunitymarriagegradualtext(ascr,at)) {
			// speaker (optional)
			if(getint4(ascr,at+20)!=0x0fffffff) process(ascr,at+20);
			// text line (mandatory)
			process(ascr,at+32);
			at+=44;
		} else if(isnewchapterorscene(ascr,at)) {
			// new chapter thing
			process(ascr,at+4);
			at+=20;
		} else if(isakatsukispecialtextline(ascr,at)) {
			process(ascr,at+12);
			at+=36;
		} else if(ischoice(ascr,at)) {
			process(ascr,at+20);
			at+=28;
		} else if(isflyableheartspecialtextline(ascr,at)) {
			process(ascr,at+20);
			at+=36;
		} else if(isselection(ascr,at)) {
			// "character selection", "trial version selection" and so on
			// not to be confused with normal choices
			process(ascr,at+4);
			at+=28;
		} else if(isrolleyecatch(ascr,at)) {
			// matches roll and eyecatch commands from some games
			process(ascr,at+24);
			at+=72;
// special stuff, non-script lines that translators shouldn't change (i guess)
		} else if(isakatsukistaffroll(ascr,at)) {
			// staff roll commands from akatsuki yureru trial
			process(ascr,at+32);
			at+=48;
		} else if(isrorologstaffroll(ascr,at)) {
			// staff roll commands from akatsuki yureru trial
			process(ascr,at+32);
			at+=48;
		} else if(iskizunafandiscbackslash(ascr,at)) {
			// more roll commands from kizuna kirameku koi iroha
			process(ascr,at+24);
			at+=72;
		} else if(isflyabletrialendstuff(ascr,at)) {
			// staff roll commands from akatsuki yureru trial
			process(ascr,at+28);
			at+=44;
		} else if(isplaykoisurukokoro(ascr,at)) {
			// "character selection" in japanese from kizuna trial
			process(ascr,at+32);
			at+=76;
		} else if(ispropernamekoishin(ascr,at)) {
			// proper name in koi x shin ai konojo
			process(ascr,at+16);
			at+=36;
		} else if(isleylinethingbeforechoice(ascr,at)) {
			process(ascr,at+4);
			at+=20;
		} else if(isincreaseflag(ascr,at)) {
			process(ascr,at+12);
			at+=28;
		} else if(isleylinewebsite(ascr,at)) {
			process(ascr,at+68);
			at+=116;
		} else if(iskoisurukokorotrialsavefolder(ascr,at)) {
			process(ascr,at+24);
			at+=40;
		} else if(iskoisurukokorotrialdebug(ascr,at)) {
			process(ascr,at+28);
			at+=44;
		} else if(iskoisurukokorotrial1234rarara(ascr,at)) {
			process(ascr,at+8);
			at+=44;
		} else if(iskosurukokorotrialpressthebutton(ascr,at)) {
			// press the button in 
			process(ascr,at+60);
			at+=76;
		} else if(iskosurukokorotrialmusicplaylist(ascr,at)) {
			process(ascr,at+4);
			at+=52;
		} else if(isunitymarriagemysterioustext(ascr,at)) {
			process(ascr,at+8);
			at+=64;
		} else at+=4;
	}
}
