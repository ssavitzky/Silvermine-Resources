# makefile for /rd7/as/cuv
#
GLIB = graphics.lib
CC = cl
L = ../lib
T = ../
STDH = c:\msc\include\std.h
##DFLAGS = -Zi
CFLAGS = -AL -Oi -I. -I.. $(DFLAGS)

BDIR = a:as/cuv
RDIR = a:as/cuv

HDRS = ../spec.h
SRC = uvdo.c uvset.c
OTHER= makefile
TESTS= testuv.c

#
## ... and linking
#
OBJ   =  testuv.obj uvdo.obj uvset.obj 
TOOLS = ../sp.lib

all: uv.lib

clean:
	-rm *.obj

uv.lib:	$(OBJ)
	-rm uv.lib
	lib uv +uvdo+uvset,,uv

#
### Library modules
#
uvdo.obj:	uvdo.c $(HDRS)
uvset.obj:	uvset.c $(HDRS)

#
### Test Programs
#
#
#testuv.exe: testuv.obj uv.lib $(TOOLS)
#	cl -Zi testuv uv.lib $(TOOLS)

#
### Backups
#
backup:	backup.src backup.oth backup.tst

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
release: release.src release.oth release.tst

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