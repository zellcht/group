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

all: class/html/index.html \
     define/html/index.html \
     enum/html/index.html \
     file/html/index.html \
     func/html/index.html \
     page/html/index.html \
     relates/html/index.html \
     author/html/index.html \
     par/html/index.html \
     overload/html/index.html \
     example/html/index.html \
     include/html/index.html \
     qtstyle/html/index.html \
     jdstyle/html/index.html \
     structcmd/html/index.html \
     autolink/html/index.html \
     restypedef/html/index.html \
     afterdoc/html/index.html \
     template/html/index.html \
     tag/html/index.html \
     group/html/index.html \
     diagrams/html/index.html \
     memgrp/html/index.html \
     docstring/html/index.html \
     pyexample/html/index.html \
     tclexample/html/index.html \
     mux/html/index.html \
     manual/html/index.html 

# Disabled since it is broken :-(
#     dbusxml/html/index.html

clean:
	rm -rf  class define enum file func page relates author \
                par overload example include qtstyle jdstyle structcmd \
		autolink tag restypedef afterdoc template tag group diagrams \
		memgrp docstring pyexample mux manual dbusxml tclexample

class/html/index.html: class.h class.cfg
	$(DOXYGEN)/bin/doxygen class.cfg

define/html/index.html: define.h define.cfg
	$(DOXYGEN)/bin/doxygen define.cfg

enum/html/index.html: enum.h enum.cfg
	$(DOXYGEN)/bin/doxygen enum.cfg

file/html/index.html: file.h file.cfg
	$(DOXYGEN)/bin/doxygen file.cfg

func/html/index.html: func.h func.cfg
	$(DOXYGEN)/bin/doxygen func.cfg

page/html/index.html: page.doc page.cfg
	$(DOXYGEN)/bin/doxygen page.cfg

relates/html/index.html: relates.cpp relates.cfg
	$(DOXYGEN)/bin/doxygen relates.cfg

author/html/index.html: author.cpp author.cfg
	$(DOXYGEN)/bin/doxygen author.cfg

par/html/index.html: par.cpp par.cfg
	$(DOXYGEN)/bin/doxygen par.cfg

overload/html/index.html: overload.cpp overload.cfg
	$(DOXYGEN)/bin/doxygen overload.cfg

example/html/index.html: example.cpp example_test.cpp example.cfg 
	$(DOXYGEN)/bin/doxygen example.cfg

include/html/index.html: include.cpp example_test.cpp include.cfg 
	$(DOXYGEN)/bin/doxygen include.cfg

qtstyle/html/index.html: qtstyle.cpp qtstyle.cfg 
	$(DOXYGEN)/bin/doxygen qtstyle.cfg

jdstyle/html/index.html: jdstyle.cpp jdstyle.cfg 
	$(DOXYGEN)/bin/doxygen jdstyle.cfg

structcmd/html/index.html: structcmd.h structcmd.cfg 
	$(DOXYGEN)/bin/doxygen structcmd.cfg

autolink/html/index.html: autolink.cpp autolink.cfg 
	$(DOXYGEN)/bin/doxygen autolink.cfg

tag/html/index.html: tag.cpp tag.cfg example/html/index.html
	$(DOXYGEN)/bin/doxygen tag.cfg
#	sed -e "1,1s#perl#$(PERL)#g" tag/html/installdox >tag/html/installdox.perl
#	cd tag/html ; $(PERL) installdox.perl -lexample.tag@../../example/html

restypedef/html/index.html: restypedef.cpp restypedef.cfg 
	$(DOXYGEN)/bin/doxygen restypedef.cfg

afterdoc/html/index.html: afterdoc.h afterdoc.cfg
	$(DOXYGEN)/bin/doxygen afterdoc.cfg

template/html/index.html: templ.cpp templ.cfg
	$(DOXYGEN)/bin/doxygen templ.cfg

group/html/index.html: group.cpp group.cfg
	$(DOXYGEN)/bin/doxygen group.cfg

memgrp/html/index.html: memgrp.cpp memgrp.cfg
	$(DOXYGEN)/bin/doxygen memgrp.cfg

pyexample/html/index.html: pyexample.py pyexample.cfg
	$(DOXYGEN)/bin/doxygen pyexample.cfg

tclexample/html/index.html: tclexample.tcl tclexample.cfg
	$(DOXYGEN)/bin/doxygen tclexample.cfg

mux/html/index.html: mux.vhdl mux.cfg
	$(DOXYGEN)/bin/doxygen mux.cfg

manual/html/index.html: manual.c manual.cfg
	$(DOXYGEN)/bin/doxygen manual.cfg

docstring/html/index.html: docstring.py docstring.cfg
	$(DOXYGEN)/bin/doxygen docstring.cfg

#dbusxml/html/index.html: dbusxml.xml dbusxml.cfg
#	$(DOXYGEN)/bin/doxygen dbusxml.cfg

diagrams/html/index.html: diagrams_a.h diagrams_b.h diagrams_c.h diagrams_d.h diagrams_e.h diagrams.cfg
ifneq ($(HAVE_DOT),)
	$(DOXYGEN)/bin/doxygen diagrams.cfg
endif

