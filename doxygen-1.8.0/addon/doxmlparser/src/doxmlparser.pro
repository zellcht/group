#
# This file was generated from doxmlparser.pro.in on Fri Feb 24 16:02:34 CET 2012
#

TEMPLATE     =	lib.t
CONFIG       =	console staticlib warn_on debug
HEADERS      =	basehandler.h mainhandler.h \
                compoundhandler.h sectionhandler.h \
                memberhandler.h paramhandler.h \
                dochandler.h linkedtexthandler.h \
                debug.h graphhandler.h stringimpl.h \
                loamhandler.h
SOURCES      =	mainhandler.cpp \
                compoundhandler.cpp sectionhandler.cpp \
                memberhandler.cpp paramhandler.cpp \
                dochandler.cpp linkedtexthandler.cpp \
                basehandler.cpp debug.cpp graphhandler.cpp \
                loamhandler.cpp
unix:LIBS                  += -L../../../lib -lqtools 
win32:INCLUDEPATH          += .
win32-mingw:LIBS           += -L../../../lib -lqtools 
win32-msvc:LIBS            += qtools.lib shell32.lib 
win32-msvc:TMAKE_LFLAGS    += /LIBPATH:....\\..\lib
win32-borland:LIBS         += qtools.lib doxycfg.lib shell32.lib
win32-borland:TMAKE_LFLAGS += -L..\..\..\lib
win32:TMAKE_CXXFLAGS       += -DQT_NODLL
DESTDIR                     = ../lib
OBJECTS_DIR                =  ../objects
TARGET                      = doxmlparser
INCLUDEPATH                += ../../../qtools ../include

TMAKE_MOC = /usr/bin/moc
