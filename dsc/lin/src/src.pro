QT		+=	network
TEMPLATE	 =	app
CONFIG		+=	qt warn_on release
TARGET		 =	../bin/dsc
DEPENDPATH	+=	.
INCLUDEPATH	+=	.

OBJECTS_DIR	 =	../tmp
MOC_DIR		 =	../tmp
UI_DIR		 =	../tmp

LIBS		+=	-lxerces-c

HEADERS 	+=	udp_client.h			\
			udp_server.h			\
			udp_datetime.h			\
			udp_panswer.h			\
			udp_pquery.h			\
			udp_pvalue.h			\
			udp_pvar.h			\
			udp_server_info.h 		\
			xmlconfig.h			\
			cevent.h			\
			ulog.h				\
			ulog_director.h			\
			exception.h			\
			parameter_info.h		\
			parameter_abstract.h		\
			parameter_enum.h		\
			parameter_info_creator.h	\
			parameter_creator.h		\
			parameter_type_template.h

SOURCES 	+=	main.cpp			\
			udp_client.cpp			\
			udp_server.cpp			\
			udp_panswer.cpp			\
			udp_pquery.cpp			\
			udp_pvalue.cpp			\
			udp_pvar.cpp			\
			udp_server_info.cpp		\
			xmlconfig.cpp			\
			cevent.cpp			\
			ulog.cpp			\
			exception.cpp			\
			ulog_director.cpp		\
			parameter_info.cpp		\
			parameter_info_creator.cpp	\
			parameter_creator.cpp
