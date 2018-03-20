# makefile for /steve/rd7/lib
#
CC = cl
#DFLAGS = -Zi
CFLAGS = -AL -Oi

BDIR = a:lib
RDIR = a:lib

SRC = curse.c menu.c
HDR = curse.h menu.h error.h ibmchars.h ibmkeys.h dostypes.h
OTHER = makefile menutst.c

all: curse.obj menu.obj
tests: menutst.exe

clean:
	-rm *.obj
	-rm *.exe

##
#  Library modules
##
curse.obj: curse.c curse.h
menu.obj: menu.c menu.h curse.h error.h

##
#  Test Programs
##
extest.obj: extest.c expand.h error.h scanner.h
extest.exe: extest.obj expand.obj error.obj scanner.obj
	cl extest expand error scanner

iotst.obj: iotst.c curse.h
iotst.exe: iotst.obj curse.obj rawgetc.obj
	cl iotst curse rawgetc

menutst.obj: menutst.c menu.h curse.h
menutst.exe: menutst.obj menu.obj curse.obj
	cl menutst menu curse

##
#  Backups
##
backup: backup.src backup.hdr backup.oth 

backup.src: $(SRC)
	cp $? $(BDIR)
	-date > $@

backup.hdr: $(HDR)
	cp $? $(BDIR)
	-date > $@

backup.oth: $(OTHER)
	cp $? $(BDIR)
	-date > $@

#
### Release
#
release: release.src release.hdr release.oth 

fake_release: 
	-mkdir $(RDIR)
	cp $(SRC) $(RDIR)
	cp $(HDR) $(RDIR)
	cp $(OTHER) $(RDIR)

release.src: $(SRC)
	-mkdir $(RDIR)
	cp $? $(RDIR)
	-date > $@
	cp $@ $(RDIR)

release.hdr: $(HDR)
	-mkdir $(RDIR)
	cp $? $(RDIR)
	-date > $@
	cp $@ $(RDIR)

release.oth: $(OTHER)
	-mkdir $(RDIR)
	cp $? $(RDIR)
	-date > $@
	cp $@ $(RDIR)

