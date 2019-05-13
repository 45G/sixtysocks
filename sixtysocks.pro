TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

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
    proxifier/windowsupplicant.cc \
    core/streambuffer.cc \
    proxy/authserver.cc \
    core/tlscontext.cc \
    proxifier/tfocookiesupplicationagent.cc \
    proxifier/windowsupplicationagent.cc \
    core/stickreactor.cc \
    core/tls.cc \
    core/tlsexception.cc \
    core/readabledeferreactor.cc \
    core/tlslibrary.cc \
    external/nspr_stuff.c

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
    core/spinlock.hh \
    authentication/passwordchecker.hh \
    authentication/simplepasswordchecker.hh \
    core/uniqfd.hh \
    core/streambuffer.hh \
    proxifier/windowsupplicant.hh \
    authentication/syncedtokenstuff.h \
    proxy/authserver.hh \
    core/tlscontext.hh \
    core/rescheduleexception.hh \
    proxifier/tfocookiesupplicationagent.hh \
    proxifier/windowsupplicationagent.hh \
    core/stickreactor.hh \
    core/tls.hh \
    core/tlsexception.hh \
    core/socket.hh \
    core/tlslibrary.hh \
    core/readabledeferreactor.hh \
    external/nspr_stuff.h \
    external/nspr_stuff.h

# EDIT ME!
NSS_BUNDLE_ROOT = /home/vlad/nss-bundle/

NSS_ROOT        = /usr/include/nss3/
NSPR_ROOT       = /usr/include/nspr4/

INCLUDEPATH += $$NSS_ROOT
INCLUDEPATH += $$NSPR_ROOT

INCLUDEPATH += $$NSS_ROOT/lib/base
INCLUDEPATH += $$NSS_ROOT/lib/certdb
INCLUDEPATH += $$NSS_ROOT/lib/certhigh
INCLUDEPATH += $$NSS_ROOT/lib/ckfw
INCLUDEPATH += $$NSS_ROOT/lib/crmf
INCLUDEPATH += $$NSS_ROOT/lib/cryptohi
INCLUDEPATH += $$NSS_ROOT/lib/dbm
INCLUDEPATH += $$NSS_ROOT/lib/dev
INCLUDEPATH += $$NSS_ROOT/lib/freebl
INCLUDEPATH += $$NSS_ROOT/lib/jar
INCLUDEPATH += $$NSS_ROOT/lib/libpkix
INCLUDEPATH += $$NSS_ROOT/lib/nss
INCLUDEPATH += $$NSS_ROOT/lib/pk11wrap
INCLUDEPATH += $$NSS_ROOT/lib/pkcs12
INCLUDEPATH += $$NSS_ROOT/lib/pkcs7
INCLUDEPATH += $$NSS_ROOT/lib/pki
INCLUDEPATH += $$NSS_ROOT/lib/smime
INCLUDEPATH += $$NSS_ROOT/lib/softoken
INCLUDEPATH += $$NSS_ROOT/lib/sqlite
INCLUDEPATH += $$NSS_ROOT/lib/ssl
INCLUDEPATH += $$NSS_ROOT/lib/sysinit
INCLUDEPATH += $$NSS_ROOT/lib/util
INCLUDEPATH += $$NSS_ROOT/lib/zlib

INCLUDEPATH += $$NSPR_ROOT/Debug/dist/include/nspr
INCLUDEPATH += $$NSPR_ROOT/pr/include/nspr

LIBS += -L$$NSSROOT/dist/Debug/lib64/ -L$$NSSROOT/dist/Debug/lib/
LIBS += -lsocks6msg -lsocks6util -lpthread -lboost_system -lboost_filesystem -lboost_thread -lnspr4 -lnss3 -lssl3
