QT		+=	network
TEMPLATE	 =	app
CONFIG		+=	qt warn_on release
TARGET		 =	../bin/dac
DEPENDPATH	+=	.
INCLUDEPATH	+=	.

OBJECTS_DIR		= ../tmp
MOC_DIR			= ../tmp
UI_DIR			= ../tmp

LIBS		+= -lnetsnmp -lxerces-c

HEADERS 	+=	cevent.h \
        		datathread_snmp.h \
        		exception.h \
        		mwnd.h \
        		parameter_abstract.h \
			parameter_enum.h \
			parameter_creator.h \
			parameter_info.h \
			parameter_info_creator.h \
        		parameter_type_delayed.h \
        		parameter_type_group.h \
        		parameter_type_integrated.h \
        		parameter_type_template.h \
        		udp_datetime.h \
        		udp_panswer.h \
        		udp_pquery.h \
        		udp_pvalue.h \
        		udp_pvar.h \
        		udp_server.h \
        		ulog.h \
        		ulog_director.h \
        		xmlconfig.h

FORMS 		+=	mwnd.ui

SOURCES 	+=	cevent.cpp \
        		datathread_snmp.cpp \
        		exception.cpp \
        		parameter_creator.cpp \
        		parameter_info.cpp \
        		parameter_info_creator.cpp \
        		main.cpp \
        		mwnd.cpp \
        		udp_panswer.cpp \
        		udp_pquery.cpp \
        		udp_pvalue.cpp \
        		udp_pvar.cpp \
        		udp_server.cpp \
        		ulog.cpp \
        		ulog_director.cpp \
        		xmlconfig.cpp

