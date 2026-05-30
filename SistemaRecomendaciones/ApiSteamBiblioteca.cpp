#include "ApiSteamBiblioteca.h"

ApiSteamBiblioteca::ApiSteamBiblioteca(QWidget *parent)
    : DataManager(
          "steam_biblioteca",
          "https://api.steampowered.com/",
          DataManager::cargarConfiguracion("STEAM_API_KEY"),
          parent
      )
{
}

QString ApiSteamBiblioteca::get(QString endpoint)
{
    if (key.trimmed().isEmpty()) {
        return crearErrorJson(
            "Falta STEAM_API_KEY para consultar la biblioteca de Steam.",
            endpoint
        );
    }

    return ejecutarGet(agregarParametroQuery(endpoint, "key", key));
}

QString ApiSteamBiblioteca::post(QString endpoint, QJsonObject data)
{
    if (key.trimmed().isEmpty()) {
        return crearErrorJson(
            "Falta STEAM_API_KEY para enviar datos a Steam.",
            endpoint
        );
    }

    const QString normalizedEndpoint =
        agregarParametroQuery(endpoint, "key", key);
    return ejecutarPost(normalizedEndpoint, data);
}
