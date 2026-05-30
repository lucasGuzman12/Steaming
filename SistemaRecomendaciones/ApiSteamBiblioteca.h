#ifndef APISTEAMBIBLIOTECA_H
#define APISTEAMBIBLIOTECA_H

#include "DataManager.h"

class ApiSteamBiblioteca : public DataManager
{
public:
    explicit ApiSteamBiblioteca(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APISTEAMBIBLIOTECA_H
