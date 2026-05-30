#ifndef APISTEAMJUEGOS_H
#define APISTEAMJUEGOS_H

#include "DataManager.h"

class ApiSteamJuegos : public DataManager
{
public:
    explicit ApiSteamJuegos(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APISTEAMJUEGOS_H
