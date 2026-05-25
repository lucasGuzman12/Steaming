#ifndef MANAGER_H
#define MANAGER_H

#include <QWidget>

class Chat;
class Login;
class PantallaRecomendaciones;
class Principal;
class QStackedWidget;
class Registro;
class SelectorCuentas;

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
    PantallaRecomendaciones *juegos;
    PantallaRecomendaciones *peliculas;
    PantallaRecomendaciones *libros;
    PantallaRecomendaciones *videos;

    void cambiarPantalla(QWidget *pantalla, bool mostrarChat);
};

#endif // MANAGER_H
