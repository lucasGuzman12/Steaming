#ifndef APILIBROS_H
#define APILIBROS_H

#include "DataManager.h"

class ApiLibros : public DataManager
{
public:
    explicit ApiLibros(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APILIBROS_H
