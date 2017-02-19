#ifndef ACTIVECONNECTIONPROXY_H
#define ACTIVECONNECTIONPROXY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>

#include <NetworkManager.h>

class ActiveConnectionProxy : public QObject {
Q_OBJECT
public:
  explicit ActiveConnectionProxy(const QString activeConn, QObject *parent = 0);
  ~ActiveConnectionProxy();

  QString getConnection() const;
  QString getUuid() const;
  QString getType() const;
  NMActiveConnectionState getState() const;

signals:

  /**
   * @brief Emitted when the org.freedesktop.NetworkManager.Connection.Active's "State" property changed
   * @param newState the new state value
   *
   * For information on the possible values and meaning of "newState" see
   * https://developer.gnome.org/NetworkManager/1.2/nm-dbus-types.html#NMActiveConnectionState
   */
  void stateChanged(NMActiveConnectionState newState);

private slots:
  void _onPropertiesChanged(const QMap<QString, QVariant> properties);

private:
  const QString _activeConn;
};

#endif // ACTIVECONNECTIONPROXY_H
