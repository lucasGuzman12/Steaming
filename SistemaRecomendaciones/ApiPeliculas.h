#ifndef APIPELICULAS_H
#define APIPELICULAS_H

// Cliente concreto para TMDB.
#include "DataManager.h"

// Agrega la api_key a cada request y reutiliza la infraestructura comun.
class ApiPeliculas : public DataManager
{
public:
    explicit ApiPeliculas(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APIPELICULAS_H
