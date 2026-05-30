#ifndef APIPELICULAS_H
#define APIPELICULAS_H

#include "DataManager.h"

class ApiPeliculas : public DataManager
{
public:
    explicit ApiPeliculas(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APIPELICULAS_H
