#include "utama.h"
#include "ui_utama.h"
#include <dbusnetwork.h>
#include <QDebug>
#include <QDBusInterface>
#include <QMap>
#include <QVariant>
#include <QUuid>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QMetaObject>
#include <QObject>
#include <stdexcept>


typedef  QMap<QString, QMap<QString, QVariant> > NMVariantMap;
Q_DECLARE_METATYPE(NMVariantMap)

const QString Utama::ETHERNET_CONNECTION_UUID = "e1ea3417-7c4b-4008-9406-51b9221c37a8";
const QString Utama::ETHERNET_CONNECTION_ID = "Automatic Ethernet Connection";

Utama::Utama(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Utama)
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
    ui->listAccessPoint->addItem(resolved);
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
}

void Utama::on_pushButton_clicked()
{
    int i = ui->listAccessPoint->currentRow();
    QString ssid = ui->listAccessPoint->item(i)->text();
    qDebug() << " :::: ::: :: ### " << ssid;

    QString apoint;
    QStringList listAp = wifi->getListAP();
    foreach(QString ap, listAp){
      QString resolved = wifi->resolveSSID(ap);
      if(resolved == ssid) {
        apoint = ap;
        break;
      }
    }
    qDebug() << apoint;

    QString nmDBus = "org.freedesktop.NetworkManager";

    QDBusInterface iface(nmDBus,
                         "/org/freedesktop/NetworkManager/Settings",
                         "org.freedesktop.NetworkManager.Settings",
                         QDBusConnection::systemBus());
  if(iface.isValid()) {
    QMap<QString, QVariant> connection;
    connection.insert("type", "802-11-wireless");
    connection.insert("uuid", QUuid::createUuid().toString().remove('{').remove('}')); // The UUID of the new connection
    connection.insert("id", "%%Walo this is the name of the connection%%"); // A name fo the connection

    QMap<QString, QVariant> wifi;
    wifi.insert("ssid", ssid.toLatin1()); // This is the name of the SSID --> must be a QByteArray
    wifi.insert("mode", "infrastructure");

    QMap<QString, QVariant> sec;
    sec.insert("key-mgmt", "wpa-psk");
    sec.insert("auth-alg", "open");
    sec.insert("psk", "qwertyuiop15"); // Here we specify the password of the wifi

    QMap<QString, QVariant> ip4;
    ip4["method"] = "auto";

    QMap<QString, QVariant> ip6;
    ip6["method"] = "ignore";

    QMap<QString, QMap<QString, QVariant> > map;
    map["connection"] = connection;
    map["802-11-wireless"] = wifi;
    map["802-11-wireless-security"] = sec;
    map["ipv4"] = ip4;
    map["ipv6"] = ip6;



    qRegisterMetaType<NMVariantMap>();
    qDBusRegisterMetaType<NMVariantMap>();
//    qRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

    QVariant v = QVariant::fromValue(map);

    QDBusMessage query = iface.call("AddConnection", v);
    if(query.type() == QDBusMessage::ReplyMessage) {
      qDebug() << " %%%%%%%%%%%%%%%%%%%%";
    }
    else {
      qDebug() << query.errorMessage();
    }
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
