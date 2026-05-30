#ifndef APIYOUTUBE_H
#define APIYOUTUBE_H

#include "DataManager.h"

class ApiYoutube : public DataManager
{
public:
    explicit ApiYoutube(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APIYOUTUBE_H
