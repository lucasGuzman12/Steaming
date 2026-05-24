#include "registro.h"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

Registro::Registro(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("registro");
    setAttribute(Qt::WA_StyledBackground, true);

    QLabel *titulo = new QLabel("Crear cuenta");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setObjectName("titulo");

    campoNombre = new QLineEdit;
    campoNombre->setPlaceholderText("Nombre");

    campoApellido = new QLineEdit;
    campoApellido->setPlaceholderText("Apellido");

    campoEmail = new QLineEdit;
    campoEmail->setPlaceholderText("Email");

    campoContrasena = new QLineEdit;
    campoContrasena->setPlaceholderText("Contrasena");
    campoContrasena->setEchoMode(QLineEdit::Password);

    campoSteamId = new QLineEdit;
    campoSteamId->setPlaceholderText("ID de Steam");

    botonRegistrar = new QPushButton("Registrar");
    botonVolver = new QPushButton("Volver al login");

    QVBoxLayout *formulario = new QVBoxLayout;
    formulario->addWidget(titulo);
    formulario->addSpacing(16);
    formulario->addWidget(campoNombre);
    formulario->addWidget(campoApellido);
    formulario->addWidget(campoEmail);
    formulario->addWidget(campoContrasena);
    formulario->addWidget(campoSteamId);
    formulario->addWidget(botonRegistrar);
    formulario->addWidget(botonVolver);
    formulario->setContentsMargins(80, 40, 80, 40);
    formulario->setSpacing(12);

    QWidget *panelFormulario = new QWidget;
    panelFormulario->setObjectName("panelFormulario");
    panelFormulario->setLayout(formulario);
    panelFormulario->setMaximumWidth(520);

    QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
    layoutPrincipal->addStretch();
    layoutPrincipal->addWidget(panelFormulario, 0, Qt::AlignCenter);
    layoutPrincipal->addStretch();

    setWindowTitle("Steaming - Registro");
    setStyleSheet(
        "#registro { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "#panelFormulario { background: #101827; border: 1px solid #ff3df2; border-radius: 10px; }"
        "#titulo { background: transparent; font-size: 36px; font-weight: bold; color: #00e5ff; }"
        "QLineEdit { background: #070a12; color: #e8f7ff; border: 1px solid #273a5e; border-radius: 7px; padding: 11px; font-size: 15px; selection-background-color: #ff3df2; }"
        "QLineEdit:focus { border: 1px solid #ff3df2; }"
        "QLineEdit::placeholder { color: #7e8aa8; }"
        "QPushButton { background: #161f33; color: #e8f7ff; border: 1px solid #ff3df2; border-radius: 7px; padding: 11px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #ff3df2; color: #070a12; border: 1px solid #00e5ff; }"
    );

    connect(botonRegistrar, SIGNAL(clicked()), this, SLOT(registrarUsuario()));
    connect(botonVolver, SIGNAL(clicked()), this, SLOT(regresarLogin()));
}

void Registro::registrarUsuario()
{
    QString nombre = campoNombre->text().trimmed();
    QString apellido = campoApellido->text().trimmed();
    QString email = campoEmail->text().trimmed();
    QString contrasena = campoContrasena->text();
    QString steamId = campoSteamId->text().trimmed();

    if (nombre.isEmpty() || apellido.isEmpty() || email.isEmpty() || contrasena.isEmpty() || steamId.isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Completa todos los campos.");
        return;
    }

    // Aqui se integraran mas adelante DataManager y AdminDB para guardar usuarios.
    QMessageBox::information(this, "Registro", "Usuario registrado de forma simulada.");
    emit registroCompletado();
}

void Registro::regresarLogin()
{
    emit volverLogin();
}
