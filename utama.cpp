#include <stdexcept>

#include <QDebug>
#include <QDBusInterface>
#include <QMap>
#include <QVariant>
#include <QDBusMessage>
#include <QMetaObject>
#include <QObject>
#include <QListWidgetItem>
#include <Qt>

#include "utama.h"
#include "ui_utama.h"
#include "dbusnetwork.h"
#include "activeconnectionproxy.h"
#include "passworddialog.h"
#include "connectionproxy.hpp"
#include "typedefs.h"


const QString Utama::ETHERNET_CONNECTION_UUID = "e1ea3417-7c4b-4008-9406-51b9221c37a8";
const QString Utama::ETHERNET_CONNECTION_ID = "Automatic Ethernet Connection";
const QString Utama::WIFI_CONNECTION_ID_PREFIX = "Automatic Wifi Connection";

Utama::Utama(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Utama),
  _activeWifiConnProxy(nullptr)
{
  ui->setupUi(this);

  debus = new DbusNetwork();
  wifi = new DbusWifi();

  // Connect signals
  QObject::connect(debus, SIGNAL(stateChanged(int)), this, SLOT(_onNetworkManagerStateChanged(int)));

  updateUI();
  ui->ethernetSwitch->click();  // simulate user click to connect to ethernet
//  qDebug() << debus->getDevices();
//  qDebug()<<debus->getActiveConnections();
//  qDebug()<<debus->getStatus();
//  qDebug()<<debus->getDeviceType("/org/freedesktop/NetworkManager/Devices/0");
}

void Utama::updateUI(){
  int status = debus->getStatus();

  if(status < 3){
    // No active connection / not connected
    ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #F44336; border-radius: 15px}");
    ui->statusLabel->setText("Disconnected..");
  }else{
    // Connected.
    ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #4CAF50; border-radius: 15px}");
    ui->statusLabel->setText("You are connected to the Network");
  }

  // Get wifi devices
  QStringList devices = debus->getDevices();
  QString wifiDev = "";
  foreach(QString device, devices){
    if(debus->getDeviceType(device) == 2){
      qDebug()<<"INFO : Get wifi devices on "+device;
      wifiDev = device;
      break;
    }
  }

  // Get scanned wifi SSID
  wifi->setDevice(wifiDev);
  QStringList listAp = wifi->getListAP();
  QStringList listSSID;
  ssids.clear();
  ui->listAccessPoint->clear();
  foreach(QString ap, listAp){
    QString resolved = wifi->resolveSSID(ap);
    listSSID.append(resolved);
    ssids.insert(resolved, ap);
    // Tampilkan ke UI
    QListWidgetItem *item = new QListWidgetItem(resolved);
    item->setData(Qt::UserRole, QVariant(wifi->resolveBSSID(ap)));
    ui->listAccessPoint->addItem(item);
  }
  qDebug()<<listSSID;

  qDebug()<<listAp;

  QString active_ap = wifi->getActiveAP();
  qDebug() << "<<<<<<<: " << wifi->resolveSSID(active_ap);
  QString label = "You are connected to network: ";
  label.append(wifi->resolveSSID(active_ap));
  ui->statusLabel->setText(label);

  wifi->getAvailableConnectios();
  wifi->getConnections();

//  debus->getConnections();
}

Utama::~Utama()
{
  delete ui;
  delete debus;
  delete wifi;

  if (_activeWifiConnProxy != nullptr) {
    delete _activeWifiConnProxy;
    _activeWifiConnProxy = nullptr;
  }
}

void Utama::on_pushButton_clicked() {
    QString bssid = ui->listAccessPoint->currentItem()->data(Qt::UserRole).toString();

    try {
      _connectToWifiNetwork(bssid);
    } catch (const std::runtime_error& except) {
      updateUI();
    }
}

