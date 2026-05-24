#include "selectorcuentas.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

SelectorCuentas::SelectorCuentas(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("selectorCuentas");
    setAttribute(Qt::WA_StyledBackground, true);

    QLabel *logo = new QLabel;
    logo->setObjectName("logo");
    logo->setFixedSize(84, 84);
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(QPixmap(":/new/imagenes/logo.png").scaled(76, 76, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *titulo = new QLabel("Steaming");
    titulo->setObjectName("titulo");

    QHBoxLayout *encabezado = new QHBoxLayout;
    encabezado->addStretch();
    encabezado->addWidget(logo);
    encabezado->addWidget(titulo);
    encabezado->addStretch();
    encabezado->setSpacing(16);

    QLabel *subtitulo = new QLabel("Selecciona tu perfil");
    subtitulo->setObjectName("subtitulo");
    subtitulo->setAlignment(Qt::AlignCenter);

    QPushButton *cuenta1 = crearBotonCuenta("Valentina", "valentina@steaming.local");
    QPushButton *cuenta2 = crearBotonCuenta("Tomas", "tomas@steaming.local");
    QPushButton *cuenta3 = crearBotonCuenta("Familia", "familia@steaming.local");

    QVBoxLayout *listaCuentas = new QVBoxLayout;
    listaCuentas->addWidget(cuenta1);
    listaCuentas->addWidget(cuenta2);
    listaCuentas->addWidget(cuenta3);
    listaCuentas->setSpacing(14);
    listaCuentas->setContentsMargins(0, 0, 0, 0);

    QWidget *contenedorCuentas = new QWidget;
    contenedorCuentas->setObjectName("contenedorCuentas");
    contenedorCuentas->setLayout(listaCuentas);
    contenedorCuentas->setMaximumWidth(560);

    QPushButton *botonAgregar = new QPushButton("+");
    botonAgregar->setObjectName("botonAgregar");
    botonAgregar->setFixedSize(72, 72);
    botonAgregar->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *filaAgregar = new QHBoxLayout;
    filaAgregar->addStretch();
    filaAgregar->addWidget(botonAgregar);

    QVBoxLayout *contenido = new QVBoxLayout;
    contenido->addSpacing(36);
    contenido->addLayout(encabezado);
    contenido->addWidget(subtitulo);
    contenido->addSpacing(34);
    contenido->addWidget(contenedorCuentas, 0, Qt::AlignCenter);
    contenido->addStretch();
    contenido->addLayout(filaAgregar);
    contenido->setContentsMargins(64, 40, 64, 48);

    setLayout(contenido);
    setWindowTitle("Steaming - Perfiles");
    setStyleSheet(
        "#selectorCuentas { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "QWidget { font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "#logo { background: rgba(7, 10, 18, 150); border: 1px solid #ffffff; border-radius: 14px; }"
        "#titulo { background: transparent; color: #00e5ff; font-size: 52px; font-weight: bold; }"
        "#subtitulo { background: transparent; color: #ffffff; font-size: 18px; }"
        "#contenedorCuentas { background: transparent; }"
        "#contenedorCuentas QWidget { background: transparent; }"
        "#cuenta { text-align: left; background: rgba(7, 10, 18, 150); color: #ffffff; border: 1px solid #ffffff; border-radius: 12px; padding: 0; }"
        "#cuenta:hover { background: #ffffff; border: 1px solid #ffffff; }"
        "#nombreCuenta { background: transparent; color: #ffffff; font-size: 24px; font-weight: bold; }"
        "#emailCuenta { background: transparent; color: #ffffff; font-size: 14px; }"
        "QPushButton#botonAgregar { background: rgba(7, 10, 18, 150); color: #ffffff; border: 1px solid #ffffff; border-radius: 36px; font-size: 34px; font-weight: bold; padding-bottom: 5px; }"
        "QPushButton#botonAgregar:hover { background: #ffffff; color: #08122e; border: 1px solid #ffffff; }"
    );

    connect(cuenta1, SIGNAL(clicked()), this, SLOT(seleccionarCuenta()));
    connect(cuenta2, SIGNAL(clicked()), this, SLOT(seleccionarCuenta()));
    connect(cuenta3, SIGNAL(clicked()), this, SLOT(seleccionarCuenta()));
    connect(botonAgregar, SIGNAL(clicked()), this, SLOT(solicitarAgregarCuenta()));
}

QPushButton *SelectorCuentas::crearBotonCuenta(const QString &nombre, const QString &email)
{
    QPushButton *boton = new QPushButton;
    boton->setObjectName("cuenta");
    boton->setMinimumSize(520, 96);
    boton->setCursor(Qt::PointingHandCursor);

    QLabel *labelNombre = new QLabel(nombre);
    labelNombre->setObjectName("nombreCuenta");
    labelNombre->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *labelEmail = new QLabel(email);
    labelEmail->setObjectName("emailCuenta");
    labelEmail->setAttribute(Qt::WA_TransparentForMouseEvents);

    QVBoxLayout *layout = new QVBoxLayout(boton);
    layout->addWidget(labelNombre);
    layout->addWidget(labelEmail);
    layout->setContentsMargins(24, 14, 24, 14);
    layout->setSpacing(4);

    boton->installEventFilter(this);

    return boton;
}

bool SelectorCuentas::eventFilter(QObject *watched, QEvent *event)
{
    QPushButton *boton = qobject_cast<QPushButton *>(watched);

    if (boton != nullptr && boton->objectName() == "cuenta") {
        if (event->type() == QEvent::Enter) {
            actualizarHoverCuenta(boton, true);
        } else if (event->type() == QEvent::Leave) {
            actualizarHoverCuenta(boton, false);
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SelectorCuentas::actualizarHoverCuenta(QPushButton *boton, bool activo)
{
    QString color = activo ? "#08122e" : "#ffffff";

    QLabel *labelNombre = boton->findChild<QLabel *>("nombreCuenta");
    QLabel *labelEmail = boton->findChild<QLabel *>("emailCuenta");

    if (labelNombre != nullptr) {
        labelNombre->setStyleSheet("background: transparent; color: " + color + "; font-size: 24px; font-weight: bold;");
    }

    if (labelEmail != nullptr) {
        labelEmail->setStyleSheet("background: transparent; color: " + color + "; font-size: 14px;");
    }
}

void SelectorCuentas::seleccionarCuenta()
{
    emit cuentaSeleccionada();
}

void SelectorCuentas::solicitarAgregarCuenta()
{
    emit agregarCuenta();
}
