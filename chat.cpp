#include "chat.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

Chat::Chat(QWidget *parent)
    : QWidget(parent)
{
    QLabel *titulo = new QLabel("Chat");
    titulo->setObjectName("chatTitle");

    contenedorMensajes = new QWidget;
    contenedorMensajes->setObjectName("contenedorMensajes");

    layoutMensajes = new QVBoxLayout(contenedorMensajes);
    layoutMensajes->setContentsMargins(12, 12, 12, 12);
    layoutMensajes->setSpacing(8);
    layoutMensajes->addStretch();

    areaMensajes = new QScrollArea;
    areaMensajes->setObjectName("areaMensajes");
    areaMensajes->setWidget(contenedorMensajes);
    areaMensajes->setWidgetResizable(true);
    areaMensajes->setFrameShape(QFrame::NoFrame);
    areaMensajes->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    campoMensaje = new QLineEdit;
    campoMensaje->setPlaceholderText("Escribe un mensaje");

    botonEnviar = new QPushButton("Enviar");

    QHBoxLayout *layoutEntrada = new QHBoxLayout;
    layoutEntrada->addWidget(campoMensaje);
    layoutEntrada->addWidget(botonEnviar);

    QVBoxLayout *layoutBloque = new QVBoxLayout;
    layoutBloque->addWidget(titulo);
    layoutBloque->addWidget(areaMensajes);
    layoutBloque->addLayout(layoutEntrada);
    layoutBloque->setContentsMargins(12, 12, 12, 12);
    layoutBloque->setSpacing(10);

    QFrame *bloqueChat = new QFrame;
    bloqueChat->setObjectName("bloqueChat");
    bloqueChat->setLayout(layoutBloque);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->addWidget(bloqueChat);
    layoutPrincipal->setContentsMargins(0, 24, 0, 24);

    setMinimumWidth(280);
    setMaximumWidth(340);
    setObjectName("chatPanel");
    setStyleSheet(
        "#chatPanel { background: transparent; color: #e8f7ff; }"
        "#bloqueChat { background: #0b1020; border: 1px solid #00e5ff; border-radius: 12px; }"
        "#chatTitle { background: transparent; color: #9dff00; font-size: 20px; font-weight: bold; padding: 8px; }"
        "#areaMensajes { background: #070a12; border: 1px solid #263859; border-radius: 8px; }"
        "#areaMensajes > QWidget { background: #070a12; border-radius: 8px; }"
        "#contenedorMensajes { background: #070a12; border-radius: 8px; }"
        "QLineEdit { background: #070a12; color: #e8f7ff; border: 1px solid #263859; border-radius: 7px; padding: 8px; selection-background-color: #ff3df2; }"
        "QLineEdit:focus { border: 1px solid #00e5ff; }"
        "QLineEdit::placeholder { color: #7e8aa8; }"
        "QPushButton { background: #161f33; color: #e8f7ff; border: 1px solid #9dff00; border-radius: 7px; padding: 8px 12px; font-weight: bold; }"
        "QPushButton:hover { background: #9dff00; color: #070a12; border: 1px solid #00e5ff; }"
        "QScrollBar:vertical { background: #0b1020; width: 8px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #00e5ff; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    agregarMensaje("Hola, soy el asistente de Steaming. Preguntame por juegos, peliculas, libros o videos.", false);

    connect(botonEnviar, SIGNAL(clicked()), this, SLOT(enviarMensaje()));
    connect(campoMensaje, SIGNAL(returnPressed()), this, SLOT(enviarMensaje()));
}

void Chat::enviarMensaje()
{
    QString mensaje = campoMensaje->text().trimmed();

    if (mensaje.isEmpty()) {
        return;
    }

    agregarMensaje(mensaje, true);
    campoMensaje->clear();

    // Aqui se integrara mas adelante la API de OpenAI.
    agregarMensaje("Por ahora soy una respuesta automatica de prueba.", false);
}

void Chat::agregarMensaje(const QString &mensaje, bool esUsuario)
{
    QWidget *fila = new QWidget;
    fila->setObjectName("filaMensaje");

    QHBoxLayout *layoutFila = new QHBoxLayout(fila);
    layoutFila->setContentsMargins(0, 0, 0, 0);

    QLabel *burbuja = new QLabel(mensaje);
    burbuja->setTextFormat(Qt::PlainText);
    burbuja->setWordWrap(true);
    burbuja->setMaximumWidth(230);
    burbuja->setTextInteractionFlags(Qt::TextSelectableByMouse);

    if (esUsuario) {
        burbuja->setStyleSheet(
            "QLabel { background: #00e5ff; color: #070a12; border: 1px solid #9dff00; "
            "border-radius: 12px; padding: 9px 11px; font-weight: bold; }"
        );
        layoutFila->addStretch();
        layoutFila->addWidget(burbuja);
    } else {
        burbuja->setStyleSheet(
            "QLabel { background: #151f35; color: #e8f7ff; border: 1px solid #ff3df2; "
            "border-radius: 12px; padding: 9px 11px; }"
        );
        layoutFila->addWidget(burbuja);
        layoutFila->addStretch();
    }

    layoutMensajes->insertWidget(layoutMensajes->count() - 1, fila);

    QTimer::singleShot(0, this, [this]() {
        areaMensajes->verticalScrollBar()->setValue(areaMensajes->verticalScrollBar()->maximum());
    });
}
