#include "manager.h"

#include <QHBoxLayout>
#include <QStackedWidget>

#include "chat.h"
#include "juegos.h"
#include "libros.h"
#include "login.h"
#include "peliculas.h"
#include "principal.h"
#include "registro.h"
#include "selectorcuentas.h"
#include "videos.h"

Manager::Manager(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("manager");
    setAttribute(Qt::WA_StyledBackground, true);

    pantallas = new QStackedWidget;
    chat = new Chat;

    selectorCuentas = new SelectorCuentas;
    login = new Login;
    registro = new Registro;
    principal = new Principal;
    juegos = new Juegos;
    peliculas = new Peliculas;
    libros = new Libros;
    videos = new Videos;

    pantallas->addWidget(selectorCuentas);
    pantallas->addWidget(login);
    pantallas->addWidget(registro);
    pantallas->addWidget(principal);
    pantallas->addWidget(juegos);
    pantallas->addWidget(peliculas);
    pantallas->addWidget(libros);
    pantallas->addWidget(videos);

    QHBoxLayout *layoutPrincipal = new QHBoxLayout(this);
    layoutPrincipal->addWidget(pantallas, 1);
    layoutPrincipal->addWidget(chat);
    layoutPrincipal->setSpacing(18);
    layoutPrincipal->setContentsMargins(0, 0, 18, 0);

    setWindowTitle("Steaming");
    setMinimumSize(900, 600);
    setStyleSheet(
        "#manager { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); }"
    );

    connect(selectorCuentas, SIGNAL(cuentaSeleccionada()), this, SLOT(mostrarPrincipal()));
    connect(selectorCuentas, SIGNAL(agregarCuenta()), this, SLOT(mostrarLogin()));

    connect(login, SIGNAL(ingresoCorrecto()), this, SLOT(mostrarPrincipal()));
    connect(login, SIGNAL(abrirRegistro()), this, SLOT(mostrarRegistro()));

    connect(registro, SIGNAL(registroCompletado()), this, SLOT(mostrarLogin()));
    connect(registro, SIGNAL(volverLogin()), this, SLOT(mostrarLogin()));

    connect(principal, SIGNAL(abrirJuegos()), this, SLOT(mostrarJuegos()));
    connect(principal, SIGNAL(abrirPeliculas()), this, SLOT(mostrarPeliculas()));
    connect(principal, SIGNAL(abrirLibros()), this, SLOT(mostrarLibros()));
    connect(principal, SIGNAL(abrirVideos()), this, SLOT(mostrarVideos()));

    connect(juegos, SIGNAL(volverPrincipal()), this, SLOT(mostrarPrincipal()));
    connect(peliculas, SIGNAL(volverPrincipal()), this, SLOT(mostrarPrincipal()));
    connect(libros, SIGNAL(volverPrincipal()), this, SLOT(mostrarPrincipal()));
    connect(videos, SIGNAL(volverPrincipal()), this, SLOT(mostrarPrincipal()));

    mostrarSelectorCuentas();
}

void Manager::mostrarSelectorCuentas()
{
    cambiarPantalla(selectorCuentas, false);
}

void Manager::mostrarLogin()
{
    cambiarPantalla(login, false);
}

void Manager::mostrarRegistro()
{
    cambiarPantalla(registro, false);
}

void Manager::mostrarPrincipal()
{
    cambiarPantalla(principal, true);
}

void Manager::mostrarJuegos()
{
    cambiarPantalla(juegos, true);
}

void Manager::mostrarPeliculas()
{
    cambiarPantalla(peliculas, true);
}

void Manager::mostrarLibros()
{
    cambiarPantalla(libros, true);
}

void Manager::mostrarVideos()
{
    cambiarPantalla(videos, true);
}

void Manager::cambiarPantalla(QWidget *pantalla, bool mostrarChat)
{
    pantallas->setCurrentWidget(pantalla);
    chat->setVisible(mostrarChat);
}
