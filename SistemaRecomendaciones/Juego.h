#ifndef JUEGO_H
#define JUEGO_H

#include <QJsonObject>
#include <QString>
using namespace std;

class Juego
{
public:
    Juego(
        const QString &titulo = QString(),
        const QString &empresa = QString(),
        const QString &informacion = QString(),
        double precio = -1.0,
        const QString &portada = QString()
    )
        : titulo(titulo),
          empresa(empresa),
          informacion(informacion),
          precio(precio),
          portada(portada)
    {
    }

    QString get_titulo() const { return titulo; }
    QString get_empresa() const { return empresa; }
    QString get_informacion() const { return informacion; }
    double get_precio() const { return precio; }
    QString get_portada() const { return portada; }

    QJsonObject get_atributos() const
    {
        QJsonObject atributos;
        atributos.insert("titulo", titulo);
        atributos.insert("empresa", empresa);
        atributos.insert("informacion", informacion);
        if (precio >= 0.0) {
            atributos.insert("precio", precio);
        } else {
            atributos.insert("precio", QJsonValue::Null);
        }
        atributos.insert("portada", portada);
        return atributos;
    }

private:
    QString titulo;
    QString empresa;
    QString informacion;
    double precio;
    QString portada;
};

#endif // JUEGO_H
