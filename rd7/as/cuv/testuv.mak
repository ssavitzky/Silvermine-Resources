PROJ	=TESTUV
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AL /W1 /Ze 
CFLAGS_D	= /Zi /Zr /Gi$(PROJ).mdt /Od 
CFLAGS_R	= /O /Ot /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	=/NOI
LFLAGS_D	=/INCR /CO
LFLAGS_R	=
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
OBJS_EXT = 	
LIBS_EXT = 	

all:	$(PROJ).exe

testuv.obj:	testuv.c

uvdo.obj:	uvdo.c

spset.obj:	..\spset.c

krecsw1.obj:	..\krecsw1.c

cpystr.obj:	..\cpystr.c

dmopens.obj:	..\pedm\dmopens.c

zbufr.obj:	..\zbufr.c

$(PROJ).exe:	testuv.obj uvdo.obj spset.obj krecsw1.obj cpystr.obj dmopens.obj zbufr.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
testuv.obj +
uvdo.obj +
spset.obj +
krecsw1.obj +
cpystr.obj +
dmopens.obj +
zbufr.obj +
$(OBJS_EXT)
$(PROJ).exe

$(LIBS_EXT);
<<
	ilink -a -e "link $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).exe
	$(PROJ) $(RUNFLAGS)

