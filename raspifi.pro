QT       += core gui dbus widgets

TARGET = raspifi
TEMPLATE = app

SOURCES += main.cpp\
        utama.cpp \
    dbusnetwork.cpp \
    dbuswifi.cpp \
    activeconnectionproxy.cpp \
    connectionproxy.cpp \
    deviceproxy.cpp

HEADERS  += utama.h \
    dbusnetwork.h \
    dbuswifi.h \
    typedefs.h \
    activeconnectionproxy.h \
    connectionproxy.hpp \
    deviceproxy.hpp

FORMS    += utama.ui
