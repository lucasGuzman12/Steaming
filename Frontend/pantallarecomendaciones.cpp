#include "pantallarecomendaciones.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>

PantallaRecomendaciones::PantallaRecomendaciones(const QString &tituloPantalla,
                                                 const QVector<Recomendacion> &recomendaciones,
                                                 QWidget *parent)
    : QWidget(parent),
      tituloPantalla(tituloPantalla),
      recomendaciones(recomendaciones),
      slideActual(0),
      imagenPrincipal(nullptr),
      tituloPrincipal(nullptr),
      datoExtraPrincipal(nullptr),
      descripcionPrincipal(nullptr),
      botonAnterior(nullptr),
      botonSiguiente(nullptr),
      networkManager(new QNetworkAccessManager(this))
{
    setObjectName("pantallaRecomendaciones");
    setAttribute(Qt::WA_StyledBackground, true);

    QPushButton *bloqueLogoSuperior = new QPushButton("<");
    bloqueLogoSuperior->setObjectName("bloqueLogoSuperior");
    bloqueLogoSuperior->setFixedSize(110, 70);
    bloqueLogoSuperior->setIcon(QIcon(":/new/imagenes/logo.png"));
    bloqueLogoSuperior->setIconSize(QSize(52, 52));
    bloqueLogoSuperior->setLayoutDirection(Qt::RightToLeft);
    bloqueLogoSuperior->setCursor(Qt::PointingHandCursor);

    QLabel *titulo = new QLabel(tituloPantalla);
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

    QFrame *recomendacionPrincipal = crearHero();

    QLabel *tituloSecundarias = new QLabel("Tambien te puede gustar");
    tituloSecundarias->setObjectName("tituloPanel");

    QVBoxLayout *layoutSecundarias = new QVBoxLayout;
    layoutSecundarias->addWidget(tituloSecundarias);
    layoutSecundarias->addWidget(crearCajaSecundaria());
    layoutSecundarias->addWidget(crearCajaSecundaria());
    layoutSecundarias->addWidget(crearCajaSecundaria());
    layoutSecundarias->addStretch();
    layoutSecundarias->setSpacing(18);

    QFrame *panelRelacionados = new QFrame;
    panelRelacionados->setObjectName("panelRelacionados");
    panelRelacionados->setLayout(layoutSecundarias);
    panelRelacionados->setMinimumWidth(280);
    panelRelacionados->setMaximumWidth(340);

    QHBoxLayout *contenidoCentral = new QHBoxLayout;
    contenidoCentral->addWidget(panelRelacionados);
    contenidoCentral->addWidget(recomendacionPrincipal, 1);
    contenidoCentral->setSpacing(18);

    botonAnterior = new QPushButton("Anterior");
    botonAnterior->setObjectName("botonNavegacion");

    botonSiguiente = new QPushButton("Siguiente");
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

    setWindowTitle("Steaming - " + tituloPantalla);
    setStyleSheet(
        "#pantallaRecomendaciones { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
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
        "#datoExtraPrincipal { color: #ff3df2; font-size: 15px; font-weight: bold; }"
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
        "#botonNavegacion:disabled { background: #252936; color: #767d8f; border: 1px solid #4a5063; }"
    );

    connect(bloqueLogoSuperior, SIGNAL(clicked()), this, SLOT(volver()));
    connect(botonAnterior, SIGNAL(clicked()), this, SLOT(mostrarAnterior()));
    connect(botonSiguiente, SIGNAL(clicked()), this, SLOT(mostrarSiguiente()));

    actualizarContenido();
}

void PantallaRecomendaciones::setRecomendaciones(const QVector<Recomendacion> &nuevasRecomendaciones)
{
    recomendaciones = nuevasRecomendaciones;
    slideActual = 0;
    actualizarContenido();
}

Recomendacion PantallaRecomendaciones::recomendacionEn(int indice) const
{
    if (indice >= 0 && indice < recomendaciones.size()) {
        return recomendaciones[indice];
    }

    return Recomendacion {
        "Recomendacion pendiente",
        "Todavia no hay datos reales para esta recomendacion. Mas adelante vendran desde el sistema de procesamiento.",
        "",
        ""
    };
}

Recomendacion PantallaRecomendaciones::recomendacionDelSlide(int posicion) const
{
    int indice = slideActual * 4 + posicion;
    return recomendacionEn(indice);
}

int PantallaRecomendaciones::cantidadSlides() const
{
    int cantidad = (recomendaciones.size() + 3) / 4;

    if (cantidad < 1) {
        return 1;
    }

    return cantidad;
}

void PantallaRecomendaciones::actualizarContenido()
{
    Recomendacion principal = recomendacionDelSlide(0);

    if (imagenPrincipal != nullptr) {
        cargarImagenEnLabel(imagenPrincipal, principal.imagen, 980, 520);
    }

    if (tituloPrincipal != nullptr) {
        tituloPrincipal->setText(principal.titulo);
    }

    if (datoExtraPrincipal != nullptr) {
        datoExtraPrincipal->setText(principal.dato_extra.trimmed().isEmpty()
            ? "Dato destacado"
            : principal.dato_extra);
    }

    if (descripcionPrincipal != nullptr) {
        descripcionPrincipal->setText(principal.descripcion);
    }

    for (int i = 0; i < titulosSecundarios.size(); ++i) {
        Recomendacion secundaria = recomendacionDelSlide(i + 1);

        if (i < imagenesSecundarias.size() && imagenesSecundarias[i] != nullptr) {
            cargarImagenEnLabel(imagenesSecundarias[i], secundaria.imagen, 260, 130);
        }

        if (titulosSecundarios[i] != nullptr) {
            titulosSecundarios[i]->setText(secundaria.titulo);
        }
    }

    actualizarBotonesNavegacion();
}

