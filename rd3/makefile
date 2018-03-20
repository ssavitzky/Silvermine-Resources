# makefile for /steve/rd7/rd3 -- read/write PE-3500/3600 disks
#

CC = cl
L = ../lib
DFLAGS = -Zi
CFLAGS = $(DFLAGS) -AL -I../lib
MKDIR = c:/bin/mkdir

BDIR = a:/rd3
RDRV = a
RDIR = $(RDRV):/rd3

#
## File groups for backup
#
HDR   =	data.h convert.h petos.h version.h
SRC   =	data.c convert.c petos.c petosdir.c help.c main.c
OTHER = makefile rd3.lnk rd3sp.lnk shipper.c
DOC   = release.not readme

#
## ... and linking
#
OBJ   = petos.obj petosdir.obj

all:  rd3sp.exe # rd3.exe shipper.exe raw.exe

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
rd3.exe: main.obj data.obj help.obj convert.obj \
	 $(OBJ) $(COOPO) $(LIBO) rd3.lnk
	link @rd3.lnk

rd3sp.exe: mainsp.obj datasp.obj helpsp.obj converts.obj \
	   $(OBJ) $(COOPO) $(LIBO) rd3sp.lnk
	link @rd3sp.lnk

shipper.exe: shipper.c version.h
	$(CC) $(CFLAGS) shipper.c -o shipper.exe
#
### Main Program(s): Compiling
#
main.obj:	main.c data.h convert.h $(COOPH) $(LIBH) # petos.h
help.obj:	help.c version.h
data.obj:	data.c data.h convert.h $(COOPH) $(LIBH)
convert.obj:	convert.c convert.h $(COOPH)

mainsp.obj:	main.c data.h convert.h $(COOPH) $(LIBH) # petos.h
	$(CC) -c $(CFLAGS) -DV_SP -Fo$@ main.c
helpsp.obj:	help.c version.h
	$(CC) -c $(CFLAGS) -DV_SP -Fo$@ help.c
datasp.obj:	data.c data.h convert.h $(COOPH) $(LIBH)
	$(CC) -c $(CFLAGS) -DV_SP -Fo$@ data.c
converts.obj:	convert.c convert.h $(COOPH)
	$(CC) -c $(CFLAGS) -DV_SP -Fo$@ convert.c

petos.obj:	petos.c petos.h $(COOPH)
petosdir.obj:	petosdir.c petos.h $(COOPH)

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
	-$(MKDIR) $(RDIR)
	c:/bin/cp $(HDR) $(RDIR)
	-c:/bin/date > release.hdr
	c:/bin/cp $(SRC) $(RDIR)
	-c:/bin/date > release.src
	c:/bin/cp $(OTHER) $(RDIR)
	-c:/bin/date > release.oth
	cp readme release.not $(RDRV):/
	df $(RDRV)

release: release.hdr release.src release.oth
	cp readme release.not rd3sp.exe shipper.exe $(RDRV):/
	df $(RDRV)

release.hdr: $(HDR)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.src: $(SRC)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)

release.oth: $(OTHER)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)


