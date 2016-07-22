#ifndef DBUSNETWORK_H
#define DBUSNETWORK_H

#include <QObject>

class DbusNetwork : public QObject
{
    Q_OBJECT
public:
    explicit DbusNetwork(QObject *parent = 0);
    QStringList getDevices();
    QStringList getActiveConnection();
    QString getProperties(QString property);
    int getStatus();
    int getDeviceType(QString dev);

    QStringList getConnections();

    ~DbusNetwork();

signals:

public slots:
private:
    QString nmDBus;
};

#endif // DBUSNETWORK_H
