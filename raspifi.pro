QT       += core gui dbus widgets

TARGET = raspifi
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += /usr/include/NetworkManager \
    /usr/include/dbus-1.0

SOURCES += main.cpp\
        utama.cpp \
    dbusnetwork.cpp \
    dbuswifi.cpp \
    activeconnectionproxy.cpp \
    connectionproxy.cpp \
    deviceproxy.cpp \
    settingsproxy.cpp \
    passworddialog.cpp

HEADERS  += utama.h \
    dbusnetwork.h \
    dbuswifi.h \
    typedefs.h \
    activeconnectionproxy.h \
    connectionproxy.hpp \
    deviceproxy.hpp \
    settingsproxy.hpp \
    passworddialog.h

FORMS    += utama.ui \
    passwordddialog.ui
