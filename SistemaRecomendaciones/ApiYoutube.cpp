// Cliente concreto para consumir la API de YouTube.
#include "ApiYoutube.h"

ApiYoutube::ApiYoutube(QWidget *parent)
    : DataManager(
          "youtube",
          "https://www.googleapis.com/youtube/v3/",
          DataManager::cargarConfiguracion("YOUTUBE_API_KEY"),
          parent
      )
{
}

QString ApiYoutube::get(QString endpoint)
{
    // La API de YouTube requiere clave obligatoria.
    if (key.trimmed().isEmpty()) {
        return crearErrorJson(
            "Falta YOUTUBE_API_KEY para consumir la API de YouTube.",
            endpoint
        );
    }

    return ejecutarGet(agregarParametroQuery(endpoint, "key", key));
}

QString ApiYoutube::post(QString endpoint, QJsonObject data)
{
    // Conserva la misma validacion para operaciones POST.
    if (key.trimmed().isEmpty()) {
        return crearErrorJson(
            "Falta YOUTUBE_API_KEY para enviar datos a la API de YouTube.",
            endpoint
        );
    }

    const QString normalizedEndpoint = agregarParametroQuery(endpoint, "key", key);
    return ejecutarPost(normalizedEndpoint, data);
}
