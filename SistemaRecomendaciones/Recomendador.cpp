#include "Recomendador.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QList>
#include <QPair>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>

namespace
{
QJsonObject parsearObjetoJson(const QString &json)
{
    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(
        json.toUtf8(),
        &error
    );

    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        return QJsonObject();
    }

    return documento.object();
}

bool esVerdadero(const QString &valor)
{
    const QString valorNormalizado = valor.trimmed().toLower();
    return valorNormalizado == "1"
        || valorNormalizado == "true"
        || valorNormalizado == "yes"
        || valorNormalizado == "si";
}

bool estadoRespuestaEnProgreso(const QString &status)
{
    const QString estado = status.trimmed().toLower();
    return estado == "queued"
        || estado == "in_progress"
        || estado == "interpreting";
}
} // namespace

Recomendador::Recomendador(QObject *parent)
    : QObject(parent)
{
}

QJsonObject Recomendador::obtenerBibliotecaSteam(
    const QString &steamId,
    int enrichTop
)
{
    const QString steamIdNormalizado = steamId.trimmed();
    if (steamIdNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "steam_biblioteca",
            "Debes indicar un Steam ID valido."
        );
    }

    QUrlQuery query;
    query.addQueryItem("steamid", steamIdNormalizado);
    query.addQueryItem("include_appinfo", "true");
    query.addQueryItem("include_played_free_games", "true");
    query.addQueryItem("format", "json");

    const QString respuesta = apiSteamBiblioteca.get(
        "IPlayerService/GetOwnedGames/v1/?"
        + query.toString(QUrl::FullyEncoded)
    );

    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(
        respuesta.toUtf8(),
        &error
    );
    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        return crearErrorObjeto(
            "steam_biblioteca",
            "No se pudo parsear la respuesta de la biblioteca de Steam.",
            respuesta
        );
    }

    const QJsonObject raiz = documento.object();
    if (raiz.value("ok").isBool() && !raiz.value("ok").toBool()) {
        return raiz;
    }

    const QJsonObject response = raiz.value("response").toObject();
    const QJsonArray games = response.value("games").toArray();

    QList<QJsonObject> juegosOrdenados;
    for (const QJsonValue &valorJuego : games) {
        juegosOrdenados.append(valorJuego.toObject());
    }

    std::sort(
        juegosOrdenados.begin(),
        juegosOrdenados.end(),
        [](const QJsonObject &left, const QJsonObject &right) {
            return left.value("playtime_forever").toInt()
                > right.value("playtime_forever").toInt();
        }
    );

    QJsonArray juegos;
    int indice = 0;
    for (const QJsonObject &juegoBase : juegosOrdenados) {
        QJsonObject juego;
        juego.insert("appid", juegoBase.value("appid").toInt());
        juego.insert("nombre", juegoBase.value("name").toString("Sin nombre"));

        const int minutos = juegoBase.value("playtime_forever").toInt();
        juego.insert("minutos_jugados", minutos);
        juego.insert("horas_jugadas", minutos / 60.0);

        if (indice < enrichTop) {
            const QJsonObject detalle =
                obtenerDetalleJuegoSteam(juegoBase.value("appid").toInt());
            for (auto it = detalle.constBegin(); it != detalle.constEnd(); ++it) {
                juego.insert(it.key(), it.value());
            }
        }

        juegos.append(juego);
        ++indice;
    }

    QJsonObject resultado;
    resultado.insert("api", "steam_biblioteca");
    resultado.insert("ok", true);
    resultado.insert("steam_id", steamIdNormalizado);
    resultado.insert("cantidad", juegos.size());
    resultado.insert("perfil_accesible", response.contains("games"));
    resultado.insert("juegos", juegos);
    if (juegos.isEmpty()) {
        resultado.insert(
            "mensaje",
            "No se encontraron juegos o el perfil de Steam es privado."
        );
    }

    return resultado;
}

QJsonObject Recomendador::buscarLibros(
    const QString &titulo,
    int maxResults
)
{
    return buscarLibroDetalle(titulo, maxResults);
}

QString Recomendador::buscarPeliculas(const QString &nombre)
{
    const QJsonObject detalle = buscarPeliculaDetalle(nombre, 5);
    if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
        return QString::fromUtf8(
            QJsonDocument(detalle).toJson(QJsonDocument::Indented)
        );
    }

    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return "No se encontraron peliculas.";
    }

    QString resultado;
    for (const QJsonValue &valor : resultados) {
        const QJsonObject pelicula = valor.toObject();
        resultado += pelicula.value("titulo").toString() + "\n";
        if (!pelicula.value("autor").toString().trimmed().isEmpty()) {
            resultado += "Director: " + pelicula.value("autor").toString() + "\n";
        }
        resultado += pelicula.value("sinopsis").toString() + "\n";
        if (pelicula.value("anio").toInt() > 0) {
            resultado += QString::number(pelicula.value("anio").toInt()) + "\n";
        }
        resultado += pelicula.value("portada").toString("Sin imagen") + "\n";
        resultado += "-----\n";
    }

    return resultado;
}

QJsonObject Recomendador::buscarVideosYoutube(
    const QString &texto,
    int maxResults
)
{
    return buscarVideosDetalle(texto, maxResults);
}

QJsonObject Recomendador::recomendar(
    const QString &steamId,
    int itemsPorCategoria,
    int enrichTop
)
{
    const QJsonObject biblioteca = obtenerBibliotecaSteam(steamId, enrichTop);
    if (biblioteca.value("ok").isBool() && !biblioteca.value("ok").toBool()) {
        return biblioteca;
    }

    if (biblioteca.value("juegos").toArray().isEmpty()) {
        QJsonObject resultado;
        resultado.insert("api", "recomendador");
        resultado.insert("ok", true);
        resultado.insert("steam_id", steamId.trimmed());
        resultado.insert("biblioteca", biblioteca);
        resultado.insert(
            "mensaje",
            biblioteca.value("mensaje").toString(
                "No fue posible generar recomendaciones sin juegos visibles."
            )
        );
        return resultado;
    }

    const int itemsPorCategoriaGenerar =
        calcularCantidadRecomendacionesAGenerar(itemsPorCategoria);
    const QJsonObject cache = cargarOGenerarRecomendaciones(
        steamId,
        biblioteca,
        itemsPorCategoriaGenerar
    );
    if (cache.value("ok").isBool() && !cache.value("ok").toBool()) {
        return cache;
    }
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return crearErrorObjeto(
            "recomendador",
            "No fue posible obtener recomendaciones completas."
        );
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray peliculasBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "peliculas",
        "offset_peliculas",
        itemsPorCategoria
    );
    const QJsonArray librosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "libros",
        "offset_libros",
        itemsPorCategoria
    );
    const QJsonArray juegosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "juegos",
        "offset_juegos",
        itemsPorCategoria
    );
    const QJsonArray videosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "videos_youtube",
        "offset_videos_youtube",
        itemsPorCategoria
    );

    QJsonObject recomendaciones = cache.value("recomendaciones").toObject();

    QJsonObject resultado;
    resultado.insert("api", "recomendador");
    resultado.insert("ok", true);
    resultado.insert("steam_id", steamId.trimmed());
    resultado.insert("biblioteca", biblioteca);
    resultado.insert("contexto", construirContextoSteam(biblioteca));
    resultado.insert("perfil", recomendaciones.value("perfil").toObject());
    resultado.insert("peliculas", hidratarPeliculas(peliculasBloque));
    resultado.insert("libros", hidratarLibros(librosBloque));
    resultado.insert("juegos", hidratarJuegos(juegosBloque));
    resultado.insert("videos_youtube", hidratarVideos(videosBloque));
    resultado.insert("contador_ejecuciones", obtenerContadorEjecuciones());
    resultado.insert(
        "total_recomendaciones_disponibles",
        recomendaciones.value("peliculas").toArray().size()
    );

    emit recomendacionesCargadas(steamId);

    return resultado;
}

