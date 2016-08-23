#ifndef SETTINGSPROXY_HPP
#define SETTINGSPROXY_HPP

#include <QStringList>
#include <QString>
#include <QObject>

#include "typedefs.h"


class SettingsProxy : public QObject {
Q_OBJECT
public:
  explicit SettingsProxy(QObject *parent = 0);
  ~SettingsProxy();

  static QStringList listConnections();
  static QString addConnection(const ConnectionSettings &connSettings);

signals:

public slots:

};

#endif // SETTINGSPROXY_HPP
