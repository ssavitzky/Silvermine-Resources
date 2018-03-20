# makefile for /rd7/rd3/as/cuv
#
GLIB = graphics.lib
CC = cl
L = ../lib
T = ../
STDH = c:\c600\include\std.h
DFLAGS = -Zi
CFLAGS = -AL -Oi -I. -I.. $(DFLAGS)

BDIR = a:as/cuv
RDIR = a:as/cuv

HDRS = cssspec.h irspec.h lsspec.h  okrec.h
SRC = uv3do.c convir.c convuv.c convls.c uv3set.c
OTHER= makefile

#
## ... and linking
#
OBJ   =  uv3do.obj convir.obj convuv.obj convls.obj uv3set.obj
TOOLS = ../sp.lib

all: uv.lib

clean:
	-rm *.obj

uv.lib:	$(OBJ)
	-rm uv.lib
	lib uv +uv3do+convir+convuv+convls+uv3set,,uv

#
### Library modules
#
uv3do.obj:	uv3do.c $(HDRS)
convir.obj:     convir.c $(HDRS)
convuv.obj:     convuv.c $(HDRS)
convls.obj:     convls.c $(HDRS)
uv3set.obj:     uv3set.c $(HDRS)

#
### Test Programs
#
#

### Backups
#
backup:	backup.hdr backup.src backup.oth backup.tst

backup.hdr:	$(HDRS)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.src:	$(SRC)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.oth:	$(OTHER)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.tst:	$(TESTS)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

#
### Release
#
release: release.hdr release.src release.oth release.tst

release.hdr:	$(HDRS)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.src:	$(SRC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.oth:	$(OTHER)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.tst:	$(TESTS)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)


