#
# This file was generated from metrics.pro.in on Fri Feb 24 16:02:34 CET 2012
#

TEMPLATE     =	app.t
CONFIG       =	console warn_on debug
HEADERS      =	
SOURCES      =	main.cpp
unix:LIBS                   += -L../../../../lib -L../../lib -ldoxmlparser -lqtools
win32:INCLUDEPATH           += .
win32-mingw:LIBS            += -L../../../../lib -L../../lib -ldoxmlparser -lqtools
win32-msvc:LIBS             += doxmlparser.lib qtools.lib shell32.lib 
win32-msvc:TMAKE_LFLAGS     += /LIBPATH:..\..\..\..\lib;..\..\lib
win32-borland:LIBS          += doxmlparser.lib qtools.lib shell32.lib
win32-borland:TMAKE_LFLAGS  += -L..\..\..\..\lib -L..\..\lib
win32:TMAKE_CXXFLAGS        += -DQT_NODLL
DESTDIR                     = 
OBJECTS_DIR                 = obj
TARGET                      = metrics
DEPENDPATH                  = ../../include
INCLUDEPATH                += ../../../../qtools ../../include
unix:TARGETDEPS             = ../../lib/libdoxmlparser.a
win32:TARGETDEPS            = ..\..\lib\doxmlparser.lib

TMAKE_MOC = /usr/bin/moc
