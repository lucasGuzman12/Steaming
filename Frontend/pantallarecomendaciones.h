#ifndef PANTALLARECOMENDACIONES_H
#define PANTALLARECOMENDACIONES_H

#include <QWidget>
#include <QVector>

#include "recomendacion.h"

class QLabel;
class QFrame;
class QNetworkAccessManager;
class QPixmap;
class QPushButton;

class PantallaRecomendaciones : public QWidget
{
    Q_OBJECT

public:
    explicit PantallaRecomendaciones(const QString &tituloPantalla,
                                     const QVector<Recomendacion> &recomendaciones,
                                     QWidget *parent = nullptr);

    void setRecomendaciones(const QVector<Recomendacion> &nuevasRecomendaciones);

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
    QLabel *datoExtraPrincipal;
    QLabel *descripcionPrincipal;
    QVector<QLabel *> imagenesSecundarias;
    QVector<QLabel *> titulosSecundarios;
    QPushButton *botonAnterior;
    QPushButton *botonSiguiente;
    QNetworkAccessManager *networkManager;

    Recomendacion recomendacionEn(int indice) const;
    Recomendacion recomendacionDelSlide(int posicion) const;
    int cantidadSlides() const;
    void actualizarContenido();
    void actualizarBotonesNavegacion();
    QPixmap cargarImagenLocal(const QString &imagen, int ancho, int alto) const;
    void cargarImagenEnLabel(QLabel *label, const QString &imagen, int ancho, int alto);
    QFrame *crearHero();
    QFrame *crearCajaSecundaria();
    void aplicarSombra(QLabel *label, int blur, int offsetY);
};

#endif // PANTALLARECOMENDACIONES_H
