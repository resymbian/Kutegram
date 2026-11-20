#ifndef APISETTINGS_H
#define APISETTINGS_H

#include <QObject>

#include "apicredentials.h"

// Thin QML wrapper around the libkg credential store. Exposed as the
// "apiSettings" context property so the user can enter their own Telegram
// API ID/hash on the device instead of baking them into the binary.
class ApiSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int apiId READ apiId NOTIFY credentialsChanged)
    Q_PROPERTY(QString apiHash READ apiHash NOTIFY credentialsChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY credentialsChanged)

public:
    explicit ApiSettings(QObject *parent = 0) : QObject(parent) {}

    int apiId() const { return kgApiId(); }
    QString apiHash() const { return kgApiHash(); }
    bool configured() const { return kgHasApiCredentials(); }

    Q_INVOKABLE void setCredentials(int apiId, const QString &apiHash)
    {
        kgSetApiCredentials(apiId, apiHash);
        emit credentialsChanged();
    }

signals:
    void credentialsChanged();
};

#endif // APISETTINGS_H
