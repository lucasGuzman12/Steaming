// Cliente concreto para consultas de libros.
#include "ApiLibros.h"

ApiLibros::ApiLibros(QWidget *parent)
    : DataManager(
          "libros",
          "https://www.googleapis.com/books/v1/",
          DataManager::cargarConfiguracion("GOOGLE_BOOKS_API_KEY"),
          parent
      )
{
}

QString ApiLibros::get(QString endpoint)
{
    QString normalizedEndpoint = endpoint;
    // Google Books acepta la clave como query param opcional.
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "key", key);
    }

    return ejecutarGet(normalizedEndpoint);
}

QString ApiLibros::post(QString endpoint, QJsonObject data)
{
    QString normalizedEndpoint = endpoint;
    // Mantiene el mismo criterio de autenticacion para POST.
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "key", key);
    }

    return ejecutarPost(normalizedEndpoint, data);
}
