#
# This file was generated from Makefile.in on Fri Feb 24 16:02:34 CET 2012
#

DOXYGEN   = /Users/dimitri/doxygen
TMAKEPATH = /Users/dimitri/doxygen/tmake/lib/macosx-c++
ENV       = env TMAKEPATH=$(TMAKEPATH)
TMAKE     = /Users/dimitri/doxygen/tmake/bin/tmake
MAKE      = /opt/local/bin/gmake
PERL      = /opt/local/bin/perl
RM        = rm -f
CP        = cp
VERSION   = 1.7.6.1
INSTALL   = /usr/local
INSTTOOL  = /opt/local/bin/ginstall
DOXYDOCS  = ..
DOCDIR    = $(INSTALL)/share/doc/packages/doxygen
QTDIR     = /usr
HAVE_DOT  = /usr/local/bin/dot
MKSPECS   = -spec macx-g++
all clean depend: Makefile.xmlparse 
	$(MAKE) -f Makefile.xmlparse $@

distclean: clean
	$(RM) -rf Makefile.xmlparse xmlparse.pro Makefile obj

tmake:
	$(ENV) $(PERL) $(TMAKE) xmlparse.pro >Makefile.xmlparse

Makefile.xmlparse: xmlparse.pro
	$(ENV) $(PERL) $(TMAKE) xmlparse.pro >Makefile.xmlparse

install:
