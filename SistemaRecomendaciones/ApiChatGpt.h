#ifndef APICHATGPT_H
#define APICHATGPT_H

#include "DataManager.h"

class ApiChatGpt : public DataManager
{
public:
    explicit ApiChatGpt(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APICHATGPT_H
