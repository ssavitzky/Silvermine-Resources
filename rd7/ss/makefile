# makefile for /steve/rd7/ss
#
#	QFLAGS specifies the quickC compiler for things like help.c
#	which require neither huge pointers nor optimization.

CC = cl
L = ../lib
SP = ../as
#DFLAGS = -Zi
CFLAGS = $(DFLAGS) -AL -Oi
QFLAGS = -qc -AL
HFLAGS = $(DFLAGS) -AL

BDIR = a:ss
RDIR = a:ss

#
## File groups for backup
#
HDR   =	convert.h dirs.h disk.h help.h rd7.h coops.h trees.h view.h 
SRC   =	convert.c dirs.c disk.c help.c rd7.c coops.c trees.c view.c 
OTHER = makefile rd7sp.lnk shipper.c internal.not coops.not prherc.c

#
## ... and linking
#
OBJ   = convert.obj dirs.obj disk.obj view.obj
LNK   = convert dirs disk view $(COOP) $(LIB)

all: ../rd7sp/rd7.exe ../rd7ab/rd7.exe ../rd7de/rd7.exe \
	../shipper.exe ../prherc.exe

clean:
	-rm *.obj
	-rm *.exe

#
### Library modules
#
LIBO = $L/curse.obj $L/menu.obj
LIBH = $L/curse.h $L/menu.h
LIB  = $L/curse $L/menu

SPLIB = $(SP)/sp.lib $(SP)/pedm/dm.lib $(SP)/cuv/uv.lib

COOP  = coops trees
COOPH = coops.h trees.h
COOPO = coops.obj trees.obj

#
### Main Program & its Modules
#
# rd7sp.exe would use the following cl command,
#	cl rd7sp helpsp $(LNK) $(SPLIB) graphics.lib
# but we can't persuade this version of MAKE and/or CL to pass 
# the right stuff to the linker.
#
rd7sp.exe: rd7sp.obj helpsp.obj $(OBJ) $(COOPO) $(LIBO) $(SPLIB)
	link @rd7sp.lnk
../rd7sp/rd7.exe:	rd7sp.exe
	-rm $@
	exepack rd7sp.exe $@
../rd7ab/rd7.exe: rd7ab.obj helpab.obj $(OBJ) $(COOPO) $(LIBO)
	cl rd7ab helpab $(LNK)
	-rm $@
	exepack rd7ab.exe $@
../rd7de/rd7.exe: rd7de.obj helpde.obj $(OBJ) $(COOPO) $(LIBO)
	cl rd7de helpde $(LNK)
	-rm $@
	exepack rd7de.exe $@

rd7ab.obj:	rd7.c $(HDR) $(LIBH)
	$(CC) $(CFLAGS) -c -Ford7ab.obj -DV_AB rd7.c
rd7sp.obj:	rd7.c $(HDR) $(LIBH)
	$(CC) $(CFLAGS) -c -Ford7sp.obj -DV_SP rd7.c
rd7de.obj:	rd7.c $(HDR) $(LIBH)
	$(CC) $(CFLAGS) -c -Ford7de.obj -DV_DE rd7.c

helpab.obj:	help.c rd7.h
	$(CC) $(CFLAGS) -c -Fohelpab.obj -DV_AB help.c
helpsp.obj:	help.c rd7.h
	$(CC) $(CFLAGS) -c -Fohelpsp.obj -DV_SP help.c
helpde.obj:	help.c rd7.h
	$(CC) $(CFLAGS) -c -Fohelpde.obj -DV_DE help.c

dirs.obj:	dirs.c rd7.h dirs.h view.h $(COOPH)
convert.obj:	convert.c convert.h disk.h rd7.h dirs.h $(COOPH)
disk.obj:	disk.c disk.h rd7.h dirs.h view.h $(COOPH)
view.obj:	view.c rd7.h view.h dirs.h $(COOPH) $L/curse.h

trees.obj:	trees.c $(COOPH)
coops.obj:	coops.c coops.h

#
### Utilities
#

../shipper.exe:	shipper.c rd7.h
	$(CC) $(HFLAGS) shipper.c
	-rm $@
	mv shipper.exe ..

../prherc.exe: prherc.c
	$(CC) -Fm prherc.c graphics.lib
	-rm $@
	mv prherc.exe ..

#
### Backups
#
backup: backup.src backup.hdr backup.oth

backup.src: $(SRC)
	c:/bin/cp $? $(BDIR)
	-c:/bin/date > $@

backup.hdr: $(HDR)
	c:/bin/cp $? $(BDIR)
	-c:/bin/date > $@

backup.oth: $(OTHER)
	c:/bin/cp $? $(BDIR)
	-c:/bin/date > $@

#
### Release
#
release: release.src release.hdr release.oth

release.src: $(SRC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.hdr: $(HDR)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.oth: $(OTHER)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)


