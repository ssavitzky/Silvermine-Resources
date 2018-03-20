# makefile for /rd7/rd3/as
#
GLIB = graphics.lib
CC = cl
L = ../lib
#DFLAGS = -Zi -qc
CFLAGS = -AL -I../../as $(DFLAGS)
MKDIR = c:/bin/mkdir

BDIR = e:/rd3/as
RDIR = e:/rd3/as

#
## Macros
#
GSRC = 	graph3sp.c graphset.c 
JSRC = 	dataout.c difdupo.c getstol.c jc3hdr1.c \
	jc3hdr2.c jc3hdr3.c jcamp3.c kr3to7pc.c flips.c
HDRS = 	okrec.h
OTHER =	jcamp.hdr makefile

GOBJ = 	graph3sp.obj graphset.obj
JOBJ = 	dataout.obj difdupo.obj getstol.obj jc3hdr1.obj \
	jc3hdr2.obj jc3hdr3.obj	jcamp3.obj kr3to7pc.obj	flips.obj

all: sp3.lib

clean:
	-rm *.obj

sp3.lib:	$(GOBJ)	$(JOBJ)
	-rm sp3.lib
	lib sp3 +graph3sp+graphset+jcamp3+getstol+kr3to7pc,,sp3
	lib sp3	+dataout+jc3hdr1+jc3hdr2+jc3hdr3+difdupo+flips,,sp3

#
### Library modules
#
graph3sp.obj:	$(HDRS) 
graphset.obj:	$(HDRS)
dataout.obj:	$(HDRS)
difdupo.obj:	$(HDRS)
getstol.obj:	$(HDRS)
jc3hdr1.obj:	$(HDRS)
jc3hdr2.obj:	$(HDRS)
jc3hdr3.obj:	$(HDRS)
jcamp3.obj:	$(HDRS)
kr3to7pc.obj:	$(HDRS)


#
### Backups
#
backup:	backup.hdr backup.gr backup.js backup.oth

backup.hdr:	$(HDRS)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.gr:	$(GSRC)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.js:	$(JSRC)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.oth:	$(OTHER)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

#
### Release
#
release: release.hdr release.gr release.js release.oth

release.hdr:	$(HDRS)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.gr:	$(GSRC)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.js:	$(JSRC)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.oth:	$(OTHER)
	-$(MKDIR) $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

