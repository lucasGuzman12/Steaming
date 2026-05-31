#ifndef RECOMENDADOR_H
#define RECOMENDADOR_H

// Motor central del proyecto: toma la biblioteca de Steam, consulta OpenAI,
// hidrata sugerencias con APIs externas y administra cache/paginacion.
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "ApiChatGpt.h"
#include "ApiLibros.h"
#include "ApiPeliculas.h"
#include "ApiSteamBiblioteca.h"
#include "ApiSteamJuegos.h"
#include "ApiYoutube.h"
#include "Juego.h"
#include "Libro.h"
#include "PaqueteRecomendaciones.h"
#include "Pelicula.h"
#include "SolicitudChatGpt.h"
#include "Video.h"

// Fachada principal de negocio para toda la app Qt.
class Recomendador : public QObject
{
    Q_OBJECT

public:
    explicit Recomendador(QObject *parent = nullptr);

    // Consultas base a servicios externos.
    QJsonObject obtenerBibliotecaSteam(
        const QString &steamId,
        int enrichTop = 5
    );

    QJsonObject buscarLibros(
        const QString &titulo,
        int maxResults = 5
    );

    QString buscarPeliculas(const QString &nombre);

    QJsonObject buscarVideosYoutube(
        const QString &texto,
        int maxResults = 5
    );

    // Flujo principal de recomendacion y salidas derivadas.
    QJsonObject recomendar(
        const QString &steamId,
        int itemsPorCategoria = 5,
        int enrichTop = 5
    );

    QJsonArray recomendarPeliculas(
        const QString &steamId,
        int cantidad = 5,
        int enrichTop = 5
    );

    QJsonArray recomendarLibros(
        const QString &steamId,
        int cantidad = 5,
        int enrichTop = 5
    );

    QJsonArray recomendarJuegos(
        const QString &steamId,
        int cantidad = 5,
        int enrichTop = 5
    );

    QJsonArray recomendarVideos(
        const QString &steamId,
        int cantidad = 5,
        int enrichTop = 5
    );

    QString recomendarComoTexto(
        const QString &steamId,
        int itemsPorCategoria = 5,
        int enrichTop = 5
    );

    PaqueteRecomendaciones get_paquete_recomendaciones(
        const QString &steamId,
        int enrichTop = 5
    );

    // Navegacion sobre resultados cacheados sin volver a consultar OpenAI.
    QJsonObject obtenerSiguientesRecomendaciones(
        const QString &steamId,
        int itemsPorCategoria = 5
    );

    QJsonArray obtenerSiguientesPeliculas(
        const QString &steamId,
        int cantidad = 5
    );

    QJsonArray obtenerSiguientesLibros(
        const QString &steamId,
        int cantidad = 5
    );

    QJsonArray obtenerSiguientesJuegos(
        const QString &steamId,
        int cantidad = 5
    );

    QJsonArray obtenerSiguientesVideos(
        const QString &steamId,
        int cantidad = 5
    );

    // Helpers de presentacion y demos.
    QString construirResumenRecomendaciones(
        const QJsonObject &objeto
    ) const;

    QString construirSalidaVectorPeliculas(
        const PaqueteRecomendaciones &paquete
    ) const;

    QJsonObject ejecutarChatbotDemo(
        const QString &mensajeUsuario
    );

signals:
    // Permite que la UI se entere de que ya hay datos listos para mostrar.
    void recomendacionesCargadas(const QString &steamId);

private:
    // Clientes concretos para cada fuente externa.
    ApiSteamBiblioteca apiSteamBiblioteca;
    ApiSteamJuegos apiSteamJuegos;
    ApiLibros apiLibros;
    ApiPeliculas apiPeliculas;
    ApiYoutube apiYoutube;
    ApiChatGpt apiChatGpt;

