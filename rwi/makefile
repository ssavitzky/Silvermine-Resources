# makefile for /steve/rd7/rwi: rwi -- read/write iRMX86 disks
#			       raw -- read/write RAW disks
#
#	QFLAGS specifies the quickC compiler for things like help.c
#	which require neither huge pointers nor optimization.

CC = cl
L = ../lib
DFLAGS = -Zi
CFLAGS = $(DFLAGS) -AL -I../lib
QFLAGS = -qc -AL  -I../lib

BDIR = d:rwi
RDIR = a:rwi

#
## File groups for backup
#
HDR   =	data.h convert.h irmx.h help.h version.h
SRC   =	data.c convert.c irmx.c irmxdir.c help.c main.c
OTHER = makefile shipper.c rwi.lnk raw.lnk
DOC   = internal.not release.not readme

#
## ... and linking
#
OBJ   = main.obj convert.obj irmx.obj irmxdir.obj

all:  rwi.exe # shipper.exe raw.exe

clean:
	-rm *.obj
	-rm *.exe

#
### Library modules
#
LIBH = $L/curse.h $L/menu.h
LIBO = $L/curse.obj $L/menu.obj

COOPH = $L/coops.h $L/dir.h $L/disk.h $L/raw.h $L/screen.h $L/tree.h $L/view.h $L/viewer.h $L/filevr.h
COOPO = $L/coops.obj $L/dir.obj $L/disk.obj $L/raw.obj $L/screen.obj $L/tree.obj $L/view.obj $L/viewer.obj $L/filevr.obj

#
### Main Program(s): Linking
#
rwi.exe: data.obj help.obj $(OBJ) $(COOPO) $(LIBO) rwi.lnk
	link @rwi.lnk

raw.exe: data_R.obj help_R.obj $L/raw.obj $(OBJ) $(COOPO) $(LIBO) raw.lnk
	link @raw.lnk

#
### Main Program(s): Compiling
#
help.obj:	help.c version.h

data.obj:	data.c data.h irmx.h convert.h $(COOPH) $(LIBH)
data_R.obj:	data.c data.h irmx.h convert.h $(COOPH) $(LIBH)
	$(CC) $(CFLAGS) -c -Fodata_R.obj -DV_RAW data.c
help_R.obj:	help.c version.h
	$(CC) $(CFLAGS) -c -Fohelp_R.obj -DV_RAW help.c

main.obj:	main.c data.h irmx.h convert.h $(COOPH) $(LIBH)
convert.obj:	convert.c convert.h $(COOPH)
irmx.obj:	irmx.c irmx.h $(COOPH)
irmxdir.obj:	irmxdir.c irmx.h $(COOPH)

#
### Utilities
#

shipper.exe:	shipper.c version.h
	$(CC) $(HFLAGS) shipper.c

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