QJsonArray Recomendador::recomendarPeliculas(
    const QString &steamId,
    int cantidad,
    int enrichTop
)
{
    const QJsonObject biblioteca = obtenerBibliotecaSteam(steamId, enrichTop);
    if (biblioteca.value("ok").isBool() && !biblioteca.value("ok").toBool()) {
        return QJsonArray();
    }

    const QJsonObject cache = cargarOGenerarRecomendaciones(
        steamId,
        biblioteca,
        calcularCantidadRecomendacionesAGenerar(cantidad)
    );
    if (cache.value("ok").isBool() && !cache.value("ok").toBool()) {
        return QJsonArray();
    }
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    QJsonObject cacheEditable = cache;
    const QJsonArray bloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "peliculas",
        "offset_peliculas",
        cantidad
    );

    incrementarContadorEjecuciones();
    return hidratarPeliculas(bloque);
}

QJsonArray Recomendador::recomendarLibros(
    const QString &steamId,
    int cantidad,
    int enrichTop
)
{
    const QJsonObject biblioteca = obtenerBibliotecaSteam(steamId, enrichTop);
    if (biblioteca.value("ok").isBool() && !biblioteca.value("ok").toBool()) {
        return QJsonArray();
    }

    const QJsonObject cache = cargarOGenerarRecomendaciones(
        steamId,
        biblioteca,
        calcularCantidadRecomendacionesAGenerar(cantidad)
    );
    if (cache.value("ok").isBool() && !cache.value("ok").toBool()) {
        return QJsonArray();
    }
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    QJsonObject cacheEditable = cache;
    const QJsonArray bloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "libros",
        "offset_libros",
        cantidad
    );

    incrementarContadorEjecuciones();
    return hidratarLibros(bloque);
}

QJsonArray Recomendador::recomendarJuegos(
    const QString &steamId,
    int cantidad,
    int enrichTop
)
{
    const QJsonObject biblioteca = obtenerBibliotecaSteam(steamId, enrichTop);
    if (biblioteca.value("ok").isBool() && !biblioteca.value("ok").toBool()) {
        return QJsonArray();
    }

    const QJsonObject cache = cargarOGenerarRecomendaciones(
        steamId,
        biblioteca,
        calcularCantidadRecomendacionesAGenerar(cantidad)
    );
    if (cache.value("ok").isBool() && !cache.value("ok").toBool()) {
        return QJsonArray();
    }
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    QJsonObject cacheEditable = cache;
    const QJsonArray bloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "juegos",
        "offset_juegos",
        cantidad
    );

    incrementarContadorEjecuciones();
    return hidratarJuegos(bloque);
}

QJsonArray Recomendador::recomendarVideos(
    const QString &steamId,
    int cantidad,
    int enrichTop
)
{
    const QJsonObject biblioteca = obtenerBibliotecaSteam(steamId, enrichTop);
    if (biblioteca.value("ok").isBool() && !biblioteca.value("ok").toBool()) {
        return QJsonArray();
    }

    const QJsonObject cache = cargarOGenerarRecomendaciones(
        steamId,
        biblioteca,
        calcularCantidadRecomendacionesAGenerar(cantidad)
    );
    if (cache.value("ok").isBool() && !cache.value("ok").toBool()) {
        return QJsonArray();
    }
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    QJsonObject cacheEditable = cache;
    const QJsonArray bloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "videos_youtube",
        "offset_videos_youtube",
        cantidad
    );

    incrementarContadorEjecuciones();
    return hidratarVideos(bloque);
}

QString Recomendador::recomendarComoTexto(
    const QString &steamId,
    int itemsPorCategoria,
    int enrichTop
)
{
    return QString::fromUtf8(
        QJsonDocument(recomendar(steamId, itemsPorCategoria, enrichTop))
            .toJson(QJsonDocument::Indented)
    );
}

PaqueteRecomendaciones Recomendador::get_paquete_recomendaciones(
    const QString &steamId,
    int enrichTop
)
{
    const QString steamIdLimpio = steamId.trimmed();
    const QJsonObject resultado = recomendar(steamIdLimpio, 40, enrichTop);

    if (resultado.value("ok").isBool() && !resultado.value("ok").toBool()) {
        return PaqueteRecomendaciones(
            steamIdLimpio,
            false,
            resultado.value("error").toString(
                "No se pudieron obtener las 40 recomendaciones por categoria."
            )
        );
    }

    if (resultado.contains("mensaje")
        && resultado.value("peliculas").toArray().isEmpty()
        && resultado.value("libros").toArray().isEmpty()
        && resultado.value("juegos").toArray().isEmpty()
        && resultado.value("videos_youtube").toArray().isEmpty()) {
        return PaqueteRecomendaciones(
            steamIdLimpio,
            false,
            resultado.value("mensaje").toString(
                "No fue posible generar recomendaciones para este usuario."
            )
        );
    }

    return PaqueteRecomendaciones(
        steamIdLimpio,
        true,
        QString(),
        juegosDesdeArray(resultado.value("juegos").toArray()),
        librosDesdeArray(resultado.value("libros").toArray()),
        peliculasDesdeArray(resultado.value("peliculas").toArray()),
        videosDesdeArray(resultado.value("videos_youtube").toArray())
    );
}

