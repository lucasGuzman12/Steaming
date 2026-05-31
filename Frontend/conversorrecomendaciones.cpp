#include "conversorrecomendaciones.h"

#include <QString>

QVector<Recomendacion> ConversorRecomendaciones::convertirJuegos(const QVector<Juego> &juegos)
{
    QVector<Recomendacion> recomendaciones;
    recomendaciones.reserve(juegos.size());

    for (const Juego &juego : juegos) {
        recomendaciones.append(Recomendacion {
            juego.get_titulo(),
            juego.get_informacion(),
            juego.get_portada(),
            juego.get_empresa()
        });
    }

    return recomendaciones;
}

QVector<Recomendacion> ConversorRecomendaciones::convertirLibros(const QVector<Libro> &libros)
{
    QVector<Recomendacion> recomendaciones;
    recomendaciones.reserve(libros.size());

    for (const Libro &libro : libros) {
        recomendaciones.append(Recomendacion {
            libro.get_titulo(),
            libro.get_sinopsis(),
            libro.get_portada(),
            libro.get_autor()
        });
    }

    return recomendaciones;
}

QVector<Recomendacion> ConversorRecomendaciones::convertirPeliculas(const QVector<Pelicula> &peliculas)
{
    QVector<Recomendacion> recomendaciones;
    recomendaciones.reserve(peliculas.size());

    for (const Pelicula &pelicula : peliculas) {
        recomendaciones.append(Recomendacion {
            pelicula.get_titulo(),
            pelicula.get_sinopsis(),
            pelicula.get_portada(),
            QString::number(pelicula.get_anio())
        });
    }

    return recomendaciones;
}

QVector<Recomendacion> ConversorRecomendaciones::convertirVideos(const QVector<Video> &videos)
{
    QVector<Recomendacion> recomendaciones;
    recomendaciones.reserve(videos.size());

    for (const Video &video : videos) {
        recomendaciones.append(Recomendacion {
            video.get_titulo(),
            video.get_descripcion(),
            video.get_portada(),
            video.get_canal()
        });
    }

    return recomendaciones;
}
