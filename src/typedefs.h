#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <QMetaType>

typedef QMap<QString, QMap<QString, QVariant> > _MapOfMap;
Q_DECLARE_METATYPE(_MapOfMap)

typedef _MapOfMap ConnectionSettings, ConnectionSecrets;

#endif // TYPEDEFS_H
