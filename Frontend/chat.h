#ifndef CHAT_H
#define CHAT_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);

private slots:
    void enviarMensaje();

private:
    QScrollArea *areaMensajes;
    QWidget *contenedorMensajes;
    QVBoxLayout *layoutMensajes;
    QLineEdit *campoMensaje;
    QPushButton *botonEnviar;

    void agregarMensaje(const QString &mensaje, bool esUsuario);
};

#endif // CHAT_H
