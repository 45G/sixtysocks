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
    core/strikecounter.cc

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
    core/strikecounter.hh

INCLUDEPATH += /usr/local/wolfssl/include

LIBS += -L/usr/local/wolfssl/lib64 -lsocks6msg -lsocks6util -lpthread -lboost_system -lboost_filesystem -lwolfssl
