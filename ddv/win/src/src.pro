QT += network
TEMPLATE = app
CONFIG += qt \
    release
TARGET = ../bin/ddv
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = ../tmp
MOC_DIR = ../tmp
UI_DIR = ../tmp
LIBS += -lxerces-c
HEADERS += udp_client.h \
    udp_datetime.h \
    udp_panswer.h \
    udp_pquery.h \
    udp_pvalue.h \
    udp_pvar.h \
    xmlconfig.h \
    ulog.h \
    exception.h \
    mwnd.h
SOURCES += main.cpp \
    udp_client.cpp \
    udp_panswer.cpp \
    udp_pquery.cpp \
    udp_pvalue.cpp \
    udp_pvar.cpp \
    xmlconfig.cpp \
    ulog.cpp \
    exception.cpp \
    mwnd.cpp

