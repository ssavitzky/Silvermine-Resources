# makefile for /steve/rd7/doc
#
CC = cl
L = ../lib
CFLAGS = -Zi -AL

BDIR =	a:doc
RDIR =	a:doc

#
## File groups for backup
#
DOC   = manual.tex label.tex brochure.tex rd7.tex order.rd7


all: 


#
### Backups
#
backup: backup.doc

backup.doc: $(DOC)
	c:/bin/cp $? $(BDIR)
	-c:/bin/date > $@

#
### Release
#
release: release.doc

release.doc: $(DOC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	-c:/bin/date > $@
	cp $@ $(RDIR)


