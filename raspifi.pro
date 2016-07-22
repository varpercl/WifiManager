QT       += core gui dbus widgets

TARGET = raspifi
TEMPLATE = app

SOURCES += main.cpp\
        utama.cpp \
    dbusnetwork.cpp \
    dbuswifi.cpp

HEADERS  += utama.h \
    dbusnetwork.h \
    dbuswifi.h

FORMS    += utama.ui
