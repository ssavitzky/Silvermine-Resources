PROJ	=TESTJC
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AL /W1 /Ze 
CFLAGS_D	= /Zi /Zr /Od /DDEBUG 
CFLAGS_R	= /O /Ot /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	=/NOI
LFLAGS_D	=/CO
LFLAGS_R	=
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
OBJS_EXT = 	
LIBS_EXT = 	

all:	$(PROJ).exe

testjc.obj:	testjc.c

cpystr.obj:	cpystr.c

dataout.obj:	dataout.c

difdupo.obj:	difdupo.c

do17.obj:	do17.c

dtof.obj:	dtof.c

gttitle.obj:	gttitle.c

itob.obj:	itob.c

jcampdo.obj:	jcampdo.c

jchdr1.obj:	jchdr1.c

jchdr2.obj:	jchdr2.c

jchdr3.obj:	jchdr3.c

jcopens.obj:	jcopens.c

jcutils.obj:	jcutils.c

krecsw1.obj:	krecsw1.c

spset.obj:	spset.c

zbufr.obj:	zbufr.c

$(PROJ).exe:	cpystr.obj dataout.obj difdupo.obj do17.obj dtof.obj gttitle.obj itob.obj jcampdo.obj jchdr1.obj jchdr2.obj jchdr3.obj jcopens.obj jcutils.obj krecsw1.obj spset.obj zbufr.obj testjc.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
testjc.obj +
cpystr.obj +
dataout.obj +
difdupo.obj +
do17.obj +
dtof.obj +
gttitle.obj +
itob.obj +
jcampdo.obj +
jchdr1.obj +
jchdr2.obj +
jchdr3.obj +
jcopens.obj +
jcutils.obj +
krecsw1.obj +
spset.obj +
zbufr.obj +
$(OBJS_EXT)
$(PROJ).exe

$(LIBS_EXT);
<<
	ilink -a -e "link $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).exe
	$(PROJ) $(RUNFLAGS)

