SUBDIRS = .
INCLUDES = -I$(apache2_prefix)/include
LDADD = -lpthread -ldl
lib_LTLIBRARIES =  libmodfastcpp.la
libmodfastcpp_la_SOURCES =  mod_fastcpp.cxx \
Lock.cxx  Mutex.cxx FCException.cxx FCParser.cxx FCPage.cxx \
FastCPP.cxx FCPageFile.cxx FCRequest.cxx FCSession.cxx FCManager.cxx \
FCSessionManager.cxx

fastcpp_includedir = $(pkgincludedir)

fastcpp_include_DATA = \
	FCException.hxx\
	FCPage.hxx\
	FCParser.hxx\
	FCSession.hxx\
        FCThread.hxx\
	Lock.hxx\
	Mutex.hxx\
	FCManager.hxx\
	FCPageFile.hxx\
	FCRequest.hxx\
	FCSessionManager.hxx\
	FastCPP.hxx\
	LruList.hxx

EXTRA_DIST = $(fastcpp_include_DATA)
install-libLTLIBRARIES:
	cp .libs/libmodfastcpp.so $(apache2_prefix)/modules/mod_fastcpp.so