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
    return ejecutarGet(endpoint);
}

QString ApiSteamJuegos::post(QString endpoint, QJsonObject data)
{
    return ejecutarPost(endpoint, data);
}
