# makefile for /steve/rd7/lib
#
CC = cl
#DFLAGS = -Zi
CFLAGS = -AL -Oi
MKDIR = c:/bin/mkdir

BDIR = a:/lib
RDIR = a:/lib
RDRV = a

SRC   = curse.c menu.c
HDR   = curse.h menu.h error.h ibmchars.h ibmkeys.h dostypes.h
OOHDR = coops.h dir.h disk.h raw.h screen.h tree.h view.h viewer.h filevr.h
OOSRC = coops.c dir.c disk.c raw.c screen.c tree.c view.c viewer.c filevr.c
OTHER = makefile menutst.c

OBJ   = curse.obj menu.obj 
OOBJ  =	coops.obj dir.obj disk.obj raw.obj \
	screen.obj tree.obj view.obj viewer.obj filevr.obj

all: $(OBJ) $(OOBJ)
tests: menutst.exe

clean:
	-rm *.obj
	-rm *.exe

##
#  Library modules
##
curse.obj:	curse.c curse.h
menu.obj:	menu.c menu.h curse.h error.h

coops.obj:	coops.c coops.h
dir.obj:	dir.c dir.h tree.h coops.h
disk.obj:	disk.c disk.h dir.h viewer.h tree.h coops.h
raw.obj:	raw.c disk.h dir.h view.h viewer.h tree.h coops.h
screen.obj:	screen.c screen.h view.h viewer.h dir.h tree.h coops.h curse.h
tree.obj:	tree.c tree.h coops.h
view.obj:	view.c view.h viewer.h dir.h tree.h coops.h curse.h
viewer.obj:	viewer.c viewer.h dir.h tree.h coops.h curse.h
filevr.obj:	filevr.c filevr.h viewer.h dir.h tree.h coops.h curse.h


##
#  Test Programs
##
menutst.obj: menutst.c menu.h curse.h
menutst.exe: menutst.obj menu.obj curse.obj
	cl menutst menu curse

##
#  Backups
##
backup: backup.src backup.hdr backup.oos backup.ooh backup.oth 

backup.oos: $(OOSRC)
	cp $? $(BDIR)
	-c:/bin/date > $@

backup.ooh: $(OOHDR)
	cp $? $(BDIR)
	-c:/bin/date > $@

backup.src: $(SRC)
	cp $? $(BDIR)
	-c:/bin/date > $@

backup.hdr: $(HDR)
	cp $? $(BDIR)
	-c:/bin/date > $@

backup.oth: $(OTHER)
	cp $? $(BDIR)
	-c:/bin/date > $@

#
### Release
#
release: release.src release.hdr release.oos release.ooh release.oth 

full_release: 
	-$(MKDIR) $(RDIR)
	cp $(SRC) $(RDIR)
	-c:/bin/date > release.src
	cp $(HDR) $(RDIR)
	-c:/bin/date > release.hdr
	cp $(OOSRC) $(RDIR)
	-c:/bin/date > release.oos
	cp $(OOHDR) $(RDIR)
	-c:/bin/date > release.ooh
	cp $(OTHER) $(RDIR)
	-c:/bin/date > release.oth

release.src: $(SRC)
	-$(MKDIR) $(RDIR)
	cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.hdr: $(HDR)
	-$(MKDIR) $(RDIR)
	cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.oos: $(OOSRC)
	-$(MKDIR) $(RDIR)
	cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.ooh: $(OOHDR)
	-$(MKDIR) $(RDIR)
	cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.oth: $(OTHER)
	-$(MKDIR) $(RDIR)
	cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

