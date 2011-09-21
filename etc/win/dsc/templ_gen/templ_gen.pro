TEMPLATE	 =	app
CONFIG		+=	qt warn_on
TARGET		 =	templ_gen.bin
DEPENDPATH	+=	.
INCLUDEPATH	+=	.

LIBS		+= 	-lxerces-c

HEADERS 	+=	xmlconfig.h \
			exception.h

SOURCES 	+=	main.cpp \
        		xmlconfig.cpp \
        		exception.cpp
