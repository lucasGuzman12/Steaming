#ifndef APISTEAMBIBLIOTECA_H
#define APISTEAMBIBLIOTECA_H

// Cliente para la API oficial de Steam vinculada a la biblioteca del usuario.
#include "DataManager.h"

// Fuerza la presencia de STEAM_API_KEY porque esta API no responde sin ella.
class ApiSteamBiblioteca : public DataManager
{
public:
    explicit ApiSteamBiblioteca(QWidget *parent = nullptr);

    QString get(QString endpoint) override;
    QString post(QString endpoint, QJsonObject data) override;
};

#endif // APISTEAMBIBLIOTECA_H
