#ifndef DBUSNETWORK_H
#define DBUSNETWORK_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

class DbusNetwork : public QObject
{
    Q_OBJECT
public:
    explicit DbusNetwork(QObject *parent = 0);
    QStringList getDevices();
    QStringList getActiveConnection();
    QString getProperties(QString property);
    int getStatus() const;
    int getDeviceType(QString dev);

    QStringList getConnections();

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

public slots:

private slots:
    void _onPropertiesChanged(const QMap<QString, QVariant> properties);

private:
    const QString nmService;
    const QString nmObject;
    const QString nmInterface;
};

#endif // DBUSNETWORK_H
