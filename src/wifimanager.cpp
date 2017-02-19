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

#include "wifimanager.h"
#include "ui_utama.h"
#include "dbusnetwork.h"
#include "activeconnectionproxy.h"
#include "passworddialog.h"
#include "connectionproxy.h"
#include "typedefs.h"

const QString WifiManager::ETHERNET_CONNECTION_UUID = "e1ea3417-7c4b-4008-9406-51b9221c37a8";
const QString WifiManager::ETHERNET_CONNECTION_ID = "Automatic Ethernet Connection";
const QString WifiManager::WIFI_CONNECTION_ID_PREFIX = "Automatic Wifi Connection";

WifiManager::WifiManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WifiManager),
    _activeWifiConnProxy(nullptr)
{
    ui->setupUi(this);

    debus = new DbusNetwork();
    wifi = new DbusWifi();

    // Connect signals
    QObject::connect(debus, SIGNAL(stateChanged(int)), this, SLOT(_onNetworkManagerStateChanged(int)));
    QObject::connect(debus, SIGNAL(activeConnectionsChanged(QStringList)), SLOT(onActiveConnectionsChanged(QStringList)));
    QObject::connect(ui->btnUpdate, SIGNAL(clicked()), SLOT(updateUI()));
    updateUI();
    ui->ethernetSwitch->click();  // simulate user click to connect to ethernet
}

void WifiManager::updateUI(){
    int status = debus->getStatus();

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

    QString label;
    switch(status){
        case 1: //No active connection
            if(active_ap != "/") //When is trying to get a connection
            {
                label = "Trying to connect.";
                ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #F4F436; border-radius: 15px}");
            }else{ //If not conected
                label = "Disconnected.";
                ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #F44336; border-radius: 15px}");
            }
            break;
        case 4: //When connected
            label = "You are now connected to: ";
            label.append(wifi->resolveSSID(active_ap));
            ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #4CAF50; border-radius: 15px}");
            break;
        default:
            ui->labelIndicator->setStyleSheet("QLabel#labelIndicator{background-color: #F44336; border-radius: 15px}");
            label = "Disconnected.";

            break;
    }

    ui->statusLabel->setText(label);

    wifi->getAvailableConnectios();
    wifi->getConnections();

    ui->ethernetSwitch->setText(debus->getEthernetActiveConnections().empty() ? "Connect" : "Disconnect");
}

WifiManager::~WifiManager()
{
    delete ui;
    delete debus;
    delete wifi;

    if (_activeWifiConnProxy != nullptr) {
        delete _activeWifiConnProxy;
        _activeWifiConnProxy = nullptr;
    }
}

void WifiManager::on_btnConnect_clicked() {

    QListWidgetItem *curItem = ui->listAccessPoint->currentItem();


    if(curItem)
    {
        QString
                ap = wifi->getActiveAP(),
                ssid = curItem->text();

        if(ssid == wifi->resolveSSID(ap))
        {
            try
            {
                debus->deactivateWifiConnections();
            }
            catch(const std::runtime_error &e)
            {
                qDebug() << e.what();
            }

        }
        else
        {
            QString bssid = curItem->data(Qt::UserRole).toString();

            try {
                _connectToWifiNetwork(bssid, ui->txtPassword->text());
            } catch (const std::runtime_error &except) {
                updateUI();
            }
        }
    }
}

void WifiManager::on_ethernetSwitch_clicked(bool checked) {
    try {
        debus->ethernetDevicesSetAutoconnect(false);
        debus->deactivateEthernetConnections();

        if (checked) {
            QString ethernetConn;

            try {
                ethernetConn = debus->getConnectionByUuid(WifiManager::ETHERNET_CONNECTION_UUID);
            } catch (const std::runtime_error &except) {
                ethernetConn = debus->createAutomaticEthernetConnection(WifiManager::ETHERNET_CONNECTION_UUID,
                                                                        WifiManager::ETHERNET_CONNECTION_ID);
            }

            debus->activateEthernetConnection(ethernetConn);
        }
    } catch (const std::runtime_error &except) {
        ui->ethernetSwitch->setChecked(!checked);
    }

    ui->ethernetSwitch->setText(ui->ethernetSwitch->isChecked() ? "Disconnect" : "Connect");
}

void WifiManager::_onNetworkManagerStateChanged(const int /*newState*/) {
    updateUI();
}

void WifiManager::onActiveConnectionsChanged(const QStringList &/*actives*/)
{
    updateUI();
    qDebug() << "Connection list changed";
}

void WifiManager::_onActiveWifiConnStateChanged(NMActiveConnectionState newState) {
    if (newState == NM_ACTIVE_CONNECTION_STATE_ACTIVATING ||
            newState == NM_ACTIVE_CONNECTION_STATE_DEACTIVATING) {
        return;
    }

    if (newState == NM_ACTIVE_CONNECTION_STATE_ACTIVATED) {
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

    updateUI();
}

void WifiManager::_connectToWifiNetwork(const QString& bssid, QString password) {
    QString
            ap,
            resolvedAP;
    bool apFound = false;

    QStringList aps = wifi->getListAP();

    foreach(ap, aps) {
        resolvedAP = wifi->resolveBSSID(ap);
        if (bssid == resolvedAP) {
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

    QString wifiConnId = WifiManager::WIFI_CONNECTION_ID_PREFIX + " (" + ssid + "-" + bssid + ")";
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

void WifiManager::on_listAccessPoint_currentRowChanged(int currentRow)
{
    if(currentRow == -1) return;

    QString
            ssid = ui->listAccessPoint->currentItem()->text(),
            bssid = ui->listAccessPoint->currentItem()->data(Qt::UserRole).toString(),
            wifiConn,
            currPassword,
            wifiConnId = WifiManager::WIFI_CONNECTION_ID_PREFIX + " (" + ssid + "-" + bssid + ")",
            currentWifiConn = wifi->resolveSSID(wifi->getActiveAP());

    ui->btnConnect->setText(currentWifiConn == ssid ? "Disconnect" : "Connect");

    try
    {
        wifiConn = debus->getConnectionById(wifiConnId);
        currPassword = ConnectionProxy(wifiConn).getSecrets("802-11-wireless-security")["802-11-wireless-security"]["psk"].toString();

        ui->txtPassword->setText(currPassword);
    } catch (const std::runtime_error &except) {
        qDebug() << except.what();
    }

}
