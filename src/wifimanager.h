#ifndef UTAMA_H
#define UTAMA_H

#include <QtWidgets/QMainWindow>
#include <dbusnetwork.h>
#include <dbuswifi.h>
#include <QMap>
#include <QString>
#include <QTimer>

#include <NetworkManager.h>

class ActiveConnectionProxy;

namespace Ui {
class WifiManager;
}

class WifiManager : public QMainWindow
{
    Q_OBJECT

public:
  explicit WifiManager(QWidget *parent = 0);

  ~WifiManager();

private:
  void _connectToWifiNetwork(const QString& bssid, QString password=QString());

private slots:
    void on_btnConnect_clicked();
    void on_ethernetSwitch_clicked(bool checked);
    void _onNetworkManagerStateChanged(const int newState);
    void onActiveConnectionsChanged(const QStringList &actives);
    void _onActiveWifiConnStateChanged(NMActiveConnectionState newState);

    void on_listAccessPoint_currentRowChanged(int currentRow);

    void updateUI();

public:
  static const QString ETHERNET_CONNECTION_UUID;
  static const QString ETHERNET_CONNECTION_ID;
  static const QString WIFI_CONNECTION_ID_PREFIX;

private:
  Ui::WifiManager *ui;
  DbusNetwork *debus;
  DbusWifi *wifi;
  ActiveConnectionProxy *_activeWifiConnProxy;

  QMap<QString /* SSID */, QString /* Access Point */> ssids;
};

#endif // UTAMA_H
