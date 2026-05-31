#ifndef LIBRO_H
#define LIBRO_H

// Representa un libro con los campos minimos que consume la interfaz.
#include <QJsonObject>
#include <QString>
using namespace std;

// Modelo de datos para resultados de Google Books y recomendaciones.
class Libro
{
public:
    Libro(
        const QString &titulo = QString(),
        const QString &autor = QString(),
        const QString &sinopsis = QString(),
        const QString &portada = QString()
    )
        : titulo(titulo),
          autor(autor),
          sinopsis(sinopsis),
          portada(portada)
    {
    }

    QString get_titulo() const { return titulo; }
    QString get_autor() const { return autor; }
    QString get_sinopsis() const { return sinopsis; }
    QString get_portada() const { return portada; }

    // Devuelve una version JSON util para render, cache o depuracion.
    QJsonObject get_atributos() const
    {
        QJsonObject atributos;
        atributos.insert("titulo", titulo);
        atributos.insert("autor", autor);
        atributos.insert("sinopsis", sinopsis);
        atributos.insert("portada", portada);
        return atributos;
    }

private:
    QString titulo;
    QString autor;
    QString sinopsis;
    QString portada;
};

#endif // LIBRO_H
