# makefile for /rd7/as
#
GLIB = graphics.lib
CC = cl
L = ../lib
STDH = std.h
#DFLAGS = -Zi -qc
CFLAGS = -AL -I. $(DFLAGS)

BDIR = a:as
RDIR = a:as

#
## Macros
#
GOBJ   =  spset.obj krecsw1.obj graphsp.obj grstatus.obj cpystr.obj
JOBJ   =  jcampdo.obj dataout.obj jchdr1.obj jchdr2.obj\
            jchdr3.obj do17.obj difdupo.obj gttitle.obj
JTOOLS =  itob.obj cpystr.obj krecsw1.obj\
            jcutils.obj jcopens.obj zbufr.obj dtof.obj 

OTHER  =  jcamp.hdr makefile
HDRS   =  std.h spec.h 9spec.h spec18.h ftspec.h irspec.h lsspec.h uvspec.h 
GSRC   =  spset.c krecsw1.c graphsp.c grstatus.c cpystr.c
JSRC   =  jcampdo.c dataout.c jchdr1.c jchdr2.c\
            jchdr3.c do17.c difdupo.c gttitle.c
JTSRC  =  itob.c dtof.c cpystr.c krecsw1.c\
            jcutils.c jcopens.c zbufr.c
TESTS  =  testgr.c testjc.c tscrn.c tsprint.c tcpystr.c 


all: sp.lib
#	cd pedm; make; cd ..

clean:
	-rm *.obj
	-rm *.exe
#	cd pedm; make clean; cd ..

sp.lib:	$(GOBJ)	$(JOBJ) $(JTOOLS)
	-rm sp.lib
	lib sp +spset+graphsp+grstatus,,sp
	lib sp +jcampdo+dataout+jchdr1+jchdr2+jchdr3+do17+difdupo+gttitle,,sp
	lib sp +itob+cpystr+krecsw1+jcutils+jcopens+zbufr+dtof,,sp


#
### Test Programs
#
testgr.exe: testgr.obj sp.lib
	cl -Zi testgr sp.lib $(GLIB)
testjc.exe: testjc.obj sp.lib
	cl testjc sp.lib

testgr.obj:	testgr.c $(HDRS)
testjc.obj:	testjc.c $(HDRS)


#
### Library modules
#

spset.obj:	spset.c $(HDRS)
krecsw1.obj:	krecsw1.c $(HDRS)
graphsp.obj:	graphsp.c $(HDRS)
spstatus.obj:	spstatus.c $(HDRS)
cpystr.obj:	cpystr.c $(HDRS)
itob.obj:	itob.c $(HDRS)
dtof.obj:	dtof.c $(HDRS)
zbufr.obj:	zbufr.c $(HDRS)
jcutils.obj:	jcutils.c $(HDRS)
dataout.obj:	dataout.c $(HDRS)
jchdr1.obj:	jchdr1.c $(HDRS)
jchdr2.obj:	jchdr2.c $(HDRS)
jchdr3.obj:	jchdr3.c $(HDRS)
do17.obj:	do17.c $(HDRS)
difdupo.obj:	difdupo.c $(HDRS)
jcopens.obj:	jcopens.c $(HDRS)
jcampdo.obj:	jcampdo.c $(HDRS)
gttitle.obj:	gttitle.c $(HDRS)


#
### Backups
#
backup:	backup.hdr backup.gr backup.js backup.jt backup.oth backup.tst
	cd pedm; make backup; cd ..
	cd cuv; make backup; cd ..

backup.hdr:	$(HDRS)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.gr:	$(GSRC)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.js:	$(JSRC)
	c:/bin/cp $? $(BDIR)
	c:/bin/date > $@

backup.jt:	$(JTSRC)
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
release: release.hdr release.gr release.js release.jt release.oth release.tst
	cd pedm; make release; cd ..
	cd cuv; make release; cd ..

release.hdr:	$(HDRS)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.gr:	$(GSRC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.js:	$(JSRC)
	-mkdir $(RDIR)
	c:/bin/cp $? $(RDIR)
	c:/bin/date > $@
	cp $@ $(RDIR)

release.jt:	$(JTSRC)
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

