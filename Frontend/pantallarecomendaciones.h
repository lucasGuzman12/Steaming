#ifndef PANTALLARECOMENDACIONES_H
#define PANTALLARECOMENDACIONES_H

#include <QWidget>
#include <QVector>

#include "recomendacion.h"

class QLabel;
class QFrame;
class QPixmap;
class QPushButton;

class PantallaRecomendaciones : public QWidget
{
    Q_OBJECT

public:
    explicit PantallaRecomendaciones(const QString &tituloPantalla,
                                     const QVector<Recomendacion> &recomendaciones,
                                     QWidget *parent = nullptr);

signals:
    void volverPrincipal();

private slots:
    void volver();
    void mostrarAnterior();
    void mostrarSiguiente();

private:
    QString tituloPantalla;
    QVector<Recomendacion> recomendaciones;
    int slideActual;

    QLabel *imagenPrincipal;
    QLabel *tituloPrincipal;
    QLabel *descripcionPrincipal;
    QVector<QLabel *> imagenesSecundarias;
    QVector<QLabel *> titulosSecundarios;
    QPushButton *botonAnterior;
    QPushButton *botonSiguiente;

    Recomendacion recomendacionEn(int indice) const;
    Recomendacion recomendacionDelSlide(int posicion) const;
    int cantidadSlides() const;
    void actualizarContenido();
    void actualizarBotonesNavegacion();
    QPixmap cargarImagen(const QString &rutaImagen, int ancho, int alto) const;
    QFrame *crearHero();
    QFrame *crearCajaSecundaria();
    void aplicarSombra(QLabel *label, int blur, int offsetY);
};

#endif // PANTALLARECOMENDACIONES_H
