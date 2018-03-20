# makefile for /rd7/as/pedm
#
GLIB = graphics.lib
CC = cl
L = ../lib
T = ../
STDH = c:\msc\include\std.h
#DFLAGS = -Zi -qc
CFLAGS = -AL -I. -I.. $(DFLAGS)

BDIR = a:as/pedm
RDIR = a:as/pedm

HDRS = defs.h dmkrec.h dmspec.h dmstd.h error.h \
	ovl.h portab.h slm.h spec.h          
DSRC = dmdo.c dmopens.c encode.c enc_msb.c
OTHER= makefile
TESTS= testdm.c

#
## ... and linking
#
DOBJ   =  testdm.obj dmdo.obj dmopens.obj encode.obj enc_msb.obj 
DTOOLS = ../sp.lib

all: dm.lib

clean:
	-rm *.obj

dm.lib:	$(DOBJ)
	-rm dm.lib
	lib dm +dmdo+dmopens+encode+enc_msb,,dm

#
### Library modules
#
dmdo.obj:	dmdo.c $(HDRS)
dmopens.obj:	dmopens.c $(HDRS)
encode.obj:	encode.c $(HDRS)
enc_msb.obj:	enc_msb.c $(HDRS)

#
### Test Programs
#
#
#testdm.exe: testdm.obj dm.lib $(DTOOLS)
#	cl -Zi testdm dm.lib $(DTOOLS)

#
### Backups
#
backup:	backup.hdr backup.src backup.oth backup.tst

backup.hdr:	$(HDRS)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.src:	$(DSRC)
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

release.src:	$(DSRC)
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