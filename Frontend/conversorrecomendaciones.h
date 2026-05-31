#ifndef CONVERSORRECOMENDACIONES_H
#define CONVERSORRECOMENDACIONES_H

#include <QVector>

#include "Juego.h"
#include "Libro.h"
#include "Pelicula.h"
#include "Video.h"
#include "recomendacion.h"

class ConversorRecomendaciones
{
public:
    static QVector<Recomendacion> convertirJuegos(const QVector<Juego> &juegos);
    static QVector<Recomendacion> convertirLibros(const QVector<Libro> &libros);
    static QVector<Recomendacion> convertirPeliculas(const QVector<Pelicula> &peliculas);
    static QVector<Recomendacion> convertirVideos(const QVector<Video> &videos);
};

#endif // CONVERSORRECOMENDACIONES_H
