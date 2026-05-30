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
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "key", key);
    }

    return ejecutarGet(normalizedEndpoint);
}

QString ApiLibros::post(QString endpoint, QJsonObject data)
{
    QString normalizedEndpoint = endpoint;
    if (!key.trimmed().isEmpty()) {
        normalizedEndpoint = agregarParametroQuery(normalizedEndpoint, "key", key);
    }

    return ejecutarPost(normalizedEndpoint, data);
}
