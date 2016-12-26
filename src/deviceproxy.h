#ifndef DEVICEPROXY_HPP
#define DEVICEPROXY_HPP

#include <QObject>
#include <QString>

class DeviceProxy : public QObject {
Q_OBJECT
public:
  explicit DeviceProxy(const QString device, QObject *parent = 0);
  ~DeviceProxy();

  bool getAutoconnect() const;
  void setAutoconnect(bool autoconnect) const;
  void disconnect_() const;

signals:

public slots:

private:
  const QString _device;
};

#endif // DEVICEPROXY_HPP
