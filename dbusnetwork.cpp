#include "dbusnetwork.h"
#include <QtDBus>
#include <QDebug>
#include <QDBusConnection>

DbusNetwork::DbusNetwork(QObject *parent) : QObject(parent)
{
    // Set DBUS
    nmDBus = "org.freedesktop.NetworkManager";
}

QStringList DbusNetwork::getDevices(){
    /*
     * Interface : org.freedesktop.NetworkManager
     * Method    : GetDevices
     */
    qDebug()<<"INFO : Get Devices Lists";
    QDBusInterface busIface(nmDBus,
                            "/org/freedesktop/NetworkManager",
                            nmDBus,
                            QDBusConnection::systemBus());
    QStringList devicesList;
    if(busIface.isValid()){
        QDBusMessage dbusQuery = busIface.call("GetDevices");

        if( dbusQuery.type() == QDBusMessage::ReplyMessage ){
            QDBusArgument queryResult = dbusQuery
                    .arguments()
                    .at(0)
                    .value<QDBusArgument>();
            queryResult.beginArray();
            while (!queryResult.atEnd()) {
               QString device = qdbus_cast<QString>(queryResult);
               devicesList.append(device);
            }
            queryResult.endArray();
        } //endif dbusquery type
    }else{
        qDebug()<<"ERROR : DBUS Interface not valid "<<__FILE__<<__LINE__;
    }

    // Return the results
    return devicesList;
}
QString DbusNetwork::getProperties(QString property){
}
QStringList DbusNetwork::getActiveConnection(){
    qDebug()<<"INFO : Get active connection ...";

    QDBusInterface dbusIface(nmDBus,
                             "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QStringList connections;
    if(dbusIface.isValid()){
        QDBusMessage queryConnection = dbusIface.call("Get",
                                                    nmDBus,
                                                    "ActiveConnections");
        if(queryConnection.type() == QDBusMessage::ReplyMessage){
            QDBusArgument result = queryConnection
                    .arguments()
                    .at(0)
                    .value<QDBusVariant>()
                    .variant()
                    .value<QDBusArgument>();
            result.beginArray();
            while(!result.atEnd()){
                QString connection = qdbus_cast<QString>(result);
                connections.append(connection);
            }
            result.endArray();


        }
    }else{
        qDebug()<<"ERROR: DBUS Interface invalid";
    }

    return connections;
}
int DbusNetwork::getStatus(){
    qDebug()<<"INFO : Get Network status.. ";

    QDBusInterface iface(nmDBus,
                         "/org/freedesktop/NetworkManager",
                         nmDBus,
                         QDBusConnection::systemBus());
    int status = 0;

    if(iface.isValid()){
        QDBusMessage query = iface.call("CheckConnectivity");

        if(query.type() == QDBusMessage::ReplyMessage){
             status = query.arguments().at(0).toInt();
        }
    }else{
        qDebug()<<"ERROR : Dbus iface invalid "<<__FILE__<<__LINE__;
    }
    return status;
}
int DbusNetwork::getDeviceType(QString dev){
    /*
     * 0 Unknown
     * 1 Ethernet
     * 2 Wifi
     */
    qDebug()<<"INFO : Get Device type";

    QDBusInterface iface(nmDBus,
                         dev,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    int type = 0;

    if(iface.isValid()){
        QDBusMessage query = iface.call("Get","org.freedesktop.NetworkManager.Device","DeviceType");

        if(query.type() == QDBusMessage::ReplyMessage){
             type = query.arguments().at(0).value<QDBusVariant>().variant().toInt();
             qDebug()<<"INFO : Device type is "+QString::number(type);
        }
    }else{
        qDebug()<<"ERROR : Dbus iface invalid "<<__FILE__<<__LINE__;
    }
    return type;
}

QStringList DbusNetwork::getConnections()
{
  QDBusInterface busIface(nmDBus,
                          "/org/freedesktop/NetworkManager",
                          "org.freedesktop.DBus.Properties", //"org.freedesktop.NetworkManager.Settings",
                          QDBusConnection::systemBus());
  QStringList devicesList;
  if(busIface.isValid()){
      QDBusMessage dbusQuery = busIface.call("Get", "org.freedesktop.NetworkManager", "Devices");
      // busIface.call("ListConnections");

      if( dbusQuery.type() == QDBusMessage::ReplyMessage ){
          QDBusArgument queryResult = dbusQuery
                  .arguments()
                  .at(0)
                  .value<QDBusArgument>();
          queryResult.beginArray();
          while (!queryResult.atEnd()) {
             QString device = qdbus_cast<QString>(queryResult);
             devicesList.append(device);
             qDebug() << "%%%%%%%%%%%%%% " << device;
          }
          queryResult.endArray();
      } //endif dbusquery type
  }else{
      qDebug()<<"ERROR : DBUS Interface not valid "<<__FILE__<<__LINE__;
  }

  // Return the results
  return devicesList;
}

DbusNetwork::~DbusNetwork()
{
    qDebug()<<this<<" Destroyed";
}

