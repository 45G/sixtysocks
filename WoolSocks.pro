TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    fsm.cc \
    main.cc \
    poller.cc \
    proxifieracceptfsm.cc

HEADERS += \
    fsm.hh \
    poller.hh \
    proxifieracceptfsm.hh
