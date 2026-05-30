#include "authwidget.h"

#include "debugdialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Acceso al sistema de recomendacion");
    resize(1040, 640);

    if (!admin_db.inicializar_base()) {
        QMessageBox::critical(this, "Error de base de datos", admin_db.get_ultimo_error());
    }

    paginas_stack = new QStackedWidget(this);
    login_page = crear_login_page();
    registro_page = crear_registro_page();
    paginas_stack->addWidget(login_page);
    paginas_stack->addWidget(registro_page);

    QHBoxLayout *layout_principal = new QHBoxLayout(this);
    layout_principal->setContentsMargins(32, 32, 32, 32);
    layout_principal->addWidget(paginas_stack);

    aplicar_estilos();
}

void AuthWidget::intentar_login()
{
    login_estado_label->clear();

    const QString username = login_username_input->text().trimmed();
    const QString contrasenia = login_contrasenia_input->text();

    if (username.isEmpty() || contrasenia.isEmpty()) {
        login_estado_label->setStyleSheet("color: #c0392b; font-weight: 600;");
        login_estado_label->setText("Completa usuario y contrasenia.");
        return;
    }

    Usuario usuario;
    if (!admin_db.validar_credenciales(username, contrasenia, &usuario)) {
        login_estado_label->setStyleSheet("color: #c0392b; font-weight: 600;");
        login_estado_label->setText("Credenciales incorrectas.");
        return;
    }

    login_estado_label->setStyleSheet("color: #148f77; font-weight: 600;");
    login_estado_label->setText("Inicio de sesion correcto.");
    DebugDialog dialogo(usuario.get_username(), this);
    dialogo.exec();
}

void AuthWidget::registrar_usuario()
{
    registro_estado_label->clear();

    const QString nombre = registro_nombre_input->text().trimmed();
    const QString apellido = registro_apellido_input->text().trimmed();
    const QString username = registro_username_input->text().trimmed();
    const QString mail = registro_mail_input->text().trimmed();
    const QString contrasenia = registro_contrasenia_input->text();

    if (nombre.isEmpty() || apellido.isEmpty() || username.isEmpty() || mail.isEmpty() || contrasenia.isEmpty()) {
        registro_estado_label->setStyleSheet("color: #c0392b; font-weight: 600;");
        registro_estado_label->setText("Todos los campos son obligatorios.");
        return;
    }

    Usuario usuario(
        0,
        nombre,
        apellido,
        username,
        mail,
        contrasenia,
        0,
        0,
        0);

    if (!admin_db.guardar_usuario(usuario)) {
        registro_estado_label->setStyleSheet("color: #c0392b; font-weight: 600;");
        registro_estado_label->setText(admin_db.get_ultimo_error().isEmpty() ? "No se pudo registrar el usuario." : admin_db.get_ultimo_error());
        return;
    }

    registro_estado_label->setStyleSheet("color: #148f77; font-weight: 600;");
    registro_estado_label->setText("Usuario registrado correctamente.");
    registro_nombre_input->clear();
    registro_apellido_input->clear();
    registro_username_input->clear();
    registro_mail_input->clear();
    registro_contrasenia_input->clear();
    paginas_stack->setCurrentWidget(login_page);
    login_username_input->setText(username);
}

void AuthWidget::mostrar_login()
{
    paginas_stack->setCurrentWidget(login_page);
}

void AuthWidget::mostrar_registro()
{
    paginas_stack->setCurrentWidget(registro_page);
}

QWidget *AuthWidget::crear_tarjeta(const QString &titulo, const QString &subtitulo)
{
    QFrame *tarjeta = new QFrame(this);
    tarjeta->setObjectName("tarjeta");

    QVBoxLayout *layout = new QVBoxLayout(tarjeta);
    layout->setContentsMargins(36, 36, 36, 36);
    layout->setSpacing(14);

    QLabel *titulo_label = new QLabel(titulo, tarjeta);
    titulo_label->setObjectName("titulo");

    QLabel *subtitulo_label = new QLabel(subtitulo, tarjeta);
    subtitulo_label->setObjectName("subtitulo");
    subtitulo_label->setWordWrap(true);

    layout->addWidget(titulo_label);
    layout->addWidget(subtitulo_label);

    return tarjeta;
}

QWidget *AuthWidget::crear_login_page()
{
    QWidget *pagina = new QWidget(this);

    QWidget *panel_info = crear_tarjeta(
        "Bienvenido al hub de recomendaciones",
        "Consulta usuarios locales, prueba el acceso al sistema y prepara la capa de datos para futuras integraciones con Steam y OpenAI.");

    login_username_input = crear_input("Username");
    login_contrasenia_input = crear_input("Contrasenia", true);
    login_estado_label = new QLabel(pagina);
    login_estado_label->setObjectName("estado");

    QPushButton *ingresar_button = new QPushButton("Ingresar", pagina);
    ingresar_button->setObjectName("primario");
    QPushButton *ir_registro_button = new QPushButton("Crear usuario de prueba", pagina);
    ir_registro_button->setObjectName("secundario");

    connect(ingresar_button, &QPushButton::clicked, this, &AuthWidget::intentar_login);
    connect(ir_registro_button, &QPushButton::clicked, this, &AuthWidget::mostrar_registro);

    QFrame *formulario = new QFrame(pagina);
    formulario->setObjectName("tarjeta");
    QVBoxLayout *form_layout = new QVBoxLayout(formulario);
    form_layout->setContentsMargins(36, 36, 36, 36);
    form_layout->setSpacing(14);

    QLabel *titulo = new QLabel("Iniciar sesion", formulario);
    titulo->setObjectName("titulo");
    QLabel *texto = new QLabel("Usuario de prueba disponible: Admin / 1234", formulario);
    texto->setObjectName("subtitulo");

    form_layout->addWidget(titulo);
    form_layout->addWidget(texto);
    form_layout->addWidget(login_username_input);
    form_layout->addWidget(login_contrasenia_input);
    form_layout->addWidget(login_estado_label);
    form_layout->addWidget(ingresar_button);
    form_layout->addWidget(ir_registro_button);

    QHBoxLayout *layout = new QHBoxLayout(pagina);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(24);
    layout->addWidget(panel_info, 5);
    layout->addWidget(formulario, 4);

    return pagina;
}

