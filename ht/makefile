# makefile for /steve/rd7/ht -- HyperTree
#

CC = cl
L = ../lib
DFLAGS = -Zi
CFLAGS = $(DFLAGS) -AL -I../lib

BDIR = e:ht
RDIR = a:ht

#
## File groups for backup
#
HDR   =	data.h convert.h version.h
SRC   =	data.c convert.c help.c main.c
OTHER = makefile ht.lnk
DOC   = release.not readme

#
## ... and linking
#
OBJ   = main.obj convert.obj

all:  ht.exe # shipper.exe raw.exe

clean:
	-rm *.obj
	-rm *.exe

#
### Library modules
#
LIBH = $L/curse.h $L/menu.h
LIBO = $L/curse.obj $L/menu.obj

COOPH = $L/coops.h $L/dir.h $L/disk.h $L/screen.h $L/tree.h $L/view.h $L/viewer.h $L/filevr.h
COOPO = $L/coops.obj $L/dir.obj $L/disk.obj $L/screen.obj $L/tree.obj $L/view.obj $L/viewer.obj $L/filevr.obj

#
### Main Program(s): Linking
#
ht.exe: data.obj help.obj $(OBJ) $(COOPO) $(LIBO) ht.lnk
	link @ht.lnk

#
### Main Program(s): Compiling
#
help.obj:	help.c version.h
data.obj:	data.c data.h convert.h $(COOPH) $(LIBH)
main.obj:	main.c data.h convert.h $(COOPH) $(LIBH)
convert.obj:	convert.c convert.h $(COOPH)

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

backup.oth: $(OTHER) $(DOC) 
	c:/bin/cp $? $(BDIR)
	-c:/bin/date > $@

#
### Release
#
full_release:	$(HDR) $(SRC) $(OTHER)
	-mkdir $(RDIR)
	c:/bin/cp $(HDR) $(RDIR)
	-c:/bin/date > release.hdr
	c:/bin/cp $(SRC) $(RDIR)
	-c:/bin/date > release.src
	c:/bin/cp $(OTHER) $(RDIR)
	-c:/bin/date > release.oth
	cp readme release.not $(RDRV):/
	df $(RDRV)

release: release.hdr release.src release.oth
	cp readme release.not $(RDRV):/
	df $(RDRV)

release.hdr: $(HDR)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.src: $(SRC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.oth: $(OTHER)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)