QJsonObject Recomendador::obtenerSiguientesRecomendaciones(
    const QString &steamId,
    int itemsPorCategoria
)
{
    QJsonObject cache = cargarCache(steamId);
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return crearErrorObjeto(
            "recomendador",
            "No hay recomendaciones en cache. Genera primero con recomendar()."
        );
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray peliculasBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "peliculas",
        "offset_peliculas",
        itemsPorCategoria
    );
    const QJsonArray librosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "libros",
        "offset_libros",
        itemsPorCategoria
    );
    const QJsonArray juegosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "juegos",
        "offset_juegos",
        itemsPorCategoria
    );
    const QJsonArray videosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "videos_youtube",
        "offset_videos_youtube",
        itemsPorCategoria
    );

    const QJsonObject recomendaciones = cache.value("recomendaciones").toObject();

    QJsonObject resultado;
    resultado.insert("api", "recomendador");
    resultado.insert("ok", true);
    resultado.insert("steam_id", steamId.trimmed());
    resultado.insert("biblioteca", QJsonObject());
    resultado.insert("contexto", QJsonObject());
    resultado.insert("perfil", recomendaciones.value("perfil").toObject());
    resultado.insert("peliculas", hidratarPeliculas(peliculasBloque));
    resultado.insert("libros", hidratarLibros(librosBloque));
    resultado.insert("juegos", hidratarJuegos(juegosBloque));
    resultado.insert("videos_youtube", hidratarVideos(videosBloque));
    resultado.insert("contador_ejecuciones", obtenerContadorEjecuciones());
    resultado.insert(
        "total_recomendaciones_disponibles",
        recomendaciones.value("peliculas").toArray().size()
    );

    emit recomendacionesCargadas(steamId);

    return resultado;
}

QJsonArray Recomendador::obtenerSiguientesPeliculas(
    const QString &steamId,
    int cantidad
)
{
    QJsonObject cache = cargarCache(steamId);
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray peliculasBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "peliculas",
        "offset_peliculas",
        cantidad
    );

    return hidratarPeliculas(peliculasBloque);
}

QJsonArray Recomendador::obtenerSiguientesLibros(
    const QString &steamId,
    int cantidad
)
{
    QJsonObject cache = cargarCache(steamId);
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray librosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "libros",
        "offset_libros",
        cantidad
    );

    return hidratarLibros(librosBloque);
}

QJsonArray Recomendador::obtenerSiguientesJuegos(
    const QString &steamId,
    int cantidad
)
{
    QJsonObject cache = cargarCache(steamId);
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray juegosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "juegos",
        "offset_juegos",
        cantidad
    );

    return hidratarJuegos(juegosBloque);
}

QJsonArray Recomendador::obtenerSiguientesVideos(
    const QString &steamId,
    int cantidad
)
{
    QJsonObject cache = cargarCache(steamId);
    if (cache.isEmpty() || !cache.contains("recomendaciones")) {
        return QJsonArray();
    }

    incrementarContadorEjecuciones();

    QJsonObject cacheEditable = cache;
    const QJsonArray videosBloque = seleccionarBloqueCategoria(
        steamId,
        cacheEditable,
        "videos_youtube",
        "offset_videos_youtube",
        cantidad
    );

    return hidratarVideos(videosBloque);
}

QString Recomendador::construirResumenRecomendaciones(
    const QJsonObject &objeto
) const
{
    const QJsonObject biblioteca = objeto.value("biblioteca").toObject();
    const QJsonObject perfil = objeto.value("perfil").toObject();

    QString resumen;
    resumen += "Steam ID: " + objeto.value("steam_id").toString() + "\n";
    resumen += "Cantidad de juegos en biblioteca: "
        + QString::number(biblioteca.value("cantidad").toInt()) + "\n\n";

    if (!perfil.isEmpty()) {
        resumen += "Perfil inferido:\n";
        resumen += perfil.value("resumen").toString() + "\n\n";

        const QString generos =
            listaComoTexto(perfil.value("generos_probables").toArray());
        if (!generos.isEmpty()) {
            resumen += "Generos probables: " + generos + "\n";
        }

        const QString franquicias =
            listaComoTexto(perfil.value("franquicias_detectadas").toArray());
        if (!franquicias.isEmpty()) {
            resumen += "Franquicias detectadas: " + franquicias + "\n";
        }
    } else if (objeto.contains("mensaje")) {
        resumen += objeto.value("mensaje").toString() + "\n";
    }

    resumen += "\nLas recomendaciones detalladas se muestran abajo, separadas por categoria.";
    return resumen;
}

QString Recomendador::construirSalidaVectorPeliculas(
    const PaqueteRecomendaciones &paquete
) const
{
    QString salida;
    salida += "ok: ";
    salida += paquete.get_ok() ? "true" : "false";
    salida += "\n";
    salida += "error: ";
    salida += paquete.get_error().isEmpty() ? "(sin error)" : paquete.get_error();
    salida += "\n";
    salida += "id: " + paquete.get_id() + "\n";
    salida += "peliculas:\n";
    salida += vectorPeliculasComoTexto(paquete.get_peliculas());
    return salida;
}

QJsonObject Recomendador::ejecutarChatbotDemo(const QString &mensajeUsuario)
{
    const QString mensajeNormalizado = mensajeUsuario.trimmed();
    if (mensajeNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "Debes escribir un mensaje para probar el chatbot."
        );
    }

    const QString modelo = DataManager::cargarConfiguracion(
        "OPENAI_MODEL",
        "gpt-5-mini"
    );
    const bool usarSegundoPlano = true;

    const QString instrucciones =
        "Eres un chatbot de ejemplo para una app de escritorio en Qt. "
        "Responde en espanol, con un tono claro y amable, en un maximo de "
        "tres parrafos cortos.";

    const SolicitudChatGpt solicitud(
        QJsonObject(),
        1,
        modelo,
        instrucciones,
        "chat_demo",
        usarSegundoPlano,
        mensajeNormalizado,
        false
    );

    QJsonObject resultado = ejecutarSolicitudChatGpt(solicitud);
    if (resultado.value("ok").isBool() && !resultado.value("ok").toBool()) {
        return resultado;
    }

    resultado.insert("mensaje_usuario", mensajeNormalizado);
    resultado.insert("prompt", solicitud.construirPrompt());
    resultado.insert("payload", solicitud.crearPayload());
    return resultado;
}

QJsonObject Recomendador::construirContextoSteam(
    const QJsonObject &biblioteca
) const
{
    const QJsonArray juegos = biblioteca.value("juegos").toArray();
    double horasTotales = 0.0;
    QHash<QString, int> generos;
    QHash<QString, int> franquicias;

    QJsonArray topJuegos;
    QJsonArray bibliotecaResumida;

    for (int i = 0; i < juegos.size(); ++i) {
        const QJsonObject juego = juegos.at(i).toObject();
        horasTotales += juego.value("horas_jugadas").toDouble();

        if (i < 10) {
            topJuegos.append(
                QJsonObject{
                    {"nombre", juego.value("nombre").toString()},
                    {"horas_jugadas", juego.value("horas_jugadas").toDouble()},
                    {"generos", juego.value("generos").toArray()},
                    {"descripcion", juego.value("descripcion").toString()}
                }
            );

            for (const QString &genero : arrayALista(
                     juego.value("generos").toArray())) {
                generos[genero] += 1;
            }

            const QString franquicia =
                inferirFranquicia(juego.value("nombre").toString());
            if (!franquicia.isEmpty()) {
                franquicias[franquicia] += 1;
            }
        }

        if (i < 40) {
            bibliotecaResumida.append(juego.value("nombre").toString());
        }
    }

    QList<QPair<QString, int>> generosOrdenados;
    for (auto it = generos.constBegin(); it != generos.constEnd(); ++it) {
        generosOrdenados.append(qMakePair(it.key(), it.value()));
    }
    std::sort(
        generosOrdenados.begin(),
        generosOrdenados.end(),
        [](const auto &left, const auto &right) {
            return left.second > right.second;
        }
    );

    QJsonArray generosFrecuentes;
    for (int i = 0; i < generosOrdenados.size() && i < 6; ++i) {
        generosFrecuentes.append(generosOrdenados.at(i).first);
    }

    QJsonArray franquiciasDetectadas;
    for (auto it = franquicias.constBegin(); it != franquicias.constEnd(); ++it) {
        if (it.value() > 1) {
            franquiciasDetectadas.append(it.key());
        }
    }

    return QJsonObject{
        {"steam_id", biblioteca.value("steam_id").toString()},
        {"cantidad_juegos", biblioteca.value("cantidad").toInt()},
        {"horas_totales", horasTotales},
        {"top_juegos", topJuegos},
        {"generos_frecuentes", generosFrecuentes},
        {"franquicias_detectadas", franquiciasDetectadas},
        {"biblioteca_resumida", bibliotecaResumida}
    };
}

