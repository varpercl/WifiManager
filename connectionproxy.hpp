#ifndef CONNECTIONPROXY_HPP
#define CONNECTIONPROXY_HPP

#include <QObject>
#include <QString>

#include "typedefs.h"

class ConnectionProxy : public QObject {
Q_OBJECT
public:
  explicit ConnectionProxy(const QString conn, QObject *parent = 0);
  ~ConnectionProxy();

  ConnectionSettings getSettings() const;

signals:

public slots:

private:
  const QString _conn;
};

#endif // CONNECTIONPROXY_HPP
