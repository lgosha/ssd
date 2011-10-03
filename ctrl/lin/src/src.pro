QT		+=	network
TEMPLATE	 =	app
CONFIG		+=	qt warn_on release qtestlib
TARGET		 =	../bin/ctrl
DEPENDPATH	+=	.
INCLUDEPATH	+=	.

OBJECTS_DIR		= ../tmp
MOC_DIR			= ../tmp
UI_DIR			= ../tmp

LIBS		+=	-lxerces-c

HEADERS 	+=	ctrl.h \
        		exception.h \
        		ulog.h \
        		xmlconfig.h

FORMS 		+=	ctrl.ui

SOURCES 	+=	ctrl.cpp \
        		exception.cpp \
        		main.cpp \
        		ulog.cpp \
        		xmlconfig.cpp

