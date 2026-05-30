#ifndef PELICULA_H
#define PELICULA_H

#include <QJsonObject>
#include <QString>
using namespace std;

class Pelicula
{
public:
    Pelicula(
        const QString &titulo = QString(),
        const QString &autor = QString(),
        const QString &sinopsis = QString(),
        int anio = 0,
        const QString &portada = QString()
    )
        : titulo(titulo),
          autor(autor),
          sinopsis(sinopsis),
          anio(anio),
          portada(portada)
    {
    }

    QString get_titulo() const { return titulo; }
    QString get_autor() const { return autor; }
    QString get_sinopsis() const { return sinopsis; }
    int get_anio() const { return anio; }
    QString get_portada() const { return portada; }

    QJsonObject get_atributos() const
    {
        QJsonObject atributos;
        atributos.insert("titulo", titulo);
        atributos.insert("autor", autor);
        atributos.insert("sinopsis", sinopsis);
        atributos.insert("anio", anio);
        atributos.insert("portada", portada);
        return atributos;
    }

private:
    QString titulo;
    QString autor;
    QString sinopsis;
    int anio;
    QString portada;
};

#endif // PELICULA_H