QString Recomendador::listaComoTexto(const QJsonArray &array) const
{
    QStringList valores;
    for (const QJsonValue &valor : array) {
        valores << valor.toString();
    }
    return valores.join(", ");
}

QJsonArray Recomendador::peliculasAJson(
    const std::vector<Pelicula> &peliculas
) const
{
    QJsonArray resultados;
    for (const Pelicula &pelicula : peliculas) {
        resultados.append(pelicula.get_atributos());
    }
    return resultados;
}

QString Recomendador::vectorPeliculasComoTexto(
    const std::vector<Pelicula> &peliculas
) const
{
    const QJsonArray peliculasJson = peliculasAJson(peliculas);
    return QString::fromUtf8(
        QJsonDocument(peliculasJson).toJson(QJsonDocument::Indented)
    );
}

bool Recomendador::tieneCantidadMinimaPorCategoria(
    const QJsonObject &recomendaciones,
    int cantidadMinima
) const
{
    return recomendaciones.value("peliculas").toArray().size() >= cantidadMinima
        && recomendaciones.value("libros").toArray().size() >= cantidadMinima
        && recomendaciones.value("juegos").toArray().size() >= cantidadMinima
        && recomendaciones.value("videos_youtube").toArray().size() >= cantidadMinima;
}

int Recomendador::calcularCantidadRecomendacionesAGenerar(
    int itemsSolicitados
) const
{
    const int minimoNecesario = std::max(1, itemsSolicitados);
    int cantidadConfig = DataManager::cargarConfiguracion(
        "RECOMMENDATION_POOL_SIZE",
        "15"
    ).toInt();

    if (cantidadConfig <= 0) {
        cantidadConfig = 15;
    }

    return std::max(cantidadConfig, minimoNecesario);
}

QString Recomendador::inferirFranquicia(const QString &nombreJuego) const
{
    const QStringList separadores = {":", "-", "|"};
    for (const QString &separador : separadores) {
        const int indice = nombreJuego.indexOf(separador);
        if (indice > 0) {
            return nombreJuego.left(indice).trimmed();
        }
    }

    const QStringList palabras = nombreJuego.split(' ', Qt::SkipEmptyParts);
    if (palabras.size() >= 2) {
        return palabras.at(0) + " " + palabras.at(1);
    }

    return nombreJuego.trimmed();
}

QStringList Recomendador::arrayALista(const QJsonArray &array) const
{
    QStringList lista;
    for (const QJsonValue &valor : array) {
        lista.append(valor.toString());
    }
    return lista;
}

Libro Recomendador::libroDesdeJson(const QJsonObject &objeto) const
{
    return Libro(
        objeto.value("titulo").toString(),
        objeto.value("autor").toString(),
        objeto.value("sinopsis").toString(),
        objeto.value("portada").toString()
    );
}

Pelicula Recomendador::peliculaDesdeJson(const QJsonObject &objeto) const
{
    return Pelicula(
        objeto.value("titulo").toString(),
        objeto.value("autor").toString(),
        objeto.value("sinopsis").toString(),
        objeto.value("anio").toInt(),
        objeto.value("portada").toString()
    );
}

Juego Recomendador::juegoDesdeJson(const QJsonObject &objeto) const
{
    return Juego(
        objeto.value("titulo").toString(),
        objeto.value("empresa").toString(),
        objeto.value("informacion").toString(),
        objeto.value("precio").isDouble()
            ? objeto.value("precio").toDouble()
            : -1.0,
        objeto.value("portada").toString()
    );
}

Video Recomendador::videoDesdeJson(const QJsonObject &objeto) const
{
    return Video(
        objeto.value("titulo").toString(),
        objeto.value("canal").toString(),
        objeto.value("descripcion").toString(),
        objeto.value("portada").toString()
    );
}

std::vector<Libro> Recomendador::librosDesdeArray(
    const QJsonArray &array
) const
{
    std::vector<Libro> libros;
    libros.reserve(array.size());

    for (const QJsonValue &valor : array) {
        libros.push_back(libroDesdeJson(valor.toObject()));
    }

    return libros;
}

std::vector<Pelicula> Recomendador::peliculasDesdeArray(
    const QJsonArray &array
) const
{
    std::vector<Pelicula> peliculas;
    peliculas.reserve(array.size());

    for (const QJsonValue &valor : array) {
        peliculas.push_back(peliculaDesdeJson(valor.toObject()));
    }

    return peliculas;
}

std::vector<Juego> Recomendador::juegosDesdeArray(
    const QJsonArray &array
) const
{
    std::vector<Juego> juegos;
    juegos.reserve(array.size());

    for (const QJsonValue &valor : array) {
        juegos.push_back(juegoDesdeJson(valor.toObject()));
    }

    return juegos;
}

std::vector<Video> Recomendador::videosDesdeArray(
    const QJsonArray &array
) const
{
    std::vector<Video> videos;
    videos.reserve(array.size());

    for (const QJsonValue &valor : array) {
        videos.push_back(videoDesdeJson(valor.toObject()));
    }

    return videos;
}

QJsonArray Recomendador::hidratarLibros(const QJsonArray &sugerencias)
{
    QJsonArray resultados;
    for (const QJsonValue &valor : sugerencias) {
        const QJsonObject sugerencia = valor.toObject();
        const QString query = sugerencia.value("query").toString(
            sugerencia.value("titulo").toString()
        );

        const QJsonObject detalle = buscarLibroDetalle(query, 1);
        QString error;
        if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
            error = detalle.value("error").toString();
        }

        Libro libro;
        const QJsonArray resultadosDetalle = detalle.value("resultados").toArray();
        if (!resultadosDetalle.isEmpty()) {
            libro = libroDesdeJson(resultadosDetalle.first().toObject());
        } else {
            libro = Libro(
                sugerencia.value("titulo").toString(),
                QString(),
                QString(),
                QString()
            );
        }

        resultados.append(
            anexarMetadatos(libro.get_atributos(), sugerencia, error)
        );
    }

    return resultados;
}

