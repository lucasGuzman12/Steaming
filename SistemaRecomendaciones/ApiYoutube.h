#ifndef APIYOUTUBE_H
#define APIYOUTUBE_H

// Cliente concreto para la API de busqueda de YouTube.
#include "DataManager.h"

// Valida la presencia de la API key y la agrega a cada request.
class ApiYoutube : public DataManager
{
public:
    explicit ApiYoutube(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APIYOUTUBE_H
