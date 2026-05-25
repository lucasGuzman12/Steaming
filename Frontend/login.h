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
    void volverSelector();

private slots:
    void intentarIngresar();
    void solicitarRegistro();
    void solicitarSelector();

private:
    QLineEdit *campoEmail;
    QLineEdit *campoContrasena;
    QPushButton *botonVolver;
    QPushButton *botonIngresar;
    QPushButton *botonRegistro;
};

#endif // LOGIN_H
