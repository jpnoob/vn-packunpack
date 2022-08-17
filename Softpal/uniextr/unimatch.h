#ifndef _UNIMATCH_H
#define _UNIMATCH_H

unsigned getint4(char *,int);
void writeint4(char *,int,unsigned);
void readfile(char *,unsigned *,char **);

int istextline(char *,int);
int isunitymarriagetextline(char *,int);
int isunitymarriagegradualtext(char *,int);
int isnewchapterorscene(char *,int);
int isakatsukispecialtextline(char *,int);
int ischoice(char *,int);
int isflyableheartspecialtextline(char *,int);
int isselection(char *,int);
int isrolleyecatch(char *,int);
int isunitymarriagemysterioustext(char *,int);
int isakatsukistaffroll(char *,int);
int isrorologstaffroll(char *,int);
int isflyabletrialendstuff(char *,int);
int iskizunafandiscbackslash(char *,int);
int isplaykoisurukokoro(char *,int);
int ispropernamekoishin(char *,int);
int isleylinethingbeforechoice(char *,int);
int isincreaseflag(char *,int);
int isleylinewebsite(char *,int);
int iskoisurukokorotrialsavefolder(char *,int);
int iskoisurukokorotrialdebug(char *,int);
int iskoisurukokorotrial1234rarara(char *,int);
int iskosurukokorotrialpressthebutton(char *,int);
int iskosurukokorotrialmusicplaylist(char *,int);

void process(char *,int);
void process_script(unsigned,char *);

#endif /* _UNIMATCH_H */
