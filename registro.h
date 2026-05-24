#ifndef REGISTRO_H
#define REGISTRO_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class Registro : public QWidget
{
    Q_OBJECT

public:
    explicit Registro(QWidget *parent = nullptr);

signals:
    void registroCompletado();
    void volverLogin();

private slots:
    void registrarUsuario();
    void regresarLogin();

private:
    QLineEdit *campoNombre;
    QLineEdit *campoApellido;
    QLineEdit *campoEmail;
    QLineEdit *campoContrasena;
    QLineEdit *campoSteamId;
    QPushButton *botonRegistrar;
    QPushButton *botonVolver;
};

#endif // REGISTRO_H
