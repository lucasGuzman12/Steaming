#include "principal.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

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

    QPushButton *botonJuegos = crearBloque("Juegos", "Aventuras, estrategia, accion y mas");
    QPushButton *botonPeliculas = crearBloque("Peliculas", "Historias para ver esta noche");
    QPushButton *botonVideos = crearBloque("Videos", "Contenido de YouTube para tus gustos");
    QPushButton *botonLibros = crearBloque("Libros", "Lecturas recomendadas para seguir");

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(botonJuegos, 0, 0);
    grid->addWidget(botonPeliculas, 0, 1);
    grid->addWidget(botonVideos, 1, 0);
    grid->addWidget(botonLibros, 1, 1);
    grid->setSpacing(18);

    QVBoxLayout *contenido = new QVBoxLayout;
    contenido->addLayout(barraSuperior);
    contenido->addSpacing(24);
    contenido->addLayout(grid);
    contenido->addStretch();
    contenido->setContentsMargins(28, 24, 28, 24);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->addLayout(contenido);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    setWindowTitle("Steaming - Principal");
    setStyleSheet(
        "#principal { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "#logo { background: transparent; font-size: 38px; font-weight: bold; color: #00e5ff; }"
        "#descripcion { background: transparent; font-size: 15px; color: #a7b6d8; }"
        "QPushButton { text-align: left; background: #101827; border: 1px solid #263859; border-radius: 8px; padding: 24px; font-size: 22px; font-weight: bold; color: #e8f7ff; }"
        "QPushButton:hover { background: #151f35; color: #00e5ff; border: 1px solid #ff3df2; }"
    );

    connect(botonJuegos, SIGNAL(clicked()), this, SLOT(seleccionarJuegos()));
    connect(botonPeliculas, SIGNAL(clicked()), this, SLOT(seleccionarPeliculas()));
    connect(botonVideos, SIGNAL(clicked()), this, SLOT(seleccionarVideos()));
    connect(botonLibros, SIGNAL(clicked()), this, SLOT(seleccionarLibros()));
}

QPushButton *Principal::crearBloque(const QString &titulo, const QString &descripcion)
{
    QPushButton *boton = new QPushButton(titulo + "\n" + descripcion);
    boton->setMinimumHeight(170);
    boton->setCursor(Qt::PointingHandCursor);
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
