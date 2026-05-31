#ifndef APILIBROS_H
#define APILIBROS_H

// Cliente especializado para Google Books.
#include "DataManager.h"

// Inserta la API key como query param y delega la comunicacion al transporte
// generico de DataManager.
class ApiLibros : public DataManager
{
public:
    explicit ApiLibros(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APILIBROS_H
