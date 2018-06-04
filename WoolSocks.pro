TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    main.cc \
    poller.cc \
    reactor.cc \
    proxifieracceptreactor.cc \
    listenreactor.cc \
    streamreactor.cc \
    proxiferupstreamreactor.cc

HEADERS += \
    poller.hh \
    reactor.hh \
    proxifieracceptreactor.hh \
    listenreactor.hh \
    proxiferupstreamreactor.hh \
    streamreactor.hh

LIBS = -lsocks6msg -lsocks6util
