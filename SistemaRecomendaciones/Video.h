#ifndef VIDEO_H
#define VIDEO_H

// Representa un video recomendado de YouTube.
#include <QJsonObject>
#include <QString>
using namespace std;

// Modelo minimo para transportar datos de videos dentro de la app.
class Video
{
public:
    Video(
        const QString &titulo = QString(),
        const QString &canal = QString(),
        const QString &descripcion = QString(),
        const QString &portada = QString()
    )
        : titulo(titulo),
          canal(canal),
          descripcion(descripcion),
          portada(portada)
    {
    }

    QString get_titulo() const { return titulo; }
    QString get_canal() const { return canal; }
    QString get_descripcion() const { return descripcion; }
    QString get_portada() const { return portada; }

    // Convierte el video a JSON para poder mezclarlo con otros resultados.
    QJsonObject get_atributos() const
    {
        QJsonObject atributos;
        atributos.insert("titulo", titulo);
        atributos.insert("canal", canal);
        atributos.insert("descripcion", descripcion);
        atributos.insert("portada", portada);
        return atributos;
    }

private:
    QString titulo;
    QString canal;
    QString descripcion;
    QString portada;
};

#endif // VIDEO_H
