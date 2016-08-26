#include <stdexcept>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusVariant>

#include <dbus/dbus-shared.h>

#include "activeconnectionproxy.h"
#include "connectionproxy.hpp"
#include "typedefs.h"


ActiveConnectionProxy::ActiveConnectionProxy(const QString activeConn, QObject *parent) :
                                              QObject(parent),
                                              _activeConn(activeConn) {
  // Connect to org.freedesktop.NetworkManager.Connection.Active's signals

  const QString signal = "PropertiesChanged";

  bool connected = QDBusConnection::systemBus().connect(
    NM_DBUS_SERVICE,
    _activeConn,
    NM_DBUS_INTERFACE_ACTIVE_CONNECTION,
    signal,
    "a{sv}",
    this,
    SLOT(_onPropertiesChanged(QMap<QString,QVariant>)));

  if (!connected) {
    const QString msg = "Could not connect to signal '" + signal +
                        "' of interface '" + NM_DBUS_INTERFACE_ACTIVE_CONNECTION + "'";
    throw std::runtime_error(msg.toUtf8().constData());
  }
}

ActiveConnectionProxy::~ActiveConnectionProxy() {
  // Disconnect from org.freedesktop.NetworkManager.Connection.Active's signals

  const QString signal = "PropertiesChanged";

  bool disconnected = QDBusConnection::systemBus().disconnect(
    NM_DBUS_SERVICE,
    _activeConn,
    NM_DBUS_INTERFACE_ACTIVE_CONNECTION,
    signal,
    "a{sv}",
    this,
    SLOT(_onPropertiesChanged(QMap<QString,QVariant>)));

  if (!disconnected) {
    // Ignored because underlying org.freedesktop.NetworkManager.Connection.Active
    // may be removed.
  }
}

QString ActiveConnectionProxy::getConnection() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _activeConn,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QDBusVariant> reply = iface.call("Get",
                                QVariant(NM_DBUS_INTERFACE_ACTIVE_CONNECTION),
                                QVariant("Connection"));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value().variant().value<QDBusObjectPath>().path();
}

QString ActiveConnectionProxy::getUuid() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _activeConn,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QString> reply = iface.call("Get",
                                      QVariant(NM_DBUS_INTERFACE_ACTIVE_CONNECTION),
                                      QVariant("Uuid"));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value();
}

QString ActiveConnectionProxy::getType() const {
  QString conn = getConnection();
  ConnectionProxy connProxy(conn);
  ConnectionSettings connSettings = connProxy.getSettings();
  return connSettings["connection"]["type"].toString();
}

NMActiveConnectionState ActiveConnectionProxy::getState() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _activeConn,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<uint> reply = iface.call("Get",
                                      QVariant(NM_DBUS_INTERFACE_ACTIVE_CONNECTION),
                                      QVariant("State"));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return (NMActiveConnectionState) reply.value();
}

void ActiveConnectionProxy::_onPropertiesChanged(const QMap<QString, QVariant> properties) {
  QMap<QString, QVariant>::const_iterator it;

  it = properties.find("State");

  if (it != properties.end()) {
    emit stateChanged((NMActiveConnectionState) it.value().value<uint>());
  }
}