QJsonArray Recomendador::hidratarPeliculas(const QJsonArray &sugerencias)
{
    QJsonArray resultados;
    for (const QJsonValue &valor : sugerencias) {
        const QJsonObject sugerencia = valor.toObject();
        const QString query = sugerencia.value("query").toString(
            sugerencia.value("titulo").toString()
        );

        const QJsonObject detalle = buscarPeliculaDetalle(query, 1);
        QString error;
        if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
            error = detalle.value("error").toString();
        }

        Pelicula pelicula;
        const QJsonArray resultadosDetalle = detalle.value("resultados").toArray();
        if (!resultadosDetalle.isEmpty()) {
            pelicula = peliculaDesdeJson(resultadosDetalle.first().toObject());
        } else {
            pelicula = Pelicula(
                sugerencia.value("titulo").toString(),
                QString(),
                QString(),
                0,
                QString()
            );
        }

        resultados.append(
            anexarMetadatos(pelicula.get_atributos(), sugerencia, error)
        );
    }

    return resultados;
}

QJsonArray Recomendador::hidratarJuegos(const QJsonArray &sugerencias)
{
    QJsonArray resultados;
    for (const QJsonValue &valor : sugerencias) {
        const QJsonObject sugerencia = valor.toObject();
        const QString query = sugerencia.value("query").toString(
            sugerencia.value("titulo").toString()
        );

        const QJsonObject detalle = buscarJuegoDetalle(query, 1);
        QString error;
        if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
            error = detalle.value("error").toString();
        }

        Juego juego;
        const QJsonArray resultadosDetalle = detalle.value("resultados").toArray();
        if (!resultadosDetalle.isEmpty()) {
            juego = juegoDesdeJson(resultadosDetalle.first().toObject());
        } else {
            juego = Juego(
                sugerencia.value("titulo").toString(),
                QString(),
                QString(),
                -1.0,
                QString()
            );
        }

        resultados.append(
            anexarMetadatos(juego.get_atributos(), sugerencia, error)
        );
    }

    return resultados;
}

QJsonArray Recomendador::hidratarVideos(const QJsonArray &sugerencias)
{
    QJsonArray resultados;
    for (const QJsonValue &valor : sugerencias) {
        const QJsonObject sugerencia = valor.toObject();
        const QString query = sugerencia.value("query").toString(
            sugerencia.value("titulo").toString()
        );

        const QJsonObject detalle = buscarVideosDetalle(query, 1);
        QString error;
        if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
            error = detalle.value("error").toString();
        }

        Video video;
        const QJsonArray resultadosDetalle = detalle.value("resultados").toArray();
        if (!resultadosDetalle.isEmpty()) {
            video = videoDesdeJson(resultadosDetalle.first().toObject());
        } else {
            video = Video(
                sugerencia.value("titulo").toString(),
                QString(),
                QString(),
                QString()
            );
        }

        resultados.append(
            anexarMetadatos(video.get_atributos(), sugerencia, error)
        );
    }

    return resultados;
}

QJsonObject Recomendador::anexarMetadatos(
    const QJsonObject &atributos,
    const QJsonObject &sugerencia,
    const QString &error
) const
{
    QJsonObject resultado = atributos;
    resultado.insert("motivo", sugerencia.value("motivo").toString());
    resultado.insert("query_original", sugerencia.value("query").toString());
    if (!error.trimmed().isEmpty()) {
        resultado.insert("error_detalle", error);
    }
    return resultado;
}

QJsonObject Recomendador::cargarOGenerarRecomendaciones(
    const QString &steamId,
    const QJsonObject &biblioteca,
    int itemsPorCategoria
)
{
    QJsonObject cache = cargarCache(steamId);
    const int contador = obtenerContadorEjecuciones();

    const bool cacheValido = !cache.isEmpty()
        && cache.contains("recomendaciones")
        && tieneCantidadMinimaPorCategoria(
            cache.value("recomendaciones").toObject(),
            itemsPorCategoria
        );

    const bool debeGenerar = contador == 0 || contador % 20 == 0;

    if (cacheValido && !debeGenerar) {
        bool actualizado = false;
        if (!cache.contains("offset_peliculas")) {
            cache.insert("offset_peliculas", 0);
            actualizado = true;
        }
        if (!cache.contains("offset_libros")) {
            cache.insert("offset_libros", 0);
            actualizado = true;
        }
        if (!cache.contains("offset_juegos")) {
            cache.insert("offset_juegos", 0);
            actualizado = true;
        }
        if (!cache.contains("offset_videos_youtube")) {
            cache.insert("offset_videos_youtube", 0);
            actualizado = true;
        }
        if (actualizado) {
            guardarCache(steamId, cache);
        }
        return cache;
    }

    const QJsonObject contexto = construirContextoSteam(biblioteca);
    const QJsonObject respuestaChatGpt =
        generarRecomendacionesChatGpt(contexto, itemsPorCategoria);
    if (respuestaChatGpt.value("ok").isBool()
        && !respuestaChatGpt.value("ok").toBool()) {
        return respuestaChatGpt;
    }

    const QJsonObject recomendaciones =
        respuestaChatGpt.value("recomendaciones").toObject();
    if (recomendaciones.isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "ChatGPT no devolvio recomendaciones utilizables."
        );
    }

    cache = QJsonObject{
        {"steam_id", steamId},
        {"recomendaciones", recomendaciones},
        {"fecha_generacion", QDateTime::currentDateTime().toString(Qt::ISODate)},
        {"offset_peliculas", 0},
        {"offset_libros", 0},
        {"offset_juegos", 0},
        {"offset_videos_youtube", 0}
    };
    guardarCache(steamId, cache);

    return cache;
}

QJsonObject Recomendador::generarRecomendacionesChatGpt(
    const QJsonObject &contextoSteam,
    int itemsPorCategoria
)
{
    const QString apiKey = DataManager::cargarConfiguracion("OPENAI_API_KEY");
    if (apiKey.trimmed().isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "Falta OPENAI_API_KEY para generar recomendaciones."
        );
    }

    if (contextoSteam.isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "No hay contexto de Steam para enviar a ChatGPT."
        );
    }

    const QString modelo = DataManager::cargarConfiguracion(
        "OPENAI_MODEL",
        "gpt-5-mini"
    );
    const int requestTimeoutSegundos = std::max(
        1,
        DataManager::cargarConfiguracion("REQUEST_TIMEOUT", "180").toInt()
    );
    const bool usarSegundoPlano =
        esVerdadero(DataManager::cargarConfiguracion("OPENAI_BACKGROUND"))
        || requestTimeoutSegundos <= 90;

    const SolicitudChatGpt solicitud(
        contextoSteam,
        itemsPorCategoria,
        modelo,
        QString(),
        QString("recomendaciones_usuario"),
        usarSegundoPlano,
        QString(),
        true
    );

    return ejecutarSolicitudChatGpt(solicitud);
}

