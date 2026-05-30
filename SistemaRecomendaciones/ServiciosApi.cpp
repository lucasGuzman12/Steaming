#include "ServiciosApi.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>

namespace
{
QJsonObject crearErrorApi(
    const QString &api,
    const QString &mensaje,
    const QString &detalle = QString()
)
{
    QJsonObject error;
    error.insert("api", api);
    error.insert("ok", false);
    error.insert("error", mensaje);
    if (!detalle.isEmpty()) {
        error.insert("detalle", detalle);
    }
    return error;
}

QJsonObject crearItemSchema()
{
    QJsonObject propiedades;
    propiedades.insert("titulo", QJsonObject{{"type", "string"}});
    propiedades.insert("motivo", QJsonObject{{"type", "string"}});
    propiedades.insert("query", QJsonObject{{"type", "string"}});

    QJsonArray requeridos;
    requeridos.append("titulo");
    requeridos.append("motivo");
    requeridos.append("query");

    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", propiedades);
    schema.insert("required", requeridos);
    schema.insert("additionalProperties", false);
    return schema;
}

QJsonObject crearArraySchema(const QJsonObject &itemSchema)
{
    QJsonObject schema;
    schema.insert("type", "array");
    schema.insert("items", itemSchema);
    return schema;
}
} // namespace

QJsonObject ServicioSteamBiblioteca::obtenerBiblioteca(
    const QString &steamId,
    int enrichTop
)
{
    const QString steamIdNormalizado = steamId.trimmed();
    if (steamIdNormalizado.isEmpty()) {
        return crearErrorObjeto("Debes indicar un Steam ID valido.");
    }

    QUrlQuery query;
    query.addQueryItem("steamid", steamIdNormalizado);
    query.addQueryItem("include_appinfo", "true");
    query.addQueryItem("include_played_free_games", "true");
    query.addQueryItem("format", "json");

    const QString respuesta = apiBiblioteca.get(
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
                obtenerDetalleJuego(juegoBase.value("appid").toInt());
            for (auto it = detalle.constBegin(); it != detalle.constEnd(); ++it) {
                juego.insert(it.key(), it.value());
            }
        }

        juegos.append(juego);
        ++indice;
    }

    QJsonObject resultado;
    resultado.insert("api", "steam_biblioteca");
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

QJsonObject ServicioSteamBiblioteca::obtenerDetalleJuego(int appId)
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

    const QString respuesta = apiSteamStore.get(
        "appdetails?" + query.toString(QUrl::FullyEncoded)
    );

    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(
        respuesta.toUtf8(),
        &error
    );
    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        return QJsonObject();
    }

    const QJsonObject detalleApp = documento.object().value(
        QString::number(appId)
    ).toObject();
    if (!detalleApp.value("success").toBool()) {
        return QJsonObject();
    }

    const QJsonObject data = detalleApp.value("data").toObject();
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

    QJsonObject resultado;
    resultado.insert("portada", data.value("header_image").toString());
    resultado.insert(
        "descripcion",
        data.value("short_description").toString("Sin descripcion.")
    );
    resultado.insert("generos", generos);
    resultado.insert("categorias", categorias);
    return resultado;
}

QJsonObject ServicioSteamBiblioteca::crearErrorObjeto(
    const QString &mensaje,
    const QString &detalle
) const
{
    return crearErrorApi("steam_biblioteca", mensaje, detalle);
}

QString ServicioSteamJuegos::buscarJuegos(const QString &texto)
{
    const QJsonObject detalle = buscarJuegoDetalle(texto, 5);
    if (detalle.value("ok").isBool() && !detalle.value("ok").toBool()) {
        return QString::fromUtf8(
            QJsonDocument(detalle).toJson(QJsonDocument::Indented)
        );
    }

    QString resultado;
    const QJsonArray juegos = detalle.value("resultados").toArray();
    for (const QJsonValue &valor : juegos) {
        const QJsonObject juego = valor.toObject();
        resultado += juego.value("titulo").toString() + "\n";
        resultado += juego.value("empresa").toString() + "\n";
        resultado += juego.value("informacion").toString() + "\n";
        resultado += "-----\n";
    }

    if (resultado.isEmpty()) {
        resultado = "No se encontraron juegos.";
    }

    return resultado;
}

