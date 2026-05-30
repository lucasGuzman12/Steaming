// Cliente concreto para storesearch y appdetails publicos de Steam.
#include "ApiSteamJuegos.h"

ApiSteamJuegos::ApiSteamJuegos(QWidget *parent)
    : DataManager(
          "steam_juegos",
          "https://store.steampowered.com/api/",
          QString(),
          parent
      )
{
}

QString ApiSteamJuegos::get(QString endpoint)
{
    // Hereda todo el comportamiento HTTP desde DataManager.
    return ejecutarGet(endpoint);
}

QString ApiSteamJuegos::post(QString endpoint, QJsonObject data)
{
    // Hereda todo el comportamiento HTTP desde DataManager.
    return ejecutarPost(endpoint, data);
}
