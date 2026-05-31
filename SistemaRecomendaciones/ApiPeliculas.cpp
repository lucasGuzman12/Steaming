// Cliente concreto para busquedas y detalles de peliculas.
#include "ApiPeliculas.h"

ApiPeliculas::ApiPeliculas(QWidget *parent)
    : DataManager(
          "peliculas",
          "https://api.themoviedb.org/3/",
          DataManager::cargarConfiguracion("TMDB_API_KEY"),
          parent
      )
{
}

QString ApiPeliculas::get(QString endpoint)
{
    QString normalizedEndpoint = endpoint;
    // TMDB exige autenticar las consultas con api_key.
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "api_key", key);
    }

    return ejecutarGet(normalizedEndpoint);
}

QString ApiPeliculas::post(QString endpoint, QJsonObject data)
{
    QString normalizedEndpoint = endpoint;
    // Mantiene el mismo criterio de autenticacion para POST.
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "api_key", key);
    }

    return ejecutarPost(normalizedEndpoint, data);
}
