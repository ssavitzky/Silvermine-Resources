# makefile for /steve/rd7
#
# NOTE: 'backup' requires two 360K disks
#	., ./SS, and ./DOC on one; ./AS and ./LIB on the other.
#	./OLD and ./TEST must be backed up manually
#
#	'release' may require one or two disks depending on the
#	number of files that may have changed.
#
CC = cl
CFLAGS = -Zi -AL
BDIR =	a:
RDIR =	a:

#
## File groups for backup
#

ALL = makefile readme release.not
EXE = shipper.exe prherc.exe cp.exe rm.exe mv.exe

all:  $(ALL)
	cd lib; make; cd ..
	cd as; make; cd ..
	cd ss; make; cd ..

clean:
	-rm shipper.exe 
	cd lib; make clean; cd ..
	cd as; make clean; cd ..
	cd ss; make clean; cd ..

#
### Backups
#
backup: backup.top
	cd ss; make backup; cd ..
	cd doc; make backup; cd ..
	df a
	@echo "Insert disk for ./LIB, ./AS and make backup2"

backup2:
	cd lib; make backup; cd ..
	cd as; make backup; cd ..
	df a

backup.top: $(ALL)
	cp $? $(BDIR)
	c:/bin/date > $@

#
### Release
#
release: release.top
	cd ss; make release; cd ..
	cd doc; make release; cd ..
	df a
	@echo "Insert disk for ./LIB, ./AS and make release2"

release2:
	cd lib; make release; cd ..
	cd as; make release; cd ..
	df a

release.top: $(ALL) $(EXE)
	cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)



