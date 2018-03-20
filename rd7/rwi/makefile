# makefile for /steve/rd7/rwi: rwi -- read/write iRMX86 disks
#			       raw -- read/write RAW disks
#
#	QFLAGS specifies the quickC compiler for things like help.c
#	which require neither huge pointers nor optimization.

CC = cl
L = ../lib
DFLAGS = -Zi
CFLAGS = $(DFLAGS) -AL
QFLAGS = -qc -AL
HFLAGS = $(DFLAGS) -AL

BDIR = a:rwi
RDIR = a:rwi

#
## File groups for backup
#
HDR   =	data.h convert.h dirs.h disk.h irmx.h help.h version.h\
	screen.h coops.h trees.h view.h viewer.h
SRC   =	data.c convert.c dirs.c disk.c irmx.c help.c\
	screen.c main.c coops.c trees.c view.c viewer.c
OTHER = makefile shipper.c 
DOC   = internal.not release.not readme

#
## ... and linking
#
OBJ   = main.obj convert.obj dirs.obj disk.obj irmx.obj screen.obj\
	view.obj viewer.obj
LNK   = main convert dirs disk irmx screen view viewer $(COOP) $(LIB)

all:  rwi.exe # shipper.exe raw.exe

clean:
	-rm *.obj
	-rm *.exe

#
### Library modules
#
LIBO = $L/curse.obj $L/menu.obj
LIBH = $L/curse.h $L/menu.h
LIB  = $L/curse $L/menu

COOP  = coops trees
COOPH = coops.h trees.h
COOPO = coops.obj trees.obj

#
### Main Program(s): Linking
#
rwi.exe: data.obj help.obj $(OBJ) $(COOPO) $(LIBO)
	cl $(DFLAGS) -o $@ data help $(LNK)
	mv $@ foo.exe
	exepack foo.exe $@
	rm foo.exe

raw.exe: data_R.obj help_R.obj $(OBJ) $(COOPO) $(LIBO)
	cl -o $@ data_R help_R $(LNK)
	mv $@ foo.exe
	exepack foo.exe $@
	rm foo.exe

#
### Main Program(s): Compiling
#
help.obj:	help.c version.h

data.obj:	data.c data.h disk.h irmx.h convert.h dirs.h view.h viewer.h screen.h $(COOPH) $(LIBH)
data_R.obj:	data.c data.h disk.h irmx.h convert.h dirs.h view.h viewer.h screen.h $(COOPH) $(LIBH)
	$(CC) $(CFLAGS) -c -Fodata_R.obj -DV_RAW data.c
help_R.obj:	help.c version.h
	$(CC) $(CFLAGS) -c -Fohelp_R.obj -DV_RAW help.c

main.obj:	main.c data.h disk.h irmx.h convert.h dirs.h version.h view.h viewer.h screen.h $(COOPH) $(LIBH)
dirs.obj:	dirs.c dirs.h $(COOPH)
convert.obj:	convert.c convert.h disk.h dirs.h $(COOPH)
disk.obj:	disk.c disk.h dirs.h viewer.h $(COOPH)
irmx.obj:	irmx.c disk.h irmx.h dirs.h viewer.h $(COOPH)
screen.obj:	screen.c screen.h data.h view.h viewer.h dirs.h $(COOPH) $L/curse.h
view.obj:	view.c view.h viewer.h dirs.h $(COOPH) $L/curse.h
viewer.obj:	viewer.c viewer.h dirs.h $(COOPH) $L/curse.h

trees.obj:	trees.c $(COOPH)
coops.obj:	coops.c coops.h

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
	c:/bin/cp readme release.not A:/
	df A

release: release.hdr release.src release.oth
	cp readme release.not A:/
	df A

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


