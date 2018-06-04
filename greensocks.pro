TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    proxifier/proxiferupstreamreactor.cc \
    proxifier/proxifieracceptreactor.cc \
    core/listenreactor.cc \
    core/poller.cc \
    core/reactor.cc \
    core/streamreactor.cc \
    greensocks.cc

HEADERS += \
    proxifier/proxifieracceptreactor.hh \
    proxifier/proxiferupstreamreactor.hh \
    core/poller.hh \
    core/listenreactor.hh \
    core/reactor.hh \
    core/streamreactor.hh

LIBS = -lsocks6msg -lsocks6util -lpthread