QJsonObject Recomendador::ejecutarSolicitudChatGpt(
    const SolicitudChatGpt &solicitud
)
{
    const QString apiKey = DataManager::cargarConfiguracion("OPENAI_API_KEY");
    if (apiKey.trimmed().isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "Falta OPENAI_API_KEY para consultar ChatGPT."
        );
    }

    if (solicitud.get_ejecutar_en_segundo_plano()) {
        return ejecutarSolicitudChatGptEnSegundoPlano(solicitud);
    }

    const QString respuesta = apiChatGpt.post("responses", solicitud.crearPayload());
    const QJsonObject respuestaObjeto = parsearObjetoJson(respuesta);
    if (esErrorTimeoutChatGpt(respuestaObjeto)) {
        const SolicitudChatGpt solicitudEnSegundoPlano(
            solicitud.get_contexto_steam(),
            solicitud.get_items_por_categoria(),
            solicitud.get_modelo(),
            solicitud.get_instrucciones(),
            solicitud.get_nombre_formato(),
            true,
            solicitud.get_mensaje_usuario(),
            solicitud.get_usar_formato_estructurado()
        );
        return ejecutarSolicitudChatGptEnSegundoPlano(solicitudEnSegundoPlano);
    }

    return solicitud.interpretarRespuesta(respuesta);
}

QJsonObject Recomendador::ejecutarSolicitudChatGptEnSegundoPlano(
    const SolicitudChatGpt &solicitud
)
{
    const QString respuesta = apiChatGpt.post("responses", solicitud.crearPayload());
    const QJsonObject respuestaObjeto = parsearObjetoJson(respuesta);

    if (respuestaObjeto.isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "No fue posible interpretar la respuesta inicial de ChatGPT.",
            respuesta
        );
    }

    if (respuestaObjeto.value("ok").isBool() && !respuestaObjeto.value("ok").toBool()) {
        return respuestaObjeto;
    }

    const QString status = respuestaObjeto.value("status").toString();
    if (status.compare("completed", Qt::CaseInsensitive) == 0) {
        return solicitud.interpretarRespuesta(respuesta);
    }

    const QString responseId = respuestaObjeto.value("id").toString();
    if (responseId.trimmed().isEmpty()) {
        return crearErrorObjeto(
            "chatgpt",
            "ChatGPT no devolvio un id para consultar la respuesta en segundo plano.",
            respuesta
        );
    }

    const int intervaloMs = std::max(
        250,
        DataManager::cargarConfiguracion(
            "OPENAI_BACKGROUND_POLL_INTERVAL_MS",
            "2000"
        ).toInt()
    );
    const int tiempoMaximoSegundos = std::max(
        30,
        DataManager::cargarConfiguracion(
            "OPENAI_BACKGROUND_MAX_WAIT",
            "600"
        ).toInt()
    );

    const QJsonObject respuestaFinal = esperarRespuestaChatGptEnSegundoPlano(
        responseId,
        intervaloMs,
        tiempoMaximoSegundos
    );
    if (respuestaFinal.value("ok").isBool() && !respuestaFinal.value("ok").toBool()) {
        return respuestaFinal;
    }

    return solicitud.interpretarRespuesta(
        QString::fromUtf8(
            QJsonDocument(respuestaFinal).toJson(QJsonDocument::Compact)
        )
    );
}

QJsonObject Recomendador::esperarRespuestaChatGptEnSegundoPlano(
    const QString &responseId,
    int intervaloMs,
    int tiempoMaximoSegundos
)
{
    QElapsedTimer tiempoTranscurrido;
    tiempoTranscurrido.start();

    while (tiempoTranscurrido.elapsed() < tiempoMaximoSegundos * 1000) {
        const QString respuesta = apiChatGpt.get("responses/" + responseId);
        const QJsonObject respuestaObjeto = parsearObjetoJson(respuesta);

        if (respuestaObjeto.isEmpty()) {
            return crearErrorObjeto(
                "chatgpt",
                "No fue posible interpretar la respuesta en segundo plano de ChatGPT.",
                respuesta
            );
        }

        if (respuestaObjeto.value("ok").isBool()
            && !respuestaObjeto.value("ok").toBool()) {
            return respuestaObjeto;
        }

        const QString status = respuestaObjeto.value("status").toString();
        if (!estadoRespuestaEnProgreso(status)) {
            return respuestaObjeto;
        }

        QThread::msleep(static_cast<unsigned long>(intervaloMs));
    }

    return crearErrorObjeto(
        "chatgpt",
        "Se agoto el tiempo esperando la respuesta en segundo plano de ChatGPT.",
        responseId
    );
}

bool Recomendador::esErrorTimeoutChatGpt(const QJsonObject &respuesta) const
{
    return respuesta.value("api").toString() == "chatgpt"
        && respuesta.value("ok").isBool()
        && !respuesta.value("ok").toBool()
        && respuesta.value("error").toString().contains(
            "timed out",
            Qt::CaseInsensitive
        );
}

QJsonObject Recomendador::crearErrorObjeto(
    const QString &api,
    const QString &mensaje,
    const QString &detalle
) const
{
    QJsonObject error;
    error.insert("api", api);
    error.insert("ok", false);
    error.insert("error", mensaje);
    if (!detalle.trimmed().isEmpty()) {
        error.insert("detalle", detalle);
    }
    return error;
}

