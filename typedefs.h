#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <QMetaType>

typedef QMap<QString, QMap<QString, QVariant> > ConnectionSettings;
Q_DECLARE_METATYPE(ConnectionSettings)

#endif // TYPEDEFS_H
