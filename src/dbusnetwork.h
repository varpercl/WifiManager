#ifndef DBUSNETWORK_H
#define DBUSNETWORK_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QStringList>

class DbusNetwork : public QObject
{
    Q_OBJECT
public:
    explicit DbusNetwork(QObject *parent = 0);
    QStringList getDevices() const;
    QStringList getEthernetDevices() const;
    QStringList getWifiDevices() const;
    QStringList getActiveConnections() const;
    QStringList getEthernetActiveConnections() const;
    QStringList getWifiActiveConnections() const;
    QString getActiveConnectionType(const QString &activeConn) const;
    QString getProperties(QString property);
    int getStatus() const;
    int getDeviceType(const QString &dev) const;

    QStringList getConnections();
    QString getConnectionByUuid(const QString &uuid) const;
    QString getConnectionById(const QString &id) const;

    bool isNetworkingEnabled() const;
    void setNetworkingEnabled(bool enabled) const;
    QString activateConnection(const QString &conn, const QString &device, const QString specific_object="/") const;
    QString activateEthernetConnection(const QString &ethernetConn, const QString ethernetDevice=QString()) const;
    QString activateWifiConnection(const QString &wifiConn, const QString &ap, const QString wifiDevice=QString()) const;
    void deactivateConnection(const QString &activeConn) const;
    void deactivateEthernetConnections() const;
    void deactivateWifiConnections() const;
    void disconnectEthernetDevices() const;
    void ethernetDevicesSetAutoconnect(bool autoconnect) const;
    void wifiDevicesSetAutoconnect(bool autoconnect) const;
    QString createAutomaticEthernetConnection(const QString &uuid, const QString &id) const;
    QString createAutomaticWifiConnection(const QString &id, const QString &ssid) const;

    ~DbusNetwork();

signals:
    /**
     * @brief Emitted when the NetworkManager's "State" property changed
     * @param newState the new state value
     *
     * For information on the possible values and meaning of "newState" see
     * https://developer.gnome.org/NetworkManager/1.2/nm-dbus-types.html#NMState
     */
    void stateChanged(const int newState);

    /**
     * @brief Emitted when the NetworkManager's "ActiveConnections" property changed
     * @param activeConnections a list of object paths of the current active connections
     */
    void activeConnectionsChanged(QStringList activeConnections);

public slots:

private slots:
    void _onPropertiesChanged(const QMap<QString, QVariant> properties);

private:
    const QString nmService;
    const QString nmObject;
    const QString nmInterface;
};

#endif // DBUSNETWORK_H
