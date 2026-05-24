#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);

signals:
    void ingresoCorrecto();
    void abrirRegistro();

private slots:
    void intentarIngresar();
    void solicitarRegistro();

private:
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;
    QPushButton *botonIngresar;
    QPushButton *botonRegistro;
};

#endif // LOGIN_H
