TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
    core/listenreactor.cc \
    core/poller.cc \
    core/reactor.cc \
    core/streamreactor.cc \
    proxifier/proxifier.cc \
    proxifier/proxifierdownstreamer.cc \
    proxifier/proxifierupstreamer.cc \
    proxy/proxy.cc \
    proxy/proxyupstreamer.cc \
    proxy/simpleproxydownstreamer.cc \
    proxy/connectproxydownstreamer.cc \
    sixtysocks.cc \
    authentication/passwordchecker.cc \
    authentication/simplepasswordchecker.cc \
    core/streambuffer.cc \
    proxy/authserver.cc \
    proxifier/tfocookiesupplicationagent.cc \
    core/stickreactor.cc \
    core/tls.cc \
    core/tlsexception.cc \
    core/readabledeferreactor.cc \
    core/tlslibrary.cc \
    external/nspr_stuff.c \
    proxifier/sessionsupplicant.cc \
    proxifier/sessionsupplicationagent.cc

HEADERS += \
    core/poller.hh \
    core/listenreactor.hh \
    core/reactor.hh \
    core/streamreactor.hh \
    proxifier/proxifier.hh \
    proxifier/proxifierdownstreamer.hh \
    proxifier/proxifierupstreamer.hh \
    proxy/proxy.hh \
    proxy/proxyupstreamer.hh \
    proxy/simpleproxydownstreamer.hh \
    proxy/connectproxydownstreamer.hh \
    authentication/passwordchecker.hh \
    authentication/simplepasswordchecker.hh \
    core/uniqfd.hh \
    core/streambuffer.hh \
    authentication/syncedtokenstuff.h \
    proxy/authserver.hh \
    core/tlscontext.hh \
    core/rescheduleexception.hh \
    proxifier/tfocookiesupplicationagent.hh \
    core/stickreactor.hh \
    core/tls.hh \
    core/tlsexception.hh \
    core/socket.hh \
    core/tlslibrary.hh \
    core/readabledeferreactor.hh \
    external/nspr_stuff.h \
    external/nspr_stuff.h \
    proxifier/clientsession.hh \
    proxy/serversession.hh \
    proxifier/sessionsupplicant.hh \
    proxifier/sessionsupplicationagent.hh

NSS_ROOT        = /usr/include/nss3/
NSPR_ROOT       = /usr/include/nspr4/

INCLUDEPATH += $$NSS_ROOT
INCLUDEPATH += $$NSPR_ROOT

LIBS += -lsocks6msg -lsocks6util -lpthread -ltbb -lnspr4 -lnss3 -lssl3

DISTFILES += \
    README.md
