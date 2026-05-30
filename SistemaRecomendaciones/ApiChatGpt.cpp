#include "ApiChatGpt.h"

ApiChatGpt::ApiChatGpt(QWidget *parent)
    : DataManager(
          "chatgpt",
          "https://api.openai.com/v1/",
          DataManager::cargarConfiguracion("OPENAI_API_KEY"),
          parent,
          true
      )
{
}

QString ApiChatGpt::get(QString endpoint)
{
    return ejecutarGet(endpoint);
}

QString ApiChatGpt::post(QString endpoint, QJsonObject data)
{
    return ejecutarPost(endpoint, data);
}
