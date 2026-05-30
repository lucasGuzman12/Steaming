#ifndef APISTEAMJUEGOS_H
#define APISTEAMJUEGOS_H

// Cliente para los endpoints publicos de la tienda de Steam.
#include "DataManager.h"

// No necesita API key porque consume endpoints abiertos del store.
class ApiSteamJuegos : public DataManager
{
public:
    explicit ApiSteamJuegos(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APISTEAMJUEGOS_H
