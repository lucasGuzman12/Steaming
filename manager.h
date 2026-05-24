#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

class Chat;
class Juegos;
class Libros;
class Login;
class Peliculas;
class Principal;
class QStackedWidget;
class Registro;
class SelectorCuentas;
class Videos;

class Manager : public QWidget
{
    Q_OBJECT

public:
    explicit Manager(QWidget *parent = nullptr);

public slots:
    void mostrarSelectorCuentas();
    void mostrarLogin();
    void mostrarRegistro();
    void mostrarPrincipal();
    void mostrarJuegos();
    void mostrarPeliculas();
    void mostrarLibros();
    void mostrarVideos();

private:
    QStackedWidget *pantallas;
    Chat *chat;

    SelectorCuentas *selectorCuentas;
    Login *login;
    Registro *registro;
    Principal *principal;
    Juegos *juegos;
    Peliculas *peliculas;
    Libros *libros;
    Videos *videos;

    void cambiarPantalla(QWidget *pantalla, bool mostrarChat);
};

#endif // MANAGER_H
