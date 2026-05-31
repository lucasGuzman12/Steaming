#ifndef PAQUETERECOMENDACIONES_H
#define PAQUETERECOMENDACIONES_H

// Agrupa un conjunto completo de recomendaciones ya convertidas a modelos
// nativos de C++ para consumirlas sin trabajar directamente con JSON.
#include <QVector>
#include <QString>

#include "Juego.h"
#include "Libro.h"
#include "Pelicula.h"
#include "Video.h"

// Contenedor de alto nivel que resume el resultado del recomendador.
class PaqueteRecomendaciones
{
public:
    PaqueteRecomendaciones(
        const QString &id = QString(),
        bool ok = true,
        const QString &error = QString(),
        const QVector<Juego> &juegos = {},
        const QVector<Libro> &libros = {},
        const QVector<Pelicula> &peliculas = {},
        const QVector<Video> &videos = {}
    )
        : id(id),
          ok(ok),
          error(error),
          vector_juegos(juegos),
          vector_libros(libros),
          vector_peliculas(peliculas),
          vector_videos(videos)
    {
    }

    QString get_id() const { return id; }
    bool get_ok() const { return ok; }
    QString get_error() const { return error; }
    QVector<Juego> get_juegos() const { return vector_juegos; }
    QVector<Libro> get_libros() const { return vector_libros; }
    QVector<Pelicula> get_peliculas() const { return vector_peliculas; }
    QVector<Video> get_videos() const { return vector_videos; }

    void set_id(const QString &nuevoId) { id = nuevoId; }
    void set_ok(bool nuevoOk) { ok = nuevoOk; }
    void set_error(const QString &nuevoError) { error = nuevoError; }

private:
    QString id;
    bool ok;
    QString error;
    QVector<Juego> vector_juegos;
    QVector<Libro> vector_libros;
    QVector<Pelicula> vector_peliculas;
    QVector<Video> vector_videos;
};

#endif // PAQUETERECOMENDACIONES_H
