/* test of copystr */
#include <stdio.h>
#include <std.h>
#include <dos.h>

/* store chars and attribs separately */
unsigned char asc[2048];
unsigned char attr[2048];
unsigned int scrn[2048];		/* also store together */
char far *dbufr;
union REGS inregs,outregs;
unsigned int far *sptr = 720896;

main()
	{
	int i,j;
	unsigned char *sa;		/*attributes ptr*/
	unsigned char *sc;		/*character pointer */
	unsigned int *buf;
	unsigned char page;

/*
**	buf = scrn;
**	sa = attr;
**	sc = asc;
**	inregs.h.ah = 0xf;
**	int86(0x10,&inregs,&outregs);
**	page = outregs.h.bh;
**	for (i = 0;i<25;i++)
**		for (j = 0; j < 80;j++)
**			{
**			inregs.h.ah = 0x02;
**			inregs.h.dh = i;
**			inregs.h.dl = j;
**			inregs.h.bh = page;
**			int86(0x10,&inregs,&outregs);	
**			inregs.h.ah = 0x08;
**			int86(0x10,&inregs,&outregs);
**			*buf++ = outregs.x.ax;
**			*sc++ = outregs.h.al;
**			*sa++ = outregs.h.ah;
**			}
*/
	memcpy(scrn,sptr,4096);
	printf("\n\n\n Hit any key when Ready\n\n\n");
	while (!kbhit());
	memcpy(sptr,scrn,4096);

/*	sa = attr;
**	sc = asc;
**	for (i = 0;i<25;i++)
**		for (j = 0; j < 80;j++)
**			{
**			inregs.h.ah = 0x02;
**			inregs.h.dh = i;
**			inregs.h.dl = j;
**			inregs.h.bh = page;
**			int86(0x10,&inregs,&outregs);	
**			inregs.h.ah = 0x09;
**			inregs.x.cx = 1;
**			inregs.h.al = *sc++;
**			inregs.h.bl = *sa++;
**			inregs.h.bh = page;
**			int86(0x10,&inregs,&outregs);
**			}
*/
	}