void Utama::on_ethernetSwitch_clicked(bool checked) {
  try {
    debus->ethernetDevicesSetAutoconnect(false);
    debus->deactivateEthernetConnections();

    if (checked) {
      QString ethernetConn;

      try {
        ethernetConn = debus->getConnectionByUuid(Utama::ETHERNET_CONNECTION_UUID);
      } catch (const std::runtime_error &except) {
        ethernetConn = debus->createAutomaticEthernetConnection(Utama::ETHERNET_CONNECTION_UUID,
                                                                Utama::ETHERNET_CONNECTION_ID);
      }

      debus->activateEthernetConnection(ethernetConn);
    }
  } catch (const std::runtime_error &except) {
    ui->ethernetSwitch->setChecked(!checked);
  }

  ui->ethernetSwitch->setText(ui->ethernetSwitch->isChecked() ? "Disconnect" : "Connect");
}

void Utama::_onNetworkManagerStateChanged(const int newState) {
    updateUI();
}

void Utama::_onActiveWifiConnStateChanged(NMActiveConnectionState newState) {
  if (newState == NM_ACTIVE_CONNECTION_STATE_ACTIVATING ||
      newState == NM_ACTIVE_CONNECTION_STATE_DEACTIVATING) {
    return;
  }

  QString ssid;
  QString bssid;
  QString wifiConn;

  if (_activeWifiConnProxy != nullptr) {
    ssid = _activeWifiConnProxy->property("ssid").toString();
    bssid = _activeWifiConnProxy->property("bssid").toString();
    wifiConn = _activeWifiConnProxy->property("wifiConn").toString();

    delete _activeWifiConnProxy;
    _activeWifiConnProxy = nullptr;
  }

  if (newState == NM_ACTIVE_CONNECTION_STATE_ACTIVATED) {
    return;
  }

  // Handle possible incorrect password.

  PasswordDialog passwordDialog(ssid, bssid, this);
  QString currPassword = ConnectionProxy(wifiConn).getSecrets("802-11-wireless-security")["802-11-wireless-security"]["psk"].toString();
  passwordDialog.setPassword(currPassword);

  if (!passwordDialog.exec()) {
    return;
  }

  try {
    _connectToWifiNetwork(bssid, passwordDialog.getPassword());
  } catch (const std::runtime_error &except) {
    updateUI();
  }
}

void Utama::_connectToWifiNetwork(const QString& bssid, QString password) {
  QString ap;
  bool apFound = false;

  foreach(ap, wifi->getListAP()) {
    if (bssid == wifi->resolveBSSID(ap)) {
      apFound = true;
      break;
    }
  }

  if (!apFound) {
    QString errMsg = "Could not found the Access Point with BSSID=" + bssid;
    throw std::runtime_error(errMsg.toUtf8().constData());
  }

  QString ssid = wifi->resolveSSID(ap);

  debus->wifiDevicesSetAutoconnect(false);
  debus->deactivateWifiConnections();

  QString wifiConnId = Utama::WIFI_CONNECTION_ID_PREFIX + " (" + ssid + "-" + bssid + ")";
  QString wifiConn;

  try {
    wifiConn = debus->getConnectionById(wifiConnId);
  } catch (const std::runtime_error &except) {
    wifiConn = debus->createAutomaticWifiConnection(wifiConnId, ssid);
  }

  if (!password.isNull()) {
    ConnectionProxy connProxy(wifiConn);

    ConnectionSettings connSettings = connProxy.getSettings();
    connSettings["802-11-wireless-security"]["psk"] = password;

    connProxy.update(connSettings);
  }

  QString activeWifiConn = debus->activateWifiConnection(wifiConn, ap);

  if (_activeWifiConnProxy != nullptr) {
    delete _activeWifiConnProxy;
  }

  _activeWifiConnProxy = new ActiveConnectionProxy(activeWifiConn);
  _activeWifiConnProxy->setProperty("ssid", QVariant(ssid));
  _activeWifiConnProxy->setProperty("bssid", QVariant(bssid));
  _activeWifiConnProxy->setProperty("wifiConn", QVariant(wifiConn));

  QObject::connect(_activeWifiConnProxy,
                   SIGNAL(stateChanged(NMActiveConnectionState)),
                   this,
                   SLOT(_onActiveWifiConnStateChanged(NMActiveConnectionState)));
}
