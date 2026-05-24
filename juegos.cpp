#include "juegos.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QVBoxLayout>

Juegos::Juegos(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("juegos");
    setAttribute(Qt::WA_StyledBackground, true);

    QPushButton *bloqueLogoSuperior = new QPushButton("<");
    bloqueLogoSuperior->setObjectName("bloqueLogoSuperior");
    bloqueLogoSuperior->setFixedSize(110, 70);
    bloqueLogoSuperior->setIcon(QIcon(":/new/imagenes/logo.png"));
    bloqueLogoSuperior->setIconSize(QSize(52, 52));
    bloqueLogoSuperior->setLayoutDirection(Qt::RightToLeft);
    bloqueLogoSuperior->setCursor(Qt::PointingHandCursor);

    QLabel *titulo = new QLabel("Juegos recomendados");
    titulo->setObjectName("tituloCategoria");

    QHBoxLayout *barraSuperior = new QHBoxLayout;
    barraSuperior->addWidget(titulo);
    barraSuperior->addStretch();
    barraSuperior->setContentsMargins(18, 12, 18, 12);
    barraSuperior->setSpacing(12);

    QFrame *bloqueSuperior = new QFrame;
    bloqueSuperior->setObjectName("barraSuperior");
    bloqueSuperior->setFixedHeight(70);
    bloqueSuperior->setLayout(barraSuperior);

    QHBoxLayout *filaSuperior = new QHBoxLayout;
    filaSuperior->addWidget(bloqueLogoSuperior);
    filaSuperior->addWidget(bloqueSuperior, 1);
    filaSuperior->setSpacing(18);

    QString rutaImagen = ":/new/imagenes/imagen_no_disponible.png";
    QPixmap imagenPorDefecto(rutaImagen);

    auto aplicarSombra = [](QLabel *label, int blur, int offsetY) {
        QGraphicsDropShadowEffect *sombra = new QGraphicsDropShadowEffect(label);
        sombra->setBlurRadius(blur);
        sombra->setOffset(0, offsetY);
        sombra->setColor(QColor(0, 0, 0, 220));
        label->setGraphicsEffect(sombra);
    };

    auto crearHero = [&imagenPorDefecto, &aplicarSombra](const QString &tituloRecomendacion, const QString &descripcion) {
        QFrame *tarjeta = new QFrame;
        tarjeta->setObjectName("tarjetaPrincipal");
        tarjeta->setMinimumHeight(390);
        tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QLabel *imagen = new QLabel;
        imagen->setObjectName("imagenHero");
        imagen->setAlignment(Qt::AlignCenter);
        imagen->setPixmap(imagenPorDefecto.scaled(980, 520, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QWidget *degradado = new QWidget;
        degradado->setObjectName("degradadoHero");

        QLabel *etiqueta = new QLabel("RECOMENDACION PRINCIPAL");
        etiqueta->setObjectName("etiquetaPrincipal");
        aplicarSombra(etiqueta, 12, 1);

        QLabel *titulo = new QLabel(tituloRecomendacion);
        titulo->setObjectName("tituloPrincipal");
        titulo->setWordWrap(true);
        aplicarSombra(titulo, 18, 2);

        QLabel *texto = new QLabel(descripcion);
        texto->setObjectName("descripcionPrincipal");
        texto->setWordWrap(true);
        aplicarSombra(texto, 14, 1);

        QVBoxLayout *layoutTexto = new QVBoxLayout(degradado);
        layoutTexto->addStretch();
        layoutTexto->addWidget(etiqueta);
        layoutTexto->addWidget(titulo);
        layoutTexto->addWidget(texto);
        layoutTexto->setContentsMargins(28, 120, 28, 28);
        layoutTexto->setSpacing(6);

        QStackedLayout *stack = new QStackedLayout(tarjeta);
        stack->setStackingMode(QStackedLayout::StackAll);
        stack->setContentsMargins(0, 0, 0, 0);
        stack->addWidget(imagen);
        stack->addWidget(degradado);
        stack->setCurrentWidget(degradado);

        return tarjeta;
    };

    auto crearCajaSecundaria = [&imagenPorDefecto, &aplicarSombra](const QString &tituloRecomendacion) {
        QFrame *tarjeta = new QFrame;
        tarjeta->setObjectName("tarjetaSecundaria");
        tarjeta->setMinimumHeight(150);
        tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QLabel *imagen = new QLabel;
        imagen->setObjectName("imagenSecundaria");
        imagen->setAlignment(Qt::AlignCenter);
        imagen->setPixmap(imagenPorDefecto.scaled(260, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QWidget *capaTexto = new QWidget;
        capaTexto->setObjectName("capaSecundaria");

        QLabel *titulo = new QLabel(tituloRecomendacion);
        titulo->setObjectName("tituloSecundario");
        titulo->setWordWrap(true);
        titulo->setAlignment(Qt::AlignCenter);
        aplicarSombra(titulo, 30, 3);

        QVBoxLayout *layoutTexto = new QVBoxLayout(capaTexto);
        layoutTexto->addStretch();
        layoutTexto->addWidget(titulo);
        layoutTexto->addStretch();
        layoutTexto->setContentsMargins(16, 16, 16, 16);

        QStackedLayout *stack = new QStackedLayout(tarjeta);
        stack->setStackingMode(QStackedLayout::StackAll);
        stack->setContentsMargins(3, 3, 3, 3);
        stack->addWidget(imagen);
        stack->addWidget(capaTexto);
        stack->setCurrentWidget(capaTexto);

        return tarjeta;
    };

    QFrame *recomendacionPrincipal = crearHero(
        "The Legend of Zelda: Breath of the Wild",
        "Una aventura amplia, exploratoria y moderna. Ideal si te gustan los mundos abiertos, descubrir secretos y avanzar a tu propio ritmo."
    );

    QLabel *tituloSecundarias = new QLabel("Tambien te puede gustar");
    tituloSecundarias->setObjectName("tituloPanel");

    QVBoxLayout *recomendaciones = new QVBoxLayout;
    recomendaciones->addWidget(tituloSecundarias);
    recomendaciones->addWidget(crearCajaSecundaria("Hollow Knight"));
    recomendaciones->addWidget(crearCajaSecundaria("Stardew Valley"));
    recomendaciones->addWidget(crearCajaSecundaria("Hades"));
    recomendaciones->addStretch();
    recomendaciones->setSpacing(18);

    QFrame *panelRelacionados = new QFrame;
    panelRelacionados->setObjectName("panelRelacionados");
    panelRelacionados->setLayout(recomendaciones);
    panelRelacionados->setMinimumWidth(280);
    panelRelacionados->setMaximumWidth(340);

    QHBoxLayout *contenidoCentral = new QHBoxLayout;
    contenidoCentral->addWidget(panelRelacionados);
    contenidoCentral->addWidget(recomendacionPrincipal, 1);
    contenidoCentral->setSpacing(18);

    QPushButton *botonAnterior = new QPushButton("Anterior");
    botonAnterior->setObjectName("botonNavegacion");

    QPushButton *botonSiguiente = new QPushButton("Siguiente");
    botonSiguiente->setObjectName("botonNavegacion");

    QHBoxLayout *layoutNavegacion = new QHBoxLayout;
    layoutNavegacion->addWidget(botonAnterior);
    layoutNavegacion->addStretch();
    layoutNavegacion->addWidget(botonSiguiente);
    layoutNavegacion->setContentsMargins(18, 12, 18, 12);

    QFrame *barraNavegacion = new QFrame;
    barraNavegacion->setObjectName("barraNavegacion");
    barraNavegacion->setLayout(layoutNavegacion);

    QVBoxLayout *contenido = new QVBoxLayout;
    contenido->addLayout(filaSuperior);
    contenido->addSpacing(18);
    contenido->addLayout(contenidoCentral, 1);
    contenido->addSpacing(18);
    contenido->addWidget(barraNavegacion);
    contenido->setContentsMargins(28, 24, 28, 24);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->addLayout(contenido);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);

    setWindowTitle("Steaming - Juegos");
    setStyleSheet(
        "#juegos { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "QLabel { background: transparent; color: #e8f7ff; }"
        "#barraSuperior { background: #0b1020; border: 1px solid #263859; border-radius: 10px; }"
        "#bloqueLogoSuperior { background: #0b1020; color: #00e5ff; border: 1px solid #00e5ff; border-radius: 10px; font-size: 30px; font-weight: bold; padding: 0 12px; }"
        "#bloqueLogoSuperior:hover { background: #151f35; color: #9dff00; border: 1px solid #ff3df2; }"
        "#tituloCategoria { font-size: 28px; font-weight: bold; color: #00e5ff; }"
        "#panelRelacionados { background: #0b1020; border: 1px solid #263859; border-radius: 10px; }"
        "#tituloPanel { color: #9dff00; font-size: 16px; font-weight: bold; padding: 2px; }"
        "#tarjetaPrincipal { background: #05070d; border: 1px solid #00e5ff; border-radius: 14px; }"
        "#imagenHero { background: #05070d; border-radius: 14px; }"
        "#degradadoHero { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(7, 10, 18, 0), stop:0.42 rgba(7, 10, 18, 45), stop:1 rgba(7, 10, 18, 230)); border-radius: 14px; }"
        "#etiquetaPrincipal { color: #9dff00; font-size: 13px; font-weight: bold; }"
        "#tituloPrincipal { color: #00e5ff; font-size: 34px; font-weight: bold; }"
        "#descripcionPrincipal { color: #e8f7ff; font-size: 17px; }"
        "#tarjetaSecundaria { background: #05070d; border: 1px solid #263859; border-radius: 12px; }"
        "#tarjetaSecundaria:hover { border: 1px solid #ff3df2; }"
        "#imagenSecundaria { background: #05070d; border-radius: 12px; }"
        "#capaSecundaria { background: rgba(7, 10, 18, 135); border-radius: 9px; }"
        "#tituloSecundario { color: #ffffff; font-size: 18px; font-weight: bold; }"
        "#barraNavegacion { background: #0b1020; border: 1px solid #263859; border-radius: 10px; }"
        "QPushButton { background: #161f33; color: #e8f7ff; border: 1px solid #ff3df2; border-radius: 7px; padding: 8px 12px; font-weight: bold; }"
        "QPushButton:hover { background: #ff3df2; color: #070a12; border: 1px solid #00e5ff; }"
        "#botonNavegacion { min-width: 130px; padding: 10px 16px; }"
    );

    connect(bloqueLogoSuperior, SIGNAL(clicked()), this, SLOT(volver()));
}

void Juegos::volver()
{
    emit volverPrincipal();
}
