#include <stdexcept>

#include <QDBusInterface>
#include <QDBusError>
#include <QVariant>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusConnection>

#include <NetworkManager/NetworkManager.h>

#include "connectionproxy.hpp"


ConnectionProxy::ConnectionProxy(const QString conn, QObject *parent) :
                                  QObject(parent),
                                  _conn(conn) { }

ConnectionProxy::~ConnectionProxy() { }

ConnectionSettings ConnectionProxy::getSettings() const {
  qDBusRegisterMetaType<ConnectionSettings>();
  qRegisterMetaType<ConnectionSettings>();

  QDBusInterface iface(NM_DBUS_SERVICE,
                        _conn,
                        NM_DBUS_IFACE_SETTINGS_CONNECTION,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<ConnectionSettings> reply = iface.call("GetSettings");

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value();
}
