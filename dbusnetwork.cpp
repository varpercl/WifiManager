#include "dbusnetwork.h"
#include <stdexcept>
#include <QtDBus>
#include <QDebug>
#include <QDBusConnection>

DbusNetwork::DbusNetwork(QObject *parent) :
    QObject(parent),
    nmService("org.freedesktop.NetworkManager"),
    nmObject("/org/freedesktop/NetworkManager"),
    nmInterface("org.freedesktop.NetworkManager")
{
    // Connect to NetworkManager's signals

    const QString signal = "PropertiesChanged";

    bool connected = QDBusConnection::systemBus().connect(
        nmService,
        nmObject,
        nmInterface,
        signal,
        "a{sv}",
        this,
        SLOT(_onPropertiesChanged(QMap<QString,QVariant>)));

    if (!connected) {
        const QString msg = "Could not connect to signal '" + signal +
                            "' of interface '" + nmInterface + "'";
        throw std::runtime_error(msg.toUtf8().constData());
    }
}

QStringList DbusNetwork::getDevices(){
    /*
     * Interface : org.freedesktop.NetworkManager
     * Method    : GetDevices
     */
    qDebug()<<"INFO : Get Devices Lists";
    QDBusInterface busIface(nmService,
                            nmObject,
                            nmInterface,
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

    QDBusInterface dbusIface(nmService,
                             nmObject,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QStringList connections;
    if(dbusIface.isValid()){
        QDBusMessage queryConnection = dbusIface.call("Get",
                                                    nmService,
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
int DbusNetwork::getStatus() const {
    qDebug()<<"INFO : Get Network status.. ";

    QDBusInterface iface(nmService,
                         nmObject,
                         nmInterface,
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

    QDBusInterface iface(nmService,
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
  QDBusInterface busIface(nmService,
                          nmObject,
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

void DbusNetwork::_onPropertiesChanged(const QMap<QString, QVariant> properties) {
    QMap<QString, QVariant>::const_iterator it = properties.find("State");

    if (it != properties.end()) {
      emit stateChanged(it.value().value<int>());
    }
}

DbusNetwork::~DbusNetwork()
{
    // Disconnect from NetworkManager's signals

    const QString signal = "PropertiesChanged";

    bool disconnected = QDBusConnection::systemBus().disconnect(
        nmService,
        nmObject,
        nmInterface,
        signal,
        "a{sv}",
        this,
        SLOT(_onPropertiesChanged(QMap<QString,QVariant>)));

    if (!disconnected) {
        const QString msg = "Could not disconnect from signal '" + signal +
                            "' of interface '" + nmInterface + "'";
        qWarning() << msg.toUtf8().constData();
    }

    qDebug()<<this<<" Destroyed";
}

