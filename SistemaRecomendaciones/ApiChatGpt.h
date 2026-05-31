#ifndef APICHATGPT_H
#define APICHATGPT_H

// Wrapper especifico para la API Responses de OpenAI.
#include "DataManager.h"

// Solo fija la URL base, la autenticacion Bearer y delega el transporte en
// DataManager.
class ApiChatGpt : public DataManager
{
public:
    explicit ApiChatGpt(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APICHATGPT_H
