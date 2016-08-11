#include <stdexcept>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QVariant>
#include <QDBusVariant>

#include <NetworkManager/NetworkManager.h>
#include <dbus-1.0/dbus/dbus-shared.h>

#include "deviceproxy.hpp"


DeviceProxy::DeviceProxy(const QString device, QObject *parent) :
                          QObject(parent),
                          _device(device) { }

DeviceProxy::~DeviceProxy() { }

bool DeviceProxy::getAutoconnect() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _device,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<bool> reply = iface.call("Get",
                                      QVariant(NM_DBUS_INTERFACE_DEVICE),
                                      QVariant("Autoconnect"));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  return reply.value();
}

void DeviceProxy::setAutoconnect(bool autoconnect) const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _device,
                        DBUS_INTERFACE_PROPERTIES,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<void> reply = iface.call("Set",
                                      QVariant(NM_DBUS_INTERFACE_DEVICE),
                                      QVariant("Autoconnect"),
                                      QVariant::fromValue(QDBusVariant(QVariant(autoconnect))));

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }
}

void DeviceProxy::disconnect_() const {
  QDBusInterface iface(NM_DBUS_SERVICE,
                        _device,
                        NM_DBUS_INTERFACE_DEVICE,
                        QDBusConnection::systemBus());

  if (!iface.isValid()) {
    const QDBusError err = iface.lastError();
    const QString errMsg = err.name() + ": " + err.message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QDBusReply<void> reply = iface.call("Disconnect");

  if (!reply.isValid()) {
    const QString errMsg = reply.error().name() + ": " + reply.error().message();
    throw std::runtime_error(errMsg.toUtf8().constData());
  }
}
