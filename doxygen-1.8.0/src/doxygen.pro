#
# This file was generated from doxygen.pro.in on Fri Feb 24 16:02:34 CET 2012
#

#
# $Id: doxygen.pro.in,v 1.6 2001/03/19 19:27:40 root Exp $
#
# Copyright (C) 1997-2012 by Dimitri van Heesch.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation under the terms of the GNU General Public License is hereby 
# granted. No representations are made about the suitability of this software 
# for any purpose. It is provided "as is" without express or implied warranty.
# See the GNU General Public License for more details.
#
# Documents produced by Doxygen are derivative works derived from the
# input used in their production; they are not affected by this license.
#
# TMake project file for doxygen

TEMPLATE     =	app.t
CONFIG       =	console warn_on debug
HEADERS      =	doxygen.h 
SOURCES      =	main.cpp 
unix:LIBS                  += -L../lib -ldoxygen -ldoxycfg -lqtools -lmd5 -lpthread
win32:INCLUDEPATH          += .
win32-mingw:LIBS           += -L../lib -ldoxygen -ldoxycfg -lqtools -lmd5 -lpthread
win32-msvc:LIBS            += qtools.lib md5.lib doxygen.lib doxycfg.lib shell32.lib iconv.lib
win32-msvc:TMAKE_LFLAGS    += /LIBPATH:..\lib
win32-borland:LIBS         += qtools.lib md5.lib doxygen.lib doxycfg.lib shell32.lib iconv.lib
win32-borland:TMAKE_LFLAGS += -L..\lib -L$(BCB)\lib\psdk
win32:TMAKE_CXXFLAGS       += -DQT_NODLL
win32-g++:LIBS             = -L../lib -ldoxygen -ldoxycfg -lqtools -lmd5 -liconv -lpthread
win32-g++:TMAKE_CXXFLAGS   += -fno-exceptions -fno-rtti
INCLUDEPATH                += ../qtools ../libmd5 .
DESTDIR                    =  ../bin
TARGET                     =  doxygen
unix:TARGETDEPS            =  ../lib/libdoxygen.a ../lib/libdoxycfg.a
win32:TARGETDEPS           =  ..\lib\doxygen.lib ..\lib\doxycfg.lib
win32-g++:TARGETDEPS       =  ../lib/libdoxygen.a ../lib/libdoxycfg.a
win32-mingw:TARGETDEPS     =  ../lib/libdoxygen.a ../lib/libdoxycfg.a
OBJECTS_DIR                =  ../objects

TMAKE_MOC = /usr/bin/moc
