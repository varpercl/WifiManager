#include <stdexcept>

#include <QtDBus>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QMetaType>

#include <NetworkManager/NetworkManager.h>
#include <dbus-1.0/dbus/dbus-shared.h>

#include "dbusnetwork.h"
#include "typedefs.h"
#include "activeconnectionproxy.h"
#include "deviceproxy.hpp"


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

QStringList DbusNetwork::getDevices() const {
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

QStringList DbusNetwork::getEthernetDevices() const {
  QStringList ethernetDevices;

  foreach (const QString &device, getDevices()) {
    if (getDeviceType(device) == NM_DEVICE_TYPE_ETHERNET) {
      ethernetDevices.append(device);
    }
  }

  return ethernetDevices;
}

QString DbusNetwork::getProperties(QString property){
}
QStringList DbusNetwork::getActiveConnections() const {
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

QStringList DbusNetwork::getEthernetActiveConnections() const {
  QStringList ethernetActiveConns;

  foreach (const QString &activeConn, getActiveConnections()) {
    if (getActiveConnectionType(activeConn) == "802-3-ethernet") {
      ethernetActiveConns.append(activeConn);
    }
  }

  return ethernetActiveConns;
}

QString DbusNetwork::getActiveConnectionType(const QString &activeConn) const {
  ActiveConnectionProxy activeConnProxy(activeConn);
  return activeConnProxy.getType();
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
int DbusNetwork::getDeviceType(const QString &dev) const {
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

QString DbusNetwork::getConnectionByUuid(const QString &uuid) const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH_SETTINGS,
                        NM_DBUS_IFACE_SETTINGS,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QDBusObjectPath> reply = iface.call("GetConnectionByUuid",
                                                 QVariant::fromValue(uuid));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value().path();
}

bool DbusNetwork::isNetworkingEnabled() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<bool> reply = iface.call("Get",
                                      QVariant(NM_DBUS_INTERFACE),
                                      QVariant("NetworkingEnabled"));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value();
}

void DbusNetwork::setNetworkingEnabled(bool enabled) const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH,
                        NM_DBUS_INTERFACE,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<void> reply = iface.call("Enable", QVariant::fromValue(enabled));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }
}

QString DbusNetwork::activateConnection(const QString &conn, const QString &device,
                                      const QString specific_object) const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH,
                        NM_DBUS_INTERFACE,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QDBusObjectPath> reply = iface.call("ActivateConnection",
                              QVariant::fromValue(QDBusObjectPath(conn)),
                              QVariant::fromValue(QDBusObjectPath(device)),
                              QVariant::fromValue(QDBusObjectPath(specific_object)));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value().path();
}

QString DbusNetwork::activateEthernetConnection(const QString &ethernetConn,
                                                const QString ethernetDevice) const {
  const QString finalEthernetDevice = ethernetDevice.isNull() ?
                                        getEthernetDevices().first() :
                                        ethernetDevice;
  return activateConnection(ethernetConn, finalEthernetDevice);
}

void DbusNetwork::deactivateConnection(const QString &activeConn) const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH,
                        NM_DBUS_INTERFACE,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<void> reply = iface.call("DeactivateConnection",
                              QVariant::fromValue(QDBusObjectPath(activeConn)));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }
}

void DbusNetwork::deactivateEthernetConnections() const {
  foreach (const QString &ethernetActiveConn, getEthernetActiveConnections()) {
    deactivateConnection(ethernetActiveConn);
  }
}

void DbusNetwork::disconnectEthernetDevices() const {
  foreach (const QString &ethernetDevice, getEthernetDevices()) {
    DeviceProxy(ethernetDevice).disconnect_();
  }
}

void DbusNetwork::ethernetDevicesSetAutoconnect(bool autoconnect) const {
  foreach (const QString &ethernetDevice, getEthernetDevices()) {
    DeviceProxy(ethernetDevice).setAutoconnect(autoconnect);
  }
}

QString DbusNetwork::createAutomaticEthernetConnection(const QString &uuid, const QString &id) const {
  qDBusRegisterMetaType<ConnectionSettings>();
  qRegisterMetaType<ConnectionSettings>();

  ConnectionSettings connSettings;

  connSettings["connection"]["uuid"] = uuid;
  connSettings["connection"]["id"] = id;
  connSettings["connection"]["type"] = "802-3-ethernet";

  connSettings["802-3-ethernet"];

  connSettings["ipv4"]["method"] = "auto";

  QDBusInterface iface(NM_DBUS_SERVICE,
                        NM_DBUS_PATH_SETTINGS,
                        NM_DBUS_IFACE_SETTINGS,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QDBusObjectPath> reply = iface.call("AddConnection",
                                                 QVariant::fromValue(connSettings));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value().path();
}

void DbusNetwork::_onPropertiesChanged(const QMap<QString, QVariant> properties) {
    QMap<QString, QVariant>::const_iterator it;

    it = properties.find("State");

    if (it != properties.end()) {
      emit stateChanged(it.value().value<int>());
    }

    it = properties.find("ActiveConnections");

    if (it != properties.end()) {
      QStringList activeConns;
      QDBusArgument arg = it.value().value<QDBusArgument>();

      arg.beginArray();
      while (!arg.atEnd()) {
        QDBusObjectPath activeConn;
        arg >> activeConn;
        activeConns.append(activeConn.path());
      }
      arg.endArray();

      emit activeConnectionsChanged(activeConns);
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

