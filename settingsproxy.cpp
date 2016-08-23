#include <stdexcept>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QMetaType>
#include <QVariant>

#include <NetworkManager/NetworkManager.h>

#include "settingsproxy.hpp"

SettingsProxy::SettingsProxy(QObject *parent) :
                              QObject(parent) { }

SettingsProxy::~SettingsProxy() { }

QStringList SettingsProxy::listConnections() {
  QDBusInterface iface(NM_DBUS_SERVICE,
                       NM_DBUS_PATH_SETTINGS,
                       NM_DBUS_IFACE_SETTINGS,
                       QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<QDBusArgument> reply = iface.call("ListConnections");

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusArgument dbusArg = reply.value();
  QStringList conns;
  dbusArg.beginArray();

  while (!dbusArg.atEnd()) {
    QDBusObjectPath connPath;
    dbusArg >> connPath;
    conns.append(connPath.path());
  }

  dbusArg.endArray();

  return conns;
}

QString SettingsProxy::addConnection(const ConnectionSettings &connSettings) {
  qDBusRegisterMetaType<ConnectionSettings>();
  qRegisterMetaType<ConnectionSettings>();

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
