#ifndef UTAMA_H
#define UTAMA_H

#include <QtWidgets/QMainWindow>
#include <dbusnetwork.h>
#include <dbuswifi.h>
#include <QMap>
#include <QString>

#include <NetworkManager.h>

class ActiveConnectionProxy;

namespace Ui {
class Utama;
}

class Utama : public QMainWindow
{
    Q_OBJECT

public:
  explicit Utama(QWidget *parent = 0);
  void updateUI();

  ~Utama();

private:
  void _connectToWifiNetwork(const QString& bssid, QString password=QString());

private slots:
    void on_pushButton_clicked();
    void on_ethernetSwitch_clicked(bool checked);
    void _onNetworkManagerStateChanged(const int newState);
    void _onActiveWifiConnStateChanged(NMActiveConnectionState newState);

public:
  static const QString ETHERNET_CONNECTION_UUID;
  static const QString ETHERNET_CONNECTION_ID;
  static const QString WIFI_CONNECTION_ID_PREFIX;

private:
  Ui::Utama *ui;
  DbusNetwork *debus;
  DbusWifi *wifi;
  ActiveConnectionProxy *_activeWifiConnProxy;

  QMap<QString /* SSID */, QString /* Access Point */> ssids;
};

#endif // UTAMA_H