void PantallaRecomendaciones::actualizarBotonesNavegacion()
{
    if (botonAnterior != nullptr) {
        botonAnterior->setEnabled(slideActual > 0);
    }

    if (botonSiguiente != nullptr) {
        botonSiguiente->setEnabled(slideActual < cantidadSlides() - 1);
    }
}

QPixmap PantallaRecomendaciones::cargarImagenLocal(const QString &imagen, int ancho, int alto) const
{
    QString ruta = imagen.trimmed().isEmpty()
        ? ":/new/imagenes/imagen_no_disponible.png"
        : imagen;

    QPixmap pixmap(ruta);

    if (pixmap.isNull()) {
        pixmap.load(":/new/imagenes/imagen_no_disponible.png");
    }

    return pixmap.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void PantallaRecomendaciones::cargarImagenEnLabel(QLabel *label, const QString &imagen, int ancho, int alto)
{
    if (label == nullptr) {
        return;
    }

    const QString imagenNormalizada = imagen.trimmed();
    const QUrl url(imagenNormalizada);
    label->setProperty("imagenActual", imagenNormalizada);

    if (!url.isValid() || !url.scheme().startsWith("http")) {
        label->setPixmap(cargarImagenLocal(imagenNormalizada, ancho, alto));
        return;
    }

    label->setPixmap(cargarImagenLocal(QString(), ancho, alto));

    QNetworkRequest request(url);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
    );
    request.setRawHeader("User-Agent", "Mozilla/5.0");

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, label, imagenNormalizada, ancho, alto]() {
        if (label->property("imagenActual").toString() != imagenNormalizada) {
            reply->deleteLater();
            return;
        }

        QPixmap pixmap;

        if (reply->error() == QNetworkReply::NoError) {
            pixmap.loadFromData(reply->readAll());
        }

        if (pixmap.isNull()) {
            pixmap = cargarImagenLocal(QString(), ancho, alto);
        } else {
            pixmap = pixmap.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        label->setPixmap(pixmap);
        reply->deleteLater();
    });
}

QFrame *PantallaRecomendaciones::crearHero()
{
    QFrame *tarjeta = new QFrame;
    tarjeta->setObjectName("tarjetaPrincipal");
    tarjeta->setMinimumHeight(390);
    tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    imagenPrincipal = new QLabel;
    imagenPrincipal->setObjectName("imagenHero");
    imagenPrincipal->setAlignment(Qt::AlignCenter);

    QWidget *degradado = new QWidget;
    degradado->setObjectName("degradadoHero");

    QLabel *etiqueta = new QLabel("RECOMENDACION PRINCIPAL");
    etiqueta->setObjectName("etiquetaPrincipal");
    aplicarSombra(etiqueta, 12, 1);

    tituloPrincipal = new QLabel;
    tituloPrincipal->setObjectName("tituloPrincipal");
    tituloPrincipal->setWordWrap(true);
    aplicarSombra(tituloPrincipal, 18, 2);

    datoExtraPrincipal = new QLabel;
    datoExtraPrincipal->setObjectName("datoExtraPrincipal");
    datoExtraPrincipal->setWordWrap(true);
    aplicarSombra(datoExtraPrincipal, 14, 1);

    descripcionPrincipal = new QLabel;
    descripcionPrincipal->setObjectName("descripcionPrincipal");
    descripcionPrincipal->setWordWrap(true);
    aplicarSombra(descripcionPrincipal, 14, 1);

    QVBoxLayout *layoutTexto = new QVBoxLayout(degradado);
    layoutTexto->addStretch();
    layoutTexto->addWidget(etiqueta);
    layoutTexto->addWidget(tituloPrincipal);
    layoutTexto->addWidget(datoExtraPrincipal);
    layoutTexto->addWidget(descripcionPrincipal);
    layoutTexto->setContentsMargins(28, 120, 28, 28);
    layoutTexto->setSpacing(6);

    QStackedLayout *stack = new QStackedLayout(tarjeta);
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(imagenPrincipal);
    stack->addWidget(degradado);
    stack->setCurrentWidget(degradado);

    return tarjeta;
}

QFrame *PantallaRecomendaciones::crearCajaSecundaria()
{
    QFrame *tarjeta = new QFrame;
    tarjeta->setObjectName("tarjetaSecundaria");
    tarjeta->setMinimumHeight(150);
    tarjeta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel *imagen = new QLabel;
    imagen->setObjectName("imagenSecundaria");
    imagen->setAlignment(Qt::AlignCenter);
    imagenesSecundarias.append(imagen);

    QWidget *capaTexto = new QWidget;
    capaTexto->setObjectName("capaSecundaria");

    QLabel *titulo = new QLabel;
    titulo->setObjectName("tituloSecundario");
    titulo->setWordWrap(true);
    titulo->setAlignment(Qt::AlignCenter);
    aplicarSombra(titulo, 30, 3);
    titulosSecundarios.append(titulo);

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
}

void PantallaRecomendaciones::aplicarSombra(QLabel *label, int blur, int offsetY)
{
    QGraphicsDropShadowEffect *sombra = new QGraphicsDropShadowEffect(label);
    sombra->setBlurRadius(blur);
    sombra->setOffset(0, offsetY);
    sombra->setColor(QColor(0, 0, 0, 220));
    label->setGraphicsEffect(sombra);
}

void PantallaRecomendaciones::volver()
{
    emit volverPrincipal();
}

void PantallaRecomendaciones::mostrarAnterior()
{
    if (slideActual > 0) {
        slideActual--;
        actualizarContenido();
    }
}

void PantallaRecomendaciones::mostrarSiguiente()
{
    if (slideActual < cantidadSlides() - 1) {
        slideActual++;
        actualizarContenido();
    }
}