QJsonObject ServicioSteamJuegos::buscarJuegoDetalle(
    const QString &texto,
    int maxResults
)
{
    const QString textoNormalizado = texto.trimmed();
    if (textoNormalizado.isEmpty()) {
        return crearErrorApi(
            "steam_juegos",
            "Debes indicar un texto para buscar juegos."
        );
    }

    const int cantidadMaxima = std::max(1, maxResults);

    const QString language =
        DataManager::cargarConfiguracion("STEAM_LANGUAGE", "spanish");
    const QString countryCode =
        DataManager::cargarConfiguracion("COUNTRY_CODE", "ar");

    QUrlQuery query;
    query.addQueryItem("term", textoNormalizado);
    query.addQueryItem("l", language);
    query.addQueryItem("cc", countryCode);
    query.addQueryItem("json", "1");

    const QString respuesta = api.get(
        "storesearch?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument documento =
        QJsonDocument::fromJson(respuesta.toUtf8());
    if (!documento.isObject()) {
        return crearErrorApi(
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

    for (int i = 0; i < items.size() && i < cantidadMaxima; ++i) {
        const QJsonObject item = items.at(i).toObject();
        const int appId = item.value("id").toInt();
        const QJsonObject detalle = obtenerDetalleJuego(appId);

        const QString empresa =
            detalle.value("empresa").toString("Sin empresa");
        const QString informacion =
            detalle.value("informacion").toString();
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
        {"query", textoNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Juego ServicioSteamJuegos::buscarPrimerJuego(
    const QString &texto,
    int maxResults
)
{
    const QJsonObject detalle = buscarJuegoDetalle(texto, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Juego(texto.trimmed(), QString(), QString(), -1.0, QString());
    }

    const QJsonObject juego = resultados.first().toObject();
    return Juego(
        juego.value("titulo").toString(),
        juego.value("empresa").toString(),
        juego.value("informacion").toString(),
        juego.value("precio").isDouble()
            ? juego.value("precio").toDouble()
            : -1.0,
        juego.value("portada").toString()
    );
}

QJsonObject ServicioSteamJuegos::obtenerDetalleJuego(int appId)
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

    const QString respuesta = api.get(
        "appdetails?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument documento =
        QJsonDocument::fromJson(respuesta.toUtf8());
    if (!documento.isObject()) {
        return QJsonObject();
    }

    const QJsonObject raiz =
        documento.object().value(QString::number(appId)).toObject();
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

    QJsonObject detalle;
    detalle.insert("empresa", empresa);
    detalle.insert(
        "informacion",
        data.value("short_description").toString("Sin informacion.")
    );
    detalle.insert("portada", data.value("header_image").toString());

    const QJsonObject priceOverview =
        data.value("price_overview").toObject();
    const double precio = obtenerPrecioFinal(priceOverview);
    if (precio >= 0.0) {
        detalle.insert("precio", precio);
    }

    return detalle;
}

double ServicioSteamJuegos::obtenerPrecioFinal(
    const QJsonObject &priceData
) const
{
    if (priceData.isEmpty() || !priceData.value("final").isDouble()) {
        return -1.0;
    }

    return priceData.value("final").toDouble() / 100.0;
}

QString ServicioLibros::buscarLibro(const QString &titulo)
{
    const QString tituloNormalizado = titulo.trimmed();
    if (tituloNormalizado.isEmpty()) {
        return "Debes indicar un titulo para buscar libros.";
    }

    QUrlQuery query;
    query.addQueryItem("q", "intitle:" + tituloNormalizado);
    query.addQueryItem("maxResults", "5");
    query.addQueryItem("printType", "books");

    const QString jsonResponse = api.get(
        "volumes?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return "Error al parsear la respuesta JSON.";
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return QString::fromUtf8(
            QJsonDocument(obj).toJson(QJsonDocument::Indented)
        );
    }

    const QJsonArray items = obj.value("items").toArray();
    if (items.isEmpty()) {
        return "No se encontraron libros.";
    }

    QString result;
    for (const QJsonValue &item : items) {
        const QJsonObject volumeInfo = item.toObject().value(
            "volumeInfo"
        ).toObject();
        const QJsonObject imageLinks = volumeInfo.value("imageLinks").toObject();

        const QString title = volumeInfo.value("title").toString();
        QStringList authors;
        for (const QJsonValue &author : volumeInfo.value("authors").toArray()) {
            authors << author.toString();
        }

        const QString publisher = volumeInfo.value("publisher").toString();
        const QString publishedDate = volumeInfo.value("publishedDate").toString();
        const QString description = volumeInfo.value("description").toString();

        QString coverUrl = imageLinks.value("thumbnail").toString();
        if (coverUrl.isEmpty()) {
            coverUrl = imageLinks.value("smallThumbnail").toString();
        }
        coverUrl.replace("http://", "https://");

        result += "Titulo: " + title + "\n";
        result += "Autores: " + authors.join(", ") + "\n";
        result += "Editorial: " + publisher + "\n";
        result += "Fecha: " + publishedDate + "\n";
        result += "Descripcion: " + description.left(100) + "...\n";
        result += "Portada: "
            + (coverUrl.isEmpty() ? QString("No tiene imagen") : coverUrl)
            + "\n";
        result += QString("-").repeated(50) + "\n";
    }

    return result;
}

QJsonObject ServicioLibros::buscarLibroDetalle(
    const QString &titulo,
    int maxResults
)
{
    const QString tituloNormalizado = titulo.trimmed();
    if (tituloNormalizado.isEmpty()) {
        return crearErrorApi(
            "libros",
            "Debes indicar un titulo para buscar libros."
        );
    }

    QUrlQuery query;
    query.addQueryItem("q", "intitle:" + tituloNormalizado);
    query.addQueryItem("maxResults", QString::number(std::max(1, maxResults)));
    query.addQueryItem("printType", "books");

    const QString jsonResponse = api.get(
        "volumes?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorApi(
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
        {"query", tituloNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Libro ServicioLibros::buscarPrimerLibro(
    const QString &titulo,
    int maxResults
)
{
    const QJsonObject detalle = buscarLibroDetalle(titulo, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Libro(titulo.trimmed(), QString(), QString(), QString());
    }

    const QJsonObject libro = resultados.first().toObject();
    return Libro(
        libro.value("titulo").toString(),
        libro.value("autor").toString(),
        libro.value("sinopsis").toString(),
        libro.value("portada").toString()
    );
}

QString ServicioPeliculas::buscarPelicula(const QString &nombre)
{
    const QString nombreNormalizado = nombre.trimmed();
    if (nombreNormalizado.isEmpty()) {
        return "Debes indicar un nombre de pelicula.";
    }

    QUrlQuery query;
    query.addQueryItem("query", nombreNormalizado);
    query.addQueryItem("language", "es-ES");

    const QString jsonResponse = api.get(
        "search/movie?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return "Error al parsear la respuesta JSON.";
    }

    const QJsonObject obj = doc.object();
    if (obj.value("ok").isBool() && !obj.value("ok").toBool()) {
        return QString::fromUtf8(
            QJsonDocument(obj).toJson(QJsonDocument::Indented)
        );
    }

    const QJsonArray results = obj.value("results").toArray();
    if (results.isEmpty()) {
        return "No se encontraron peliculas.";
    }

    QString result;
    for (const QJsonValue &movie : results) {
        const QJsonObject movieObj = movie.toObject();

        const QString title = movieObj.value("title").toString();
        const QString overview = movieObj.value("overview").toString();
        const QString releaseDate = movieObj.value("release_date").toString();
        const QString posterPath = movieObj.value("poster_path").toString();

        QString posterUrl;
        if (!posterPath.isEmpty()) {
            posterUrl = "https://image.tmdb.org/t/p/w500" + posterPath;
        } else {
            posterUrl = "No tiene imagen";
        }

        result += title + "\n";
        result += overview + "\n";
        result += releaseDate + "\n";
        result += posterUrl + "\n";
        result += "-----\n";
    }

    return result;
}

QJsonObject ServicioPeliculas::buscarPeliculaDetalle(
    const QString &nombre,
    int maxResults
)
{
    const QString nombreNormalizado = nombre.trimmed();
    if (nombreNormalizado.isEmpty()) {
        return crearErrorApi(
            "peliculas",
            "Debes indicar un nombre de pelicula."
        );
    }

    const int cantidadMaxima = std::max(1, maxResults);

    QUrlQuery query;
    query.addQueryItem("query", nombreNormalizado);
    query.addQueryItem("language", "es-ES");

    const QString jsonResponse = api.get(
        "search/movie?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorApi(
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

    for (int i = 0; i < results.size() && i < cantidadMaxima; ++i) {
        const QJsonObject movieObj = results.at(i).toObject();
        const int peliculaId = movieObj.value("id").toInt();

        const QString posterPath = movieObj.value("poster_path").toString();
        const QString releaseDate = movieObj.value("release_date").toString();
        const int anio = releaseDate.left(4).toInt();

        resultados.append(
            QJsonObject{
                {"titulo", movieObj.value("title").toString("Sin titulo")},
                {"autor", obtenerDirector(peliculaId)},
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
        {"query", nombreNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Pelicula ServicioPeliculas::buscarPrimeraPelicula(
    const QString &nombre,
    int maxResults
)
{
    const QJsonObject detalle = buscarPeliculaDetalle(nombre, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Pelicula(nombre.trimmed(), QString(), QString(), 0, QString());
    }

    const QJsonObject pelicula = resultados.first().toObject();
    return Pelicula(
        pelicula.value("titulo").toString(),
        pelicula.value("autor").toString(),
        pelicula.value("sinopsis").toString(),
        pelicula.value("anio").toInt(),
        pelicula.value("portada").toString()
    );
}

QString ServicioPeliculas::obtenerDirector(int peliculaId)
{
    if (peliculaId <= 0) {
        return QString();
    }

    const QString jsonResponse = api.get(
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

QJsonObject ServicioYoutube::buscarVideos(
    const QString &texto,
    int maxResults
)
{
    const QString textoNormalizado = texto.trimmed();
    if (textoNormalizado.isEmpty()) {
        return crearErrorApi(
            "youtube",
            "Debes indicar un texto para buscar videos."
        );
    }

    QUrlQuery query;
    query.addQueryItem("part", "snippet");
    query.addQueryItem("type", "video");
    query.addQueryItem("maxResults", QString::number(std::max(1, maxResults)));
    query.addQueryItem("q", textoNormalizado);

    const QString jsonResponse = api.get(
        "search?" + query.toString(QUrl::FullyEncoded)
    );

    const QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (!doc.isObject()) {
        return crearErrorApi(
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
        {"query", textoNormalizado},
        {"cantidad", resultados.size()},
        {"resultados", resultados}
    };
}

Video ServicioYoutube::buscarPrimerVideo(
    const QString &texto,
    int maxResults
)
{
    const QJsonObject detalle = buscarVideos(texto, maxResults);
    const QJsonArray resultados = detalle.value("resultados").toArray();
    if (resultados.isEmpty()) {
        return Video(texto.trimmed(), QString(), QString());
    }

    const QJsonObject video = resultados.first().toObject();
    return Video(
        video.value("titulo").toString(),
        video.value("canal").toString(),
        video.value("descripcion").toString(),
        video.value("portada").toString()
    );
}

SolicitudRecomendacionesChatGpt::SolicitudRecomendacionesChatGpt(
    const QJsonObject &contextoSteam,
    int itemsPorCategoria,
    const QString &modelo
)
    : contextoSteam(contextoSteam),
      itemsPorCategoria(itemsPorCategoria),
      modelo(modelo)
{
}

QJsonObject SolicitudRecomendacionesChatGpt::crearPayload() const
{
    QJsonObject payload;
    payload.insert("model", modelo);
    payload.insert(
        "instructions",
        "Eres un motor de recomendaciones multimedia para una app de escritorio en Qt. Debes inferir gustos del usuario a partir de su biblioteca de Steam y devolver solo JSON."
    );
    payload.insert("input", construirPrompt());

    QJsonObject format;
    format.insert("type", "json_schema");
    format.insert("name", "recomendaciones_usuario");
    format.insert("schema", construirSchema());
    format.insert("strict", true);

    QJsonObject text;
    text.insert("format", format);
    payload.insert("text", text);

    return payload;
}

QJsonObject SolicitudRecomendacionesChatGpt::interpretarRespuesta(
    const QString &respuesta
) const
{
    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(
        respuesta.toUtf8(),
        &error
    );

    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        return crearErrorObjeto(
            "La respuesta de ChatGPT no es un JSON valido.",
            respuesta
        );
    }

    const QJsonObject objeto = documento.object();
    if (objeto.value("ok").isBool() && !objeto.value("ok").toBool()) {
        return objeto;
    }

    const QString textoSalida = extraerTextoSalida(objeto);
    if (textoSalida.trimmed().isEmpty()) {
        return crearErrorObjeto(
            "ChatGPT no devolvio texto util para parsear."
        );
    }

    const QJsonDocument recomendacionesDocumento = QJsonDocument::fromJson(
        textoSalida.toUtf8(),
        &error
    );
    if (error.error != QJsonParseError::NoError
        || !recomendacionesDocumento.isObject()) {
        return crearErrorObjeto(
            "ChatGPT devolvio texto, pero no fue posible convertirlo a JSON.",
            textoSalida
        );
    }

    QJsonObject resultado;
    resultado.insert("api", "chatgpt");
    resultado.insert("modelo", modelo);
    resultado.insert("recomendaciones", recomendacionesDocumento.object());
    return resultado;
}

QString SolicitudRecomendacionesChatGpt::construirPrompt() const
{
    const QString contextoFormateado = QString::fromUtf8(
        QJsonDocument(contextoSteam).toJson(QJsonDocument::Indented)
    );

    return
        "Analiza rapidamente la biblioteca Steam y genera EXACTAMENTE "
        + QString::number(itemsPorCategoria)
        + " recomendaciones en cada categoria: peliculas, libros, juegos, videos YouTube. "
        "Criterios: Deben ser distintas entre si, relacionadas con los gustos detectados, sin repeticiones. "
        "Formato JSON con: titulo, motivo (1-2 lineas breves), query (1-3 palabras). "
        "Solo hechos, no especulaciones.\n\n"
        "Biblioteca:\n"
        + contextoFormateado;
}

QJsonObject SolicitudRecomendacionesChatGpt::construirSchema() const
{
    const QJsonObject itemSchema = crearItemSchema();

    QJsonObject perfilProperties;
    perfilProperties.insert("resumen", QJsonObject{{"type", "string"}});
    perfilProperties.insert(
        "generos_probables",
        QJsonObject{
            {"type", "array"},
            {"items", QJsonObject{{"type", "string"}}}
        }
    );
    perfilProperties.insert(
        "franquicias_detectadas",
        QJsonObject{
            {"type", "array"},
            {"items", QJsonObject{{"type", "string"}}}
        }
    );

    QJsonArray perfilRequired;
    perfilRequired.append("resumen");
    perfilRequired.append("generos_probables");
    perfilRequired.append("franquicias_detectadas");

    QJsonObject perfilSchema;
    perfilSchema.insert("type", "object");
    perfilSchema.insert("properties", perfilProperties);
    perfilSchema.insert("required", perfilRequired);
    perfilSchema.insert("additionalProperties", false);

    QJsonObject properties;
    properties.insert("perfil", perfilSchema);
    properties.insert("peliculas", crearArraySchema(itemSchema));
    properties.insert("libros", crearArraySchema(itemSchema));
    properties.insert("juegos", crearArraySchema(itemSchema));
    properties.insert("videos_youtube", crearArraySchema(itemSchema));

    QJsonArray required;
    required.append("perfil");
    required.append("peliculas");
    required.append("libros");
    required.append("juegos");
    required.append("videos_youtube");

    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", properties);
    schema.insert("required", required);
    schema.insert("additionalProperties", false);
    return schema;
}

QString SolicitudRecomendacionesChatGpt::extraerTextoSalida(
    const QJsonObject &respuesta
) const
{
    const QString outputText = respuesta.value("output_text").toString();
    if (!outputText.trimmed().isEmpty()) {
        return outputText;
    }

    QString acumulado;
    const QJsonArray output = respuesta.value("output").toArray();
    for (const QJsonValue &outputValue : output) {
        const QJsonArray contenido =
            outputValue.toObject().value("content").toArray();
        for (const QJsonValue &contenidoValue : contenido) {
            const QJsonObject contenidoObjeto = contenidoValue.toObject();
            if (contenidoObjeto.value("type").toString() == "output_text") {
                acumulado += contenidoObjeto.value("text").toString();
            }
        }
    }

    return acumulado;
}

QJsonObject SolicitudRecomendacionesChatGpt::crearErrorObjeto(
    const QString &mensaje,
    const QString &detalle
) const
{
    return crearErrorApi("chatgpt", mensaje, detalle);
}

ServicioRecomendacionesChatGpt::ServicioRecomendacionesChatGpt()
    : api(),
      apiKey(DataManager::cargarConfiguracion("OPENAI_API_KEY")),
      modelo(
          DataManager::cargarConfiguracion(
              "OPENAI_MODEL",
              "gpt-5-mini"
          )
      )
{
}

QJsonObject ServicioRecomendacionesChatGpt::generarRecomendaciones(
    const QJsonObject &contextoSteam,
    int itemsPorCategoria
)
{
    if (apiKey.trimmed().isEmpty()) {
        return crearErrorApi(
            "chatgpt",
            "Falta OPENAI_API_KEY para generar recomendaciones."
        );
    }

    if (contextoSteam.isEmpty()) {
        return crearErrorApi(
            "chatgpt",
            "No hay contexto de Steam para enviar a ChatGPT."
        );
    }

    const SolicitudRecomendacionesChatGpt solicitud(
        contextoSteam,
        itemsPorCategoria,
        modelo
    );

    const QString respuesta = api.post("responses", solicitud.crearPayload());
    return solicitud.interpretarRespuesta(respuesta);
}
