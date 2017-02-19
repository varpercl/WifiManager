QT       += core gui dbus widgets

TARGET = raspifi
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += /usr/include/NetworkManager \
    /usr/include/dbus-1.0

SOURCES += main.cpp\
    dbusnetwork.cpp \
    dbuswifi.cpp \
    activeconnectionproxy.cpp \
    connectionproxy.cpp \
    deviceproxy.cpp \
    settingsproxy.cpp \
    passworddialog.cpp \
    wifimanager.cpp

HEADERS  += \
    dbusnetwork.h \
    dbuswifi.h \
    typedefs.h \
    activeconnectionproxy.h \
    passworddialog.h \
    deviceproxy.h \
    wifimanager.h \
    connectionproxy.h \
    settingsproxy.h

FORMS    += \
    passwordddialog.ui \
    wifimanager.ui
