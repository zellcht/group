#
# This file was generated from qtools.pro.in on Fri Feb 24 16:02:34 CET 2012
#

TEMPLATE	= lib
CONFIG		= warn_on staticlib debug
HEADERS		= qarray.h \
		  qbuffer.h \
                  qcache.h \
                  qgcache.h \
		  qcollection.h \
		  qconfig.h \
		  qcstring.h \
		  scstring.h \
		  qdatastream.h \
		  qdatetime.h \
		  qdict.h \
		  qdir.h \
		  qfeatures.h \
		  qfile.h \
		  qfiledefs_p.h \
		  qfileinfo.h \
		  qgarray.h \
		  qfeatures.h \
		  qgdict.h \
		  qgeneric.h \
		  qglist.h \
		  qglobal.h \
		  qgstring.h \
		  qgvector.h \
		  qintdict.h \
		  qiodevice.h \
		  qlist.h \
		  qptrdict.h \
		  qqueue.h \
		  qregexp.h \
		  qshared.h \
		  qsortedlist.h \
		  qstack.h \
		  qstring.h \
		  qstringlist.h \
		  qstrlist.h \
		  qstrvec.h \
		  qtextstream.h \
		  qtl.h \
		  qvaluelist.h \
		  qvector.h \
                  qxml.h \
                  qvaluestack.h \
                  qmap.h \
		  qmodules.h \
		  qthread.h \
		  qthread_p.h \
		  qmutex.h \
		  qmutex_p.h \
                  qutfcodec.h \
		  qwaitcondition.h

SOURCES		= qbuffer.cpp \
		  qcollection.cpp \
		  scstring.cpp \
		  qdatastream.cpp \
		  qdatetime.cpp \
		  qdir.cpp \
		  qfile.cpp \
		  qfileinfo.cpp \
		  qgarray.cpp \
		  qgcache.cpp \
		  qgdict.cpp \
		  qglist.cpp \
		  qglobal.cpp \
		  qgstring.cpp \
		  qgvector.cpp \
		  qiodevice.cpp \
		  qregexp.cpp \
		  qstring.cpp \
                  qtextstream.cpp \
		  qtextcodec.cpp \
		  qstringlist.cpp \
	          qxml.cpp \
                  qmap.cpp \
		  qthread.cpp \
		  qmutex.cpp \
                  qutfcodec.cpp

unix:SOURCES +=   qfile_unix.cpp \
		  qdir_unix.cpp \
		  qfileinfo_unix.cpp \
		  qthread_unix.cpp \
		  qmutex_unix.cpp \
		  qwaitcondition_unix.cpp

win32:SOURCES +=  qfile_win32.cpp \
		  qdir_win32.cpp \
		  qfileinfo_win32.cpp \
		  qthread_win32.cpp \
		  qmutex_win32.cpp \
		  qwaitcondition_win32.cpp

INCLUDEPATH = .
#TMAKE_CXXFLAGS += -DQT_NO_CODECS -DQT_LITE_UNICODE
TMAKE_CXXFLAGS += -DQT_LITE_UNICODE
win32:TMAKE_CXXFLAGS += -DQT_NODLL
win32-g++:TMAKE_CXXFLAGS += -D__CYGWIN__ -DALL_STATIC
OBJECTS_DIR = ../objects
DESTDIR = ../lib
TMAKE_MOC = /usr/bin/moc
