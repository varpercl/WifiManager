#include "dbuswifi.h"
#include <QtDBus>
#include <QtDebug>
#include <QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
//#include <QtDBus/Q>
#include <QVariant>

typedef QMap<QString, QMap<QString, QVariant> > Connection;
Q_DECLARE_METATYPE(Connection)

DbusWifi::DbusWifi(QObject *parent) : QObject(parent)
{
    // Set DBUS
    nmDBus = "org.freedesktop.NetworkManager";
}

void DbusWifi::setDevice(QString dev){
    device = dev;
}

QStringList DbusWifi::getListAP(){
    qDebug()<<"INFO : Get List Access Point";
    QStringList accessPoints;
    QStringList accessPointsBSSID;

    QDBusInterface iface(nmDBus,
                         device,
                         nmDBus+".Device.Wireless",
                         QDBusConnection::systemBus());
    if(iface.isValid()){
        QDBusMessage query = iface.call("GetAccessPoints");
        if(query.type() == QDBusMessage::ReplyMessage)
        {

            QDBusArgument arg = query.arguments().at(0).value<QDBusArgument>();
            //qDebug()<<query.arguments();
            arg.beginArray();
            while(!arg.atEnd())
            {
                QString element = qdbus_cast<QString>(arg);
                QString elementBSSID = resolveBSSID(element);

                // To avoid duplicated APs due to NetworkManager's bug:
                //   https://bugs.launchpad.net/ubuntu/+source/network-manager/+bug/793960
                //   https://bugs.launchpad.net/ubuntu/+source/network-manager/+bug/891685
                if (!accessPointsBSSID.contains(elementBSSID)) {
                    accessPoints.append(element);
                    accessPointsBSSID.append(elementBSSID);
                }
            }
            arg.endArray();
        }else{
            qDebug()<< query.errorName() ;
            qDebug()<< query.errorMessage() ;
        }
    }else{
        qDebug()<<"ERROR : Interface invalid "<<__FILE__<<__LINE__;
    }


    return accessPoints;
}

QString DbusWifi::getActiveAP()
{
    qDebug() << " LLLLLLLLLLLLLLLLLL ::: " << device;
    QDBusInterface iface(nmDBus,
                         device,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    if(iface.isValid()) {
        QDBusMessage query = iface.call("Get", "org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint");
        if(query.type() == QDBusMessage::ReplyMessage) {
            QDBusObjectPath v = query.arguments().at(0).value<QDBusVariant>().variant().value
                    <QDBusObjectPath>();
            QString s = v.path();
            qDebug() << " ####### " << s;
            return s;
        }
    }

    return "";
}

QString DbusWifi::getConnections()
{


    QDBusInterface iface(nmDBus,
                         "/org/freedesktop/NetworkManager/Settings",
                         "org.freedesktop.DBus.Properties", // org.freedesktop.NetworkManager.Settings
                         QDBusConnection::systemBus());
    if(iface.isValid()) {
        qDebug() << " ~~~~~~~~~: " << "Valid iface";
        QDBusMessage query = iface.call("Get", "org.freedesktop.NetworkManager.Settings", "Connections");
        //    QDBusMessage query = iface.call("ListConnections");
        if(query.type() == QDBusMessage::ReplyMessage) {
            qDebug() << " ~~~~~~~~~: " << "Valid Call";
            QDBusArgument queryResult = query.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();

            queryResult.beginArray();
            qDebug() << " ~~~~~~~~~: " << "Begin Array";
            while (!queryResult.atEnd()) {
                QString c = qdbus_cast<QString>(queryResult);
                qDebug() << " $$$$$$$$$$$$$$$$$$$ " << c;
                connectionSettings(c);
            }
            queryResult.endArray();

            //      QString s = v.path();
            //      qDebug() << " $$$$$$$$$$$$$$$$$ " << s;
            //      return s;
        }
    }

    return "";
}

void DbusWifi::connectionSettings(QString c)
{
    QDBusInterface interface(nmDBus,
                             c,
                             "org.freedesktop.NetworkManager.Settings.Connection",
                             QDBusConnection::systemBus());

    QDBusInterface iface(nmDBus,
                         c, //"/org/freedesktop/NetworkManager/Settings",
                         "org.freedesktop.NetworkManager.Settings.Connection", // org.freedesktop.NetworkManager.Settings  org.freedesktop.DBus.Properties
                         QDBusConnection::systemBus()); // org.freedesktop.DBus.Properties

    if(iface.isValid()) {
        QList<QVariant> list;
        QDBusReply<Connection> query = iface.call("GetSettings"); //callWithCallback("GetSettings", list, this, SLOT(dumpMessage(QDBusMessage)));

        if(query.isValid()) {
            qDebug() << "Valid ##############################";
            qDebug() << "Valid ##############################";
            qDebug() << "Valid ##############################";
        }
    }
}

void DbusWifi::dumpMessage(QDBusMessage m)
{
    qDebug() << "*******: " << m.signature();
    if(m.type() == QDBusMessage::ReplyMessage) {

        QList<QVariant> args = m.arguments();

        foreach(QVariant arg, args) {
            QDBusArgument v = arg.value<QDBusVariant>().variant().value<QDBusArgument>();
            arg.type();
            qDebug() << v.currentType();
        }
    }
}

QString DbusWifi::resolveSSID(QString ap){
    qDebug()<<"INFO : Resolving SSID name for " + ap;
    QString SSID;
    QDBusInterface iface(nmDBus,
                         ap,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());

    if(iface.isValid()){
        QDBusMessage query = iface.call("Get","org.freedesktop.NetworkManager.AccessPoint","Ssid");

        if(query.type() == QDBusMessage::ReplyMessage){
            SSID = query.arguments().at(0).value<QDBusVariant>().variant().toByteArray().constData();
            qDebug()<<"INFO : The SSID is "+SSID;
        }
    }else{
        qDebug()<<"ERROR : Dbus iface invalid "<<__FILE__<<__LINE__;
    }
    return SSID;
}

QString DbusWifi::resolveBSSID(QString ap){
    qDebug()<<"INFO : Resolving BSSID name for " + ap;
    QString BSSID;
    QDBusInterface iface(nmDBus,
                         ap,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());

    if(iface.isValid()){
        QDBusMessage query = iface.call("Get","org.freedesktop.NetworkManager.AccessPoint","HwAddress");

        if(query.type() == QDBusMessage::ReplyMessage){
            BSSID = query.arguments().at(0).value<QDBusVariant>().variant().toString();
            qDebug()<<"INFO : The BSSID is "+BSSID;
        }
    }else{
        qDebug()<<"ERROR : Dbus iface invalid "<<__FILE__<<__LINE__;
    }
    return BSSID;
}

void DbusWifi::getAvailableConnectios()
{
    QDBusInterface iface(nmDBus,
                         device,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    if(iface.isValid()) {
        QDBusMessage query = iface.call("Get", "org.freedesktop.NetworkManager.Device",
                                        "AvailableConnections");
        if(query.type() == QDBusMessage::ReplyMessage) {
            QDBusArgument arg = query.arguments().at(0).value<QDBusArgument>();

            qDebug() << " ========== " << arg.currentType();
            availableConnections.clear();
            arg.beginArray();
            while(!arg.atEnd()) {
                QString c = qdbus_cast<QString>(arg);
                availableConnections << c;
                qDebug() << "::::::>> " << c;
            }
            qDebug() << "--------------------------";
        }
        else {
            qDebug() << "+++++++++++++++++++++++++++++";
        }
    }
}

DbusWifi::~DbusWifi()
{
    qDebug()<<this<<" Destroyed";
}

