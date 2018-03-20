/*********************************************************************\
**
** prherc.c -- TSR to print hercules graphics screen
**
**		This leaves the machine in graphics mode (because _getpixel 
**		won't work otherwise.)  To get it out, issue the command
**
**			PRHERC X	(or any other command-line argument)
**
**		That will make it possible to run rd7.
**
\*********************************************************************/

#include <graph.h>
#include <dos.h>
#include <bios.h>

#define MAXLEN 1024
#define PRTSCR 0x05		/* print screen interrupt */

extern char end;

unsigned memsize = 2000;	/* allocated memory size in paragraphs */
						/* === don't know how to compute this === */

unsigned far printer = 0;	/* printer number (LPT1) */

/*
** Buffer for one printer line of graphics (8 rows)
*/
char far linebuf[MAXLEN];	/* one printer line (8 rows) of graphics */
int  far width;				/* width of the screen (pixels) */
int  far height;			/* height of the screen (pixels) */

struct videoconfig vcRec;
struct videoconfig far *vc = &vcRec;

void far *oldint;

static union REGS inregs, outregs;

/*
** macro to output a byte to the printer 
*/
#define print(c)	_bios_printer(_PRINTER_WRITE, printer, c)


/*
** Print linebuf on the printer
**
**		Does not take aspect ratio (if any) into account.
*/
void far printline()
{
	register int i;

	/* header: */
	/* ESC E  for compressed print, ESC T 1 6 for close spacing */
	/* ESC S 0 7 2 0 for NEC printer and 720 columns. */

	print(27); print('E');
	print(27); print('T'); print('1'); print('6');
	print(27); print('S');
	print('0'); print('7'); print('2'); print('0');

	/* body:  use nested loops if you have to split it into short blocks */
	for (i = 0; i < width; ++i) {
		print(linebuf[i]);
	}
	/* trailer */
	print('\r');
	print('\n');
}

/*
** Get a line from the screen into linebuf
**		In my printer, at least, the least significant bit is on top.
*/
void far getline(row)
	int row;
{
	register int r, c;
	register char d;

	for (c = 0; c < width; ++c) {
		for (r = 8, d = 0; r--; ) {
			if ((r + row) >= height) 
				d = d << 1;
			else 
				d = (d << 1) | (1 & _getpixel(c, r + row));
		}
		linebuf[c] = d;
	}
}

void far prline(s)
	char *s;
{
	for (; *s; ++s) print(*s);
	print('\r'); print('\n');
}

/*
** Print the screen
*/
interrupt far printscreen()
{
	register int row;

	/* might be able to get vid mode from int 10h function F */
	/* the result comes back in AL */

	inregs.h.ah = 15;				/* what's the mode? */
	int86(0x10, &inregs, &outregs);
	if (outregs.h.al == _TEXTMONO) 
		_chain_intr(oldint);

	/* === might need to initialize the printer === */
	/* === _bios_printer(printer, _PRINTER_INIT, 0); === */

	for (row = 0; row < height; row += 8) {
		getline(row);
		printline();
	}
}


main (argc, argv)
	int argc;
	char **argv;
{
	/* 
	** It seems _getvideoconfig cheats -- you have to call _setvideomode
	** first, in the same execution.  Trust Microsoft to screw it up.
	*/
	if (argc > 1) {
		_setvideomode(_TEXTMONO);
		exit(0);
	}
	if (!_setvideomode(_HERCMONO)) {
		prline("Can't set herc mode.");
		exit(1);
	}
	_getvideoconfig(vc);
	height = vc -> numypixels;
	width  = vc -> numxpixels;

	oldint = _dos_getvect(PRTSCR);
	_dos_setvect(PRTSCR, printscreen);
	_dos_keep(0, memsize);
}
