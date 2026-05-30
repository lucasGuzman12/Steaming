// Implementacion minima del cliente OpenAI usado por Recomendador.
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
    // Reutiliza la logica base de GET definida en DataManager.
    return ejecutarGet(endpoint);
}

QString ApiChatGpt::post(QString endpoint, QJsonObject data)
{
    // Reutiliza la logica base de POST definida en DataManager.
    return ejecutarPost(endpoint, data);
}
