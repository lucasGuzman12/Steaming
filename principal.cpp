#include "principal.h"

#include <QColor>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

class BotonImagenCategoria : public QPushButton
{
public:
    explicit BotonImagenCategoria(const QString &titulo, const QString &rutaImagen, QWidget *parent = nullptr)
        : QPushButton(parent),
          imagen(rutaImagen)
    {
        setToolTip(titulo);
        setCursor(Qt::PointingHandCursor);
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        QRectF area = rect().adjusted(1, 1, -1, -1);
        QPainterPath path;
        path.addRoundedRect(area, 12, 12);

        painter.setClipPath(path);

        if (!imagen.isNull()) {
            QPixmap escalada = imagen.scaled(area.size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int x = static_cast<int>((area.width() - escalada.width()) / 2);
            int y = static_cast<int>((area.height() - escalada.height()) / 2);
            painter.drawPixmap(static_cast<int>(area.left()) + x, static_cast<int>(area.top()) + y, escalada);
        } else {
            painter.fillPath(path, QColor("#101827"));
        }

        painter.setClipping(false);
        QPen borde(underMouse() ? QColor("#ff3df2") : QColor("#263859"));
        borde.setWidth(underMouse() ? 2 : 1);
        painter.setPen(borde);
        painter.drawPath(path);
    }

private:
    QPixmap imagen;
};

Principal::Principal(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("principal");
    setAttribute(Qt::WA_StyledBackground, true);

    QLabel *logo = new QLabel("Steaming");
    logo->setObjectName("logo");

    QLabel *descripcion = new QLabel("Recomendaciones para descubrir tu proximo favorito");
    descripcion->setObjectName("descripcion");

    QVBoxLayout *barraTexto = new QVBoxLayout;
    barraTexto->addWidget(logo);
    barraTexto->addWidget(descripcion);
    barraTexto->setSpacing(2);

    QHBoxLayout *barraSuperior = new QHBoxLayout;
    barraSuperior->addLayout(barraTexto);
    barraSuperior->addStretch();

    QPushButton *botonJuegos = crearBloque("Juegos", ":/new/imagenes/Img-Juegos.png");
    QPushButton *botonPeliculas = crearBloque("Peliculas", ":/new/imagenes/Img-Peliculas.png");
    QPushButton *botonVideos = crearBloque("Videos", ":/new/imagenes/Img-Videos.png");
    QPushButton *botonLibros = crearBloque("Libros", ":/new/imagenes/Img-Libros.png");

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(botonJuegos, 0, 0);
    grid->addWidget(botonPeliculas, 0, 1);
    grid->addWidget(botonVideos, 1, 0);
    grid->addWidget(botonLibros, 1, 1);
    grid->setSpacing(26);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);

    QVBoxLayout *contenido = new QVBoxLayout;
    contenido->addLayout(barraSuperior);
    contenido->addSpacing(24);
    contenido->addLayout(grid, 1);
    contenido->setContentsMargins(28, 24, 28, 24);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->addLayout(contenido);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    setWindowTitle("Steaming - Principal");
    setStyleSheet(
        "#principal { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "#logo { background: transparent; font-size: 38px; font-weight: bold; color: #00e5ff; }"
        "#descripcion { background: transparent; font-size: 15px; color: #a7b6d8; }"
    );

    connect(botonJuegos, SIGNAL(clicked()), this, SLOT(seleccionarJuegos()));
    connect(botonPeliculas, SIGNAL(clicked()), this, SLOT(seleccionarPeliculas()));
    connect(botonVideos, SIGNAL(clicked()), this, SLOT(seleccionarVideos()));
    connect(botonLibros, SIGNAL(clicked()), this, SLOT(seleccionarLibros()));
}

QPushButton *Principal::crearBloque(const QString &titulo, const QString &rutaImagen)
{
    QPushButton *boton = new BotonImagenCategoria(titulo, rutaImagen);
    return boton;
}

void Principal::seleccionarJuegos()
{
    emit abrirJuegos();
}

void Principal::seleccionarPeliculas()
{
    emit abrirPeliculas();
}

void Principal::seleccionarLibros()
{
    emit abrirLibros();
}

void Principal::seleccionarVideos()
{
    emit abrirVideos();
}
