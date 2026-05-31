#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include "admindb.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;

class AuthWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWidget(QWidget *parent = nullptr);

private slots:
    void intentar_login();
    void registrar_usuario();
    void mostrar_login();
    void mostrar_registro();

private:
    AdminDB admin_db;

    QStackedWidget *paginas_stack;

    QWidget *login_page;
    QLineEdit *login_username_input;
    QLineEdit *login_contrasenia_input;
    QLabel *login_estado_label;

    QWidget *registro_page;
    QLineEdit *registro_nombre_input;
    QLineEdit *registro_apellido_input;
    QLineEdit *registro_username_input;
    QLineEdit *registro_mail_input;
    QLineEdit *registro_contrasenia_input;
    QLabel *registro_estado_label;

    QWidget *crear_tarjeta(const QString &titulo, const QString &subtitulo);
    QWidget *crear_login_page();
    QWidget *crear_registro_page();
    QLineEdit *crear_input(const QString &placeholder, bool es_password = false);
    void aplicar_estilos();
};

#endif // AUTHWIDGET_H
