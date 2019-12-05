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
    proxifier/readabledeferreactor.cc \
    proxy/proxy.cc \
    proxy/proxyupstreamer.cc \
    proxy/resolver.cc \
    proxy/simpleproxydownstreamer.cc \
    proxy/connectproxydownstreamer.cc \
    sixtysocks.cc \
    authentication/passwordchecker.cc \
    authentication/simplepasswordchecker.cc \
    core/streambuffer.cc \
    proxy/authserver.cc \
    proxifier/tfocookiesupplicationagent.cc \
    core/stickreactor.cc \
    proxifier/sessionsupplicant.cc \
    proxifier/sessionsupplicationagent.cc \
    tls/tls.cc \
    tls/tlscontext.cc \
    tls/tlsexception.cc \
    tls/tlslibrary.cc

HEADERS += \
    core/poller.hh \
    core/listenreactor.hh \
    core/reactor.hh \
    core/streamreactor.hh \
    proxifier/proxifier.hh \
    proxifier/proxifierdownstreamer.hh \
    proxifier/proxifierupstreamer.hh \
    proxifier/readabledeferreactor.hh \
    proxy/proxy.hh \
    proxy/proxyupstreamer.hh \
    proxy/resolver.hh \
    proxy/simpleproxydownstreamer.hh \
    proxy/connectproxydownstreamer.hh \
    authentication/passwordchecker.hh \
    authentication/simplepasswordchecker.hh \
    core/uniqfd.hh \
    core/streambuffer.hh \
    authentication/syncedtokenstuff.h \
    proxy/authserver.hh \
    core/rescheduleexception.hh \
    proxifier/tfocookiesupplicationagent.hh \
    core/stickreactor.hh \
    core/socket.hh \
    proxifier/clientsession.hh \
    proxy/serversession.hh \
    proxifier/sessionsupplicant.hh \
    proxifier/sessionsupplicationagent.hh \
    tls/tls.hh \
    tls/tlscontext.hh \
    tls/tlsexception.hh \
    tls/tlslibrary.hh

NSS_ROOT        = /usr/include/nss3/
NSPR_ROOT       = /usr/include/nspr4/

INCLUDEPATH += $$NSS_ROOT
INCLUDEPATH += $$NSPR_ROOT

LIBS += -lsocks6msg -lsocks6util -lpthread -ltbb -lnspr4 -lnss3 -lssl3

DISTFILES += \
    README.md
