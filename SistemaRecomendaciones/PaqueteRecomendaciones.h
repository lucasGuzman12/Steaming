#ifndef PAQUETERECOMENDACIONES_H
#define PAQUETERECOMENDACIONES_H

#include <QString>

#include <vector>

#include "Juego.h"
#include "Libro.h"
#include "Pelicula.h"
#include "Video.h"
using namespace std;
//agregar id a la clase (no el del usuario)
class PaqueteRecomendaciones
{
public:
    PaqueteRecomendaciones(
        const QString &id = QString(),
        bool ok = true,
        const QString &error = QString(),
        const  vector<Juego> &juegos = {},
        const  vector<Libro> &libros = {},
        const  vector<Pelicula> &peliculas = {},
        const  vector<Video> &videos = {}
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
     vector<Juego> get_juegos() const { return vector_juegos; }
     vector<Libro> get_libros() const { return vector_libros; }
     vector<Pelicula> get_peliculas() const { return vector_peliculas; }
     vector<Video> get_videos() const { return vector_videos; }

    void set_id(const QString &nuevoId) { id = nuevoId; }
    void set_ok(bool nuevoOk) { ok = nuevoOk; }
    void set_error(const QString &nuevoError) { error = nuevoError; }

private:
    QString id;
    bool ok;
    QString error;
     vector<Juego> vector_juegos;
     vector<Libro> vector_libros;
     vector<Pelicula> vector_peliculas;
     vector<Video> vector_videos;
};

#endif // PAQUETERECOMENDACIONES_H
