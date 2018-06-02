TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    main.cc \
    poller.cc \
    streamreadreactor.cc \
    reactor.cc \
    proxifieracceptreactor.cc

HEADERS += \
    poller.hh \
    reactor.hh \
    proxifieracceptreactor.hh \
    streamreadreactor.hh
