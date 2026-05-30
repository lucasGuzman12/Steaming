#include "debugdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DebugDialog::DebugDialog(const QString &username, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Dialogo de debug");
    setModal(true);
    resize(320, 180);

    mensaje_label = new QLabel(QString("Hola %1").arg(username), this);
    mensaje_label->setAlignment(Qt::AlignCenter);

    aceptar_button = new QPushButton("Aceptar", this);
    connect(aceptar_button, &QPushButton::clicked, this, &DebugDialog::accept);

    QVBoxLayout *contenedor = new QVBoxLayout(this);
    contenedor->setContentsMargins(24, 24, 24, 24);
    contenedor->setSpacing(18);
    contenedor->addWidget(mensaje_label);

    QHBoxLayout *acciones = new QHBoxLayout();
    acciones->addStretch();
    acciones->addWidget(aceptar_button);
    acciones->addStretch();
    contenedor->addLayout(acciones);

    setStyleSheet(
        "QDialog {"
        "background: white;"
        "border-radius: 16px;"
        "}"
        "QLabel {"
        "font-size: 22px;"
        "font-weight: 700;"
        "color: #20304a;"
        "}"
        "QPushButton {"
        "background-color: #00b894;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 10px 18px;"
        "font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "background-color: #00a383;"
        "}");
}