QWidget *AuthWidget::crear_registro_page()
{
    QWidget *pagina = new QWidget(this);

    QWidget *panel_info = crear_tarjeta(
        "Registro local",
        "Crea usuarios de prueba dentro de SQLite. Los campos externos como Steam, OpenAI y paquetes de recomendacion se inicializan en 0.");

    registro_nombre_input = crear_input("Nombre");
    registro_apellido_input = crear_input("Apellido");
    registro_username_input = crear_input("Username");
    registro_mail_input = crear_input("Mail");
    registro_contrasenia_input = crear_input("Contrasenia", true);
    registro_estado_label = new QLabel(pagina);
    registro_estado_label->setObjectName("estado");

    QPushButton *registrar_button = new QPushButton("Registrar usuario", pagina);
    registrar_button->setObjectName("primario");
    QPushButton *volver_button = new QPushButton("Volver al login", pagina);
    volver_button->setObjectName("secundario");

    connect(registrar_button, &QPushButton::clicked, this, &AuthWidget::registrar_usuario);
    connect(volver_button, &QPushButton::clicked, this, &AuthWidget::mostrar_login);

    QFrame *formulario = new QFrame(pagina);
    formulario->setObjectName("tarjeta");
    QVBoxLayout *form_layout = new QVBoxLayout(formulario);
    form_layout->setContentsMargins(36, 36, 36, 36);
    form_layout->setSpacing(14);

    QLabel *titulo = new QLabel("Crear cuenta", formulario);
    titulo->setObjectName("titulo");
    QLabel *texto = new QLabel("Interfaz ligera para pruebas locales en Qt Widgets.", formulario);
    texto->setObjectName("subtitulo");

    form_layout->addWidget(titulo);
    form_layout->addWidget(texto);
    form_layout->addWidget(registro_nombre_input);
    form_layout->addWidget(registro_apellido_input);
    form_layout->addWidget(registro_username_input);
    form_layout->addWidget(registro_mail_input);
    form_layout->addWidget(registro_contrasenia_input);
    form_layout->addWidget(registro_estado_label);
    form_layout->addWidget(registrar_button);
    form_layout->addWidget(volver_button);

    QHBoxLayout *layout = new QHBoxLayout(pagina);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(24);
    layout->addWidget(panel_info, 5);
    layout->addWidget(formulario, 4);

    return pagina;
}

QLineEdit *AuthWidget::crear_input(const QString &placeholder, bool es_password)
{
    QLineEdit *input = new QLineEdit(this);
    input->setPlaceholderText(placeholder);
    if (es_password) {
        input->setEchoMode(QLineEdit::Password);
    }
    return input;
}

void AuthWidget::aplicar_estilos()
{
    setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #eef6ff, stop:0.5 #f8fbff, stop:1 #fff7ef);"
        "font-family: 'Segoe UI';"
        "}"
        "#tarjeta {"
        "background: white;"
        "border: 1px solid rgba(66, 98, 118, 0.10);"
        "border-radius: 24px;"
        "}"
        "#titulo {"
        "font-size: 28px;"
        "font-weight: 700;"
        "color: #16324f;"
        "background: transparent;"
        "}"
        "#subtitulo {"
        "font-size: 15px;"
        "line-height: 1.4;"
        "color: #5c7088;"
        "background: transparent;"
        "}"
        "#estado {"
        "min-height: 22px;"
        "color: #c0392b;"
        "font-weight: 600;"
        "background: transparent;"
        "}"
        "QLineEdit {"
        "background: #f7fafc;"
        "border: 1px solid #d6e2ee;"
        "border-radius: 14px;"
        "padding: 12px 14px;"
        "font-size: 14px;"
        "color: #24384d;"
        "}"
        "QLineEdit:focus {"
        "border: 1px solid #1b9aaa;"
        "background: white;"
        "}"
        "QPushButton {"
        "padding: 12px 16px;"
        "border-radius: 14px;"
        "font-size: 14px;"
        "font-weight: 700;"
        "border: none;"
        "}"
        "QPushButton#primario {"
        "background-color: #1b9aaa;"
        "color: white;"
        "}"
        "QPushButton#primario:hover {"
        "background-color: #188a98;"
        "}"
        "QPushButton#secundario {"
        "background-color: #edf4fb;"
        "color: #294661;"
        "}"
        "QPushButton#secundario:hover {"
        "background-color: #e0ecf7;"
        "}");
}