QJsonObject Recomendador::obtenerDetalleJuegoSteam(int appId)
{
    if (appId <= 0) {
        return QJsonObject();
    }

    const QString language =
        DataManager::cargarConfiguracion("STEAM_LANGUAGE", "spanish");
    const QString countryCode =
        DataManager::cargarConfiguracion("COUNTRY_CODE", "ar");

    QUrlQuery query;
    query.addQueryItem("appids", QString::number(appId));
    query.addQueryItem("l", language);
    query.addQueryItem("cc", countryCode);

    const QString respuesta = apiSteamJuegos.get(
        "appdetails?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument documento =
        QJsonDocument::fromJson(respuesta.toUtf8());
    if (!documento.isObject()) {
        return QJsonObject();
    }

    const QJsonObject raiz = documento.object().value(
        QString::number(appId)
    ).toObject();
    if (!raiz.value("success").toBool()) {
        return QJsonObject();
    }

    const QJsonObject data = raiz.value("data").toObject();
    const QJsonArray developers = data.value("developers").toArray();

    QString empresa;
    if (!developers.isEmpty()) {
        empresa = developers.first().toString();
    } else {
        const QJsonArray publishers = data.value("publishers").toArray();
        if (!publishers.isEmpty()) {
            empresa = publishers.first().toString();
        }
    }

    QJsonArray generos;
    for (const QJsonValue &valorGenero : data.value("genres").toArray()) {
        const QString genero =
            valorGenero.toObject().value("description").toString();
        if (!genero.isEmpty()) {
            generos.append(genero);
        }
    }

    QJsonArray categorias;
    for (const QJsonValue &valorCategoria : data.value("categories").toArray()) {
        const QString categoria =
            valorCategoria.toObject().value("description").toString();
        if (!categoria.isEmpty()) {
            categorias.append(categoria);
        }
    }

    QJsonObject detalle;
    detalle.insert("empresa", empresa);
    detalle.insert(
        "informacion",
        data.value("short_description").toString("Sin informacion.")
    );
    detalle.insert(
        "descripcion",
        data.value("short_description").toString("Sin descripcion.")
    );
    detalle.insert("portada", data.value("header_image").toString());
    detalle.insert("generos", generos);
    detalle.insert("categorias", categorias);

    const QJsonObject priceOverview = data.value("price_overview").toObject();
    const double precio = obtenerPrecioFinal(priceOverview);
    if (precio >= 0.0) {
        detalle.insert("precio", precio);
    }

    return detalle;
}

QJsonObject Recomendador::buscarLibroDetalle(
    const QString &titulo,
    int maxResults
)
{
    const QString tituloNormalizado = titulo.trimmed();
    if (tituloNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "libros",
            "Debes indicar un titulo para buscar libros."
        );
    }

    QUrlQuery query;
    query.addQueryItem("q", "intitle:" + tituloNormalizado);
    query.addQueryItem("maxResults", QString::number(std::max(1, maxResults)));
    query.addQueryItem("printType", "books");

    const QString jsonResponse = apiLibros.get(
        "volumes?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorObjeto(
            "libros",
            "Error al parsear la respuesta JSON de libros.",
            jsonResponse
        );
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return obj;
    }

    const QJsonArray items = obj.value("items").toArray();
    QJsonArray resultados;

    for (const QJsonValue &item : items) {
        const QJsonObject volumeInfo = item.toObject().value(
            "volumeInfo"
        ).toObject();
        const QJsonObject imageLinks = volumeInfo.value("imageLinks").toObject();

        QStringList autores;
        for (const QJsonValue &autor : volumeInfo.value("authors").toArray()) {
            autores << autor.toString();
        }

        QString portada = imageLinks.value("thumbnail").toString(
            imageLinks.value("smallThumbnail").toString()
        );
        portada.replace("http://", "https://");

        resultados.append(
            QJsonObject{
                {"titulo", volumeInfo.value("title").toString("Sin titulo")},
                {"autor", autores.join(", ")},
                {"sinopsis", volumeInfo.value("description").toString()},
                {"portada", portada}
            }
        );
    }

    return QJsonObject{
        {"api", "libros"},
        {"ok", true},
        {"query", tituloNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Libro Recomendador::buscarPrimerLibro(
    const QString &titulo,
    int maxResults
)
{
    const QJsonObject detalle = buscarLibroDetalle(titulo, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Libro(titulo.trimmed(), QString(), QString(), QString());
    }

    return libroDesdeJson(resultados.first().toObject());
}

QJsonObject Recomendador::buscarPeliculaDetalle(
    const QString &nombre,
    int maxResults
)
{
    const QString nombreNormalizado = nombre.trimmed();
    if (nombreNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "peliculas",
            "Debes indicar un nombre de pelicula."
        );
    }

    QUrlQuery query;
    query.addQueryItem("query", nombreNormalizado);
    query.addQueryItem("language", "es-ES");

    const QString jsonResponse = apiPeliculas.get(
        "search/movie?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorObjeto(
            "peliculas",
            "Error al parsear la respuesta JSON de peliculas.",
            jsonResponse
        );
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return obj;
    }

    const QJsonArray results = obj.value("results").toArray();
    QJsonArray resultados;

    for (int i = 0; i < results.size() && i < std::max(1, maxResults); ++i) {
        const QJsonObject movieObj = results.at(i).toObject();
        const int peliculaId = movieObj.value("id").toInt();

        const QString posterPath = movieObj.value("poster_path").toString();
        const QString releaseDate = movieObj.value("release_date").toString();
        const int anio = releaseDate.left(4).toInt();

        resultados.append(
            QJsonObject{
                {"titulo", movieObj.value("title").toString("Sin titulo")},
                {"autor", obtenerDirectorPelicula(peliculaId)},
                {"sinopsis", movieObj.value("overview").toString()},
                {"anio", anio},
                {"portada", posterPath.isEmpty()
                                ? QString()
                                : "https://image.tmdb.org/t/p/w500" + posterPath}
            }
        );
    }

    return QJsonObject{
        {"api", "peliculas"},
        {"ok", true},
        {"query", nombreNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Pelicula Recomendador::buscarPrimeraPelicula(
    const QString &nombre,
    int maxResults
)
{
    const QJsonObject detalle = buscarPeliculaDetalle(nombre, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Pelicula(nombre.trimmed(), QString(), QString(), 0, QString());
    }

    return peliculaDesdeJson(resultados.first().toObject());
}

QString Recomendador::obtenerDirectorPelicula(int peliculaId)
{
    if (peliculaId <= 0) {
        return QString();
    }

    const QString jsonResponse = apiPeliculas.get(
        QString("movie/%1/credits?language=es-ES").arg(peliculaId)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return QString();
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return QString();
    }

    const QJsonArray crew = obj.value("crew").toArray();
    for (const QJsonValue &crewMember : crew) {
        const QJsonObject persona = crewMember.toObject();
        if (persona.value("job").toString().compare(
                "Director",
                Qt::CaseInsensitive
            ) == 0) {
            return persona.value("name").toString();
        }
    }

    return QString();
}

QJsonObject Recomendador::buscarJuegoDetalle(
    const QString &texto,
    int maxResults
)
{
    const QString textoNormalizado = texto.trimmed();
    if (textoNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "steam_juegos",
            "Debes indicar un texto para buscar juegos."
        );
    }

    const QString language =
        DataManager::cargarConfiguracion("STEAM_LANGUAGE", "spanish");
    const QString countryCode =
        DataManager::cargarConfiguracion("COUNTRY_CODE", "ar");

    QUrlQuery query;
    query.addQueryItem("term", textoNormalizado);
    query.addQueryItem("l", language);
    query.addQueryItem("cc", countryCode);
    query.addQueryItem("json", "1");

    const QString respuesta = apiSteamJuegos.get(
        "storesearch?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument documento =
        QJsonDocument::fromJson(respuesta.toUtf8());
    if (!documento.isObject()) {
        return crearErrorObjeto(
            "steam_juegos",
            "Error al parsear la respuesta JSON de Steam juegos.",
            respuesta
        );
    }

    const QJsonObject raiz = documento.object();
    if (raiz.value("ok").isBool() && !raiz.value("ok").toBool()) {
        return raiz;
    }

    const QJsonArray items = raiz.value("items").toArray();
    QJsonArray resultados;

    for (int i = 0; i < items.size() && i < std::max(1, maxResults); ++i) {
        const QJsonObject item = items.at(i).toObject();
        const int appId = item.value("id").toInt();
        const QJsonObject detalle = obtenerDetalleJuegoSteam(appId);

        const QString empresa = detalle.value("empresa").toString("Sin empresa");
        const QString informacion = detalle.value("informacion").toString();
        const double precio =
            detalle.contains("precio")
            ? detalle.value("precio").toDouble(-1.0)
            : obtenerPrecioFinal(item.value("price").toObject());

        QJsonObject juego;
        juego.insert("appid", appId);
        juego.insert("titulo", item.value("name").toString("Sin titulo"));
        juego.insert("empresa", empresa);
        juego.insert("informacion", informacion);
        if (precio >= 0.0) {
            juego.insert("precio", precio);
        } else {
            juego.insert("precio", QJsonValue::Null);
        }
        juego.insert(
            "portada",
            detalle.value("portada").toString(
                item.value("tiny_image").toString()
            )
        );

        resultados.append(juego);
    }

    return QJsonObject{
        {"api", "steam_juegos"},
        {"ok", true},
        {"query", textoNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Juego Recomendador::buscarPrimerJuego(
    const QString &texto,
    int maxResults
)
{
    const QJsonObject detalle = buscarJuegoDetalle(texto, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Juego(texto.trimmed(), QString(), QString(), -1.0, QString());
    }

    return juegoDesdeJson(resultados.first().toObject());
}

double Recomendador::obtenerPrecioFinal(const QJsonObject &priceData) const
{
    if (priceData.isEmpty() || !priceData.value("final").isDouble()) {
        return -1.0;
    }

    return priceData.value("final").toDouble() / 100.0;
}

QJsonObject Recomendador::buscarVideosDetalle(
    const QString &texto,
    int maxResults
)
{
    const QString textoNormalizado = texto.trimmed();
    if (textoNormalizado.isEmpty()) {
        return crearErrorObjeto(
            "youtube",
            "Debes indicar un texto para buscar videos."
        );
    }

    QUrlQuery query;
    query.addQueryItem("part", "snippet");
    query.addQueryItem("type", "video");
    query.addQueryItem("maxResults", QString::number(std::max(1, maxResults)));
    query.addQueryItem("q", textoNormalizado);

    const QString jsonResponse = apiYoutube.get(
        "search?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorObjeto(
            "youtube",
            "Error al parsear la respuesta JSON de YouTube.",
            jsonResponse
        );
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return obj;
    }

    const QJsonArray items = obj.value("items").toArray();
    QJsonArray resultados;

    for (const QJsonValue &item : items) {
        const QJsonObject itemObj = item.toObject();
        const QJsonObject snippet = itemObj.value("snippet").toObject();
        const QJsonObject thumbnails = snippet.value("thumbnails").toObject();

        QString portada =
            thumbnails.value("high").toObject().value("url").toString();
        if (portada.isEmpty()) {
            portada =
                thumbnails.value("medium").toObject().value("url").toString();
        }
        if (portada.isEmpty()) {
            portada =
                thumbnails.value("default").toObject().value("url").toString();
        }
        portada.replace("http://", "https://");

        resultados.append(
            QJsonObject{
                {"titulo", snippet.value("title").toString("Sin titulo")},
                {"canal", snippet.value("channelTitle").toString()},
                {"descripcion", snippet.value("description").toString()},
                {"portada", portada}
            }
        );
    }

    return QJsonObject{
        {"api", "youtube"},
        {"ok", true},
        {"query", textoNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Video Recomendador::buscarPrimerVideo(
    const QString &texto,
    int maxResults
)
{
    const QJsonObject detalle = buscarVideosDetalle(texto, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Video(texto.trimmed(), QString(), QString(), QString());
    }

    return videoDesdeJson(resultados.first().toObject());
}

QJsonArray Recomendador::obtenerBloqueSecuencial(
    const QJsonArray &todos,
    int cantidad,
    int &offset
) const
{
    QJsonArray resultado;
    if (todos.isEmpty() || cantidad <= 0) {
        return resultado;
    }

    const int total = todos.size();
    if (offset < 0 || offset >= total) {
        offset = 0;
    }

    for (int i = 0; i < cantidad && i < total; ++i) {
        resultado.append(todos.at((offset + i) % total));
    }

    offset = (offset + cantidad) % total;
    return resultado;
}

QJsonArray Recomendador::seleccionarBloqueCategoria(
    const QString &steamId,
    QJsonObject &cache,
    const QString &categoria,
    const QString &offsetKey,
    int cantidad
)
{
    QJsonObject recomendaciones = cache.value("recomendaciones").toObject();
    const QJsonArray todos = recomendaciones.value(categoria).toArray();
    int offset = cache.value(offsetKey).toInt(0);

    const QJsonArray bloque = obtenerBloqueSecuencial(todos, cantidad, offset);
    cache.insert(offsetKey, offset);
    guardarCache(steamId, cache);
    return bloque;
}

int Recomendador::obtenerContadorEjecuciones()
{
    const QString archivoContador =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/recomendador_contador.json";

    QFile archivo(archivoContador);
    if (!archivo.open(QIODevice::ReadOnly)) {
        return 0;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(archivo.readAll());
    archivo.close();

    if (!doc.isObject()) {
        return 0;
    }

    return doc.object().value("contador").toInt(0);
}

void Recomendador::incrementarContadorEjecuciones()
{
    const QString dirApp =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirApp);

    const QString archivoContador = dirApp + "/recomendador_contador.json";

    int contador = obtenerContadorEjecuciones();
    ++contador;

    QJsonObject obj;
    obj.insert("contador", contador);
    obj.insert(
        "ultima_actualizacion",
        QDateTime::currentDateTime().toString(Qt::ISODate)
    );

    QFile archivo(archivoContador);
    if (archivo.open(QIODevice::WriteOnly)) {
        archivo.write(QJsonDocument(obj).toJson());
        archivo.close();
    }
}

QJsonObject Recomendador::cargarCache(const QString &steamId)
{
    const QString archivoCache = obtenerArchivoCache(steamId);
    QFile archivo(archivoCache);
    if (!archivo.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }

    const QJsonDocument doc = QJsonDocument::fromJson(archivo.readAll());
    archivo.close();

    if (!doc.isObject()) {
        return QJsonObject();
    }

    return doc.object();
}

void Recomendador::guardarCache(
    const QString &steamId,
    const QJsonObject &cache
)
{
    const QString dirApp =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirApp);

    const QString archivoCache = obtenerArchivoCache(steamId);
    QFile archivo(archivoCache);
    if (archivo.open(QIODevice::WriteOnly)) {
        archivo.write(QJsonDocument(cache).toJson());
        archivo.close();
    }
}

QJsonObject Recomendador::obtenerRecomendacionesEnCache(const QString &steamId)
{
    const QJsonObject cache = cargarCache(steamId);
    return cache.value("recomendaciones").toObject();
}

QString Recomendador::obtenerArchivoCache(const QString &steamId) const
{
    const QString dirApp =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dirApp + "/cache_recomendaciones_" + steamId + ".json";
}
