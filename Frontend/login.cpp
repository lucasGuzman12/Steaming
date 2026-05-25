#include "login.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

Login::Login(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("login");
    setAttribute(Qt::WA_StyledBackground, true);

    QLabel *titulo = new QLabel("Steaming");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setObjectName("titulo");

    QLabel *subtitulo = new QLabel("Iniciar sesion");
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setObjectName("subtitulo");

    campoUsuario = new QLineEdit;
    campoUsuario->setPlaceholderText("Usuario");

    campoContrasena = new QLineEdit;
    campoContrasena->setPlaceholderText("Contrasena");
    campoContrasena->setEchoMode(QLineEdit::Password);

    botonIngresar = new QPushButton("Ingresar");
    botonRegistro = new QPushButton("Registrate aqui");
    botonRegistro->setObjectName("linkRegistro");
    botonRegistro->setCursor(Qt::PointingHandCursor);

    QLabel *textoRegistro = new QLabel("No estas registrado?");
    textoRegistro->setObjectName("textoRegistro");

    QHBoxLayout *layoutRegistro = new QHBoxLayout;
    layoutRegistro->addStretch();
    layoutRegistro->addWidget(textoRegistro);
    layoutRegistro->addWidget(botonRegistro);
    layoutRegistro->addStretch();
    layoutRegistro->setSpacing(4);

    QVBoxLayout *formulario = new QVBoxLayout;
    formulario->addWidget(titulo);
    formulario->addWidget(subtitulo);
    formulario->addSpacing(16);
    formulario->addWidget(campoUsuario);
    formulario->addWidget(campoContrasena);
    formulario->addWidget(botonIngresar);
    formulario->addLayout(layoutRegistro);
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

    setWindowTitle("Steaming - Login");
    setStyleSheet(
        "#login { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 #ff3df2, stop:0.28 #68268a, stop:0.62 #16224a, stop:1 #070a12); font-family: Segoe UI, Arial; color: #e8f7ff; }"
        "#panelFormulario { background: #101827; border: 1px solid #00e5ff; border-radius: 10px; }"
        "#titulo { background: transparent; font-size: 46px; font-weight: bold; color: #00e5ff; }"
        "#subtitulo { background: transparent; font-size: 20px; color: #ff3df2; }"
        "#textoRegistro { background: transparent; color: #a7b6d8; font-size: 14px; }"
        "QLineEdit { background: #070a12; color: #e8f7ff; border: 1px solid #273a5e; border-radius: 7px; padding: 11px; font-size: 15px; selection-background-color: #ff3df2; }"
        "QLineEdit:focus { border: 1px solid #00e5ff; }"
        "QLineEdit::placeholder { color: #7e8aa8; }"
        "QPushButton { background: #161f33; color: #e8f7ff; border: 1px solid #00e5ff; border-radius: 7px; padding: 11px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background: #00e5ff; color: #070a12; border: 1px solid #9dff00; }"
        "#linkRegistro { background: transparent; color: #00e5ff; border: none; padding: 0; font-size: 14px; font-weight: bold; }"
        "#linkRegistro:hover { color: #9dff00; text-decoration: underline; }"
    );

    connect(botonIngresar, SIGNAL(clicked()), this, SLOT(intentarIngresar()));
    connect(botonRegistro, SIGNAL(clicked()), this, SLOT(solicitarRegistro()));
}

void Login::intentarIngresar()
{
    QString usuario = campoUsuario->text().trimmed();
    QString contrasena = campoContrasena->text();

    if (usuario.isEmpty() || contrasena.isEmpty()) {
        QMessageBox::warning(this, "Datos incompletos", "Ingresa usuario y contrasena.");
        return;
    }

    // Aqui se integraran mas adelante DataManager y AdminDB para validar usuarios.
    emit ingresoCorrecto();
}

void Login::solicitarRegistro()
{
    emit abrirRegistro();
}