    // Transformacion de biblioteca a contexto y conversiones JSON/modelo.
    QJsonObject construirContextoSteam(const QJsonObject &biblioteca) const;
    QString listaComoTexto(const QJsonArray &array) const;
    QJsonArray peliculasAJson(const QVector<Pelicula> &peliculas) const;
    QString vectorPeliculasComoTexto(
        const QVector<Pelicula> &peliculas
    ) const;
    bool tieneCantidadMinimaPorCategoria(
        const QJsonObject &recomendaciones,
        int cantidadMinima
    ) const;
    int calcularCantidadRecomendacionesAGenerar(
        int itemsSolicitados
    ) const;
    QString inferirFranquicia(const QString &nombreJuego) const;
    QStringList arrayALista(const QJsonArray &array) const;
    Libro libroDesdeJson(const QJsonObject &objeto) const;
    Pelicula peliculaDesdeJson(const QJsonObject &objeto) const;
    Juego juegoDesdeJson(const QJsonObject &objeto) const;
    Video videoDesdeJson(const QJsonObject &objeto) const;
    QVector<Libro> librosDesdeArray(const QJsonArray &array) const;
    QVector<Pelicula> peliculasDesdeArray(const QJsonArray &array) const;
    QVector<Juego> juegosDesdeArray(const QJsonArray &array) const;
    QVector<Video> videosDesdeArray(const QJsonArray &array) const;
    // Hidrata sugerencias con detalle real desde las APIs de contenido.
    QJsonArray hidratarLibros(const QJsonArray &sugerencias);
    QJsonArray hidratarPeliculas(const QJsonArray &sugerencias);
    QJsonArray hidratarJuegos(const QJsonArray &sugerencias);
    QJsonArray hidratarVideos(const QJsonArray &sugerencias);
    QJsonObject anexarMetadatos(
        const QJsonObject &atributos,
        const QJsonObject &sugerencia,
        const QString &error = QString()
    ) const;

    // Integracion con OpenAI y manejo de reintentos/cache.
    QJsonObject cargarOGenerarRecomendaciones(
        const QString &steamId,
        const QJsonObject &biblioteca,
        int itemsPorCategoria
    );
    QJsonObject ejecutarSolicitudChatGpt(
        const SolicitudChatGpt &solicitud
    );
    QJsonObject generarRecomendacionesChatGpt(
        const QJsonObject &contextoSteam,
        int itemsPorCategoria
    );
    QJsonObject ejecutarSolicitudChatGptEnSegundoPlano(
        const SolicitudChatGpt &solicitud
    );
    QJsonObject esperarRespuestaChatGptEnSegundoPlano(
        const QString &responseId,
        int intervaloMs,
        int tiempoMaximoSegundos
    );
    bool esErrorTimeoutChatGpt(const QJsonObject &respuesta) const;
    QJsonObject crearErrorObjeto(
        const QString &api,
        const QString &mensaje,
        const QString &detalle = QString()
    ) const;
    // Detalles concretos contra cada API externa.
    QJsonObject obtenerDetalleJuegoSteam(int appId);
    QJsonObject buscarLibroDetalle(
        const QString &titulo,
        int maxResults = 1
    );
    Libro buscarPrimerLibro(
        const QString &titulo,
        int maxResults = 1
    );
    QJsonObject buscarPeliculaDetalle(
        const QString &nombre,
        int maxResults = 1
    );
    Pelicula buscarPrimeraPelicula(
        const QString &nombre,
        int maxResults = 1
    );
    QString obtenerDirectorPelicula(int peliculaId);
    QJsonObject buscarJuegoDetalle(
        const QString &texto,
        int maxResults = 1
    );
    Juego buscarPrimerJuego(
        const QString &texto,
        int maxResults = 1
    );
    double obtenerPrecioFinal(const QJsonObject &priceData) const;
    QJsonObject buscarVideosDetalle(
        const QString &texto,
        int maxResults = 5
    );
    Video buscarPrimerVideo(
        const QString &texto,
        int maxResults = 1
    );

    // Paginacion circular sobre recomendaciones ya cacheadas.
    QJsonArray obtenerBloqueSecuencial(
        const QJsonArray &todos,
        int cantidad,
        int &offset
    ) const;

    QJsonArray seleccionarBloqueCategoria(
        const QString &steamId,
        QJsonObject &cache,
        const QString &categoria,
        const QString &offsetKey,
        int cantidad
    );

    // Persistencia local de cache y telemetria simple de uso.
    int obtenerContadorEjecuciones();
    void incrementarContadorEjecuciones();
    QJsonObject cargarCache(const QString &steamId);
    void guardarCache(const QString &steamId, const QJsonObject &cache);
    QJsonObject obtenerRecomendacionesEnCache(const QString &steamId);
    QString obtenerArchivoCache(const QString &steamId) const;
};

#endif // RECOMENDADOR_H
