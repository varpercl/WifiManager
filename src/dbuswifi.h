#ifndef DBUSWIFI_H
#define DBUSWIFI_H

#include <QObject>
#include <QStringList>
#include <QDBusMessage>

class DbusWifi : public QObject
{
    Q_OBJECT
public:
    explicit DbusWifi(QObject *parent = 0);
    void setDevice(QString dev);
    QStringList getListAP();
    QString getActiveAP();
    QString resolveSSID(QString ap);
    QString resolveBSSID(QString ap);
    ~DbusWifi();

    void getAvailableConnectios();
    QString getConnections();
    void connectionSettings(QString c);

signals:

public slots:
    void dumpMessage(QDBusMessage m);

private:
    QString device;
    QString nmDBus;

    QStringList availableConnections;
};

#endif // DBUSWIFI_H
