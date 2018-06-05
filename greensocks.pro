TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    core/listenreactor.cc \
    core/poller.cc \
    core/reactor.cc \
    core/streamreactor.cc \
    greensocks.cc \
    proxifier/proxiferupstreamer.cc \
    proxifier/proxifier.cc \
    proxifier/proxifierdownstreamer.cc

HEADERS += \
    core/poller.hh \
    core/listenreactor.hh \
    core/reactor.hh \
    core/streamreactor.hh \
    proxifier/proxifier.hh \
    proxifier/proxiferupstreamer.hh \
    proxifier/proxifierdownstreamer.hh

LIBS = -lsocks6msg -lsocks6util -lpthread
