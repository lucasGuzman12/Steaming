#ifndef SERVICIOSAPI_H
#define SERVICIOSAPI_H

#include "ApiChatGpt.h"
#include "ApiLibros.h"
#include "ApiPeliculas.h"
#include "ApiSteamBiblioteca.h"
#include "ApiSteamJuegos.h"
#include "ApiYoutube.h"
#include "Juego.h"
#include "Libro.h"
#include "Pelicula.h"
#include "Video.h"

#include <QJsonObject>
#include <QString>

class ServicioSteamBiblioteca
{
public:
    ServicioSteamBiblioteca() = default;

    QJsonObject obtenerBiblioteca(
        const QString &steamId,
        int enrichTop = 5
    );

private:
    ApiSteamBiblioteca apiBiblioteca;
    ApiSteamJuegos apiSteamStore;

    QJsonObject obtenerDetalleJuego(int appId);
    QJsonObject crearErrorObjeto(
        const QString &mensaje,
        const QString &detalle = QString()
    ) const;
};

class ServicioSteamJuegos
{
public:
    ServicioSteamJuegos() = default;

    QString buscarJuegos(const QString &texto);
    QJsonObject buscarJuegoDetalle(
        const QString &texto,
        int maxResults = 1
    );
    Juego buscarPrimerJuego(
        const QString &texto,
        int maxResults = 1
    );

private:
    ApiSteamJuegos api;

    QJsonObject obtenerDetalleJuego(int appId);
    double obtenerPrecioFinal(const QJsonObject &priceData) const;
};

class ServicioLibros
{
public:
    ServicioLibros() = default;

    QString buscarLibro(const QString &titulo);
    QJsonObject buscarLibroDetalle(
        const QString &titulo,
        int maxResults = 1
    );
    Libro buscarPrimerLibro(
        const QString &titulo,
        int maxResults = 1
    );

private:
    ApiLibros api;
};

class ServicioPeliculas
{
public:
    ServicioPeliculas() = default;

    QString buscarPelicula(const QString &nombre);
    QJsonObject buscarPeliculaDetalle(
        const QString &nombre,
        int maxResults = 1
    );
    Pelicula buscarPrimeraPelicula(
        const QString &nombre,
        int maxResults = 1
    );

private:
    ApiPeliculas api;

    QString obtenerDirector(int peliculaId);
};

class ServicioYoutube
{
public:
    ServicioYoutube() = default;

    QJsonObject buscarVideos(
        const QString &texto,
        int maxResults = 5
    );
    Video buscarPrimerVideo(
        const QString &texto,
        int maxResults = 1
    );

private:
    ApiYoutube api;
};

class SolicitudRecomendacionesChatGpt
{
public:
    SolicitudRecomendacionesChatGpt(
        const QJsonObject &contextoSteam,
        int itemsPorCategoria,
        const QString &modelo
    );

    QJsonObject crearPayload() const;
    QJsonObject interpretarRespuesta(const QString &respuesta) const;

private:
    QString construirPrompt() const;
    QJsonObject construirSchema() const;
    QString extraerTextoSalida(const QJsonObject &respuesta) const;
    QJsonObject crearErrorObjeto(
        const QString &mensaje,
        const QString &detalle = QString()
    ) const;

    QJsonObject contextoSteam;
    int itemsPorCategoria;
    QString modelo;
};

class ServicioRecomendacionesChatGpt
{
public:
    ServicioRecomendacionesChatGpt();

    QJsonObject generarRecomendaciones(
        const QJsonObject &contextoSteam,
        int itemsPorCategoria = 40
    );

private:
    ApiChatGpt api;
    QString apiKey;
    QString modelo;
};

#endif // SERVICIOSAPI_H
