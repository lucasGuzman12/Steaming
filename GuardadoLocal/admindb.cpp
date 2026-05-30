#include "admindb.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
const char *NOMBRE_CONEXION = "conexion_admin_db";
}

AdminDB::AdminDB()
    : ruta_sqlite_cli("C:/Qt/Database/sqlite3.exe")
{
    if (QSqlDatabase::contains(NOMBRE_CONEXION)) {
        base_datos = QSqlDatabase::database(NOMBRE_CONEXION);
    } else {
        base_datos = QSqlDatabase::addDatabase("QSQLITE", NOMBRE_CONEXION);
        const QString ruta_base = QCoreApplication::applicationDirPath() + "/recomendador_local.sqlite";
        base_datos.setDatabaseName(ruta_base);
    }
}

AdminDB::~AdminDB()
{
    if (base_datos.isOpen()) {
        base_datos.close();
    }
}

bool AdminDB::inicializar_base()
{
    if (!abrir_base()) {
        return false;
    }

    if (!crear_tablas()) {
        return false;
    }

    return sembrar_usuario_prueba();
}

bool AdminDB::guardar_usuario(const Usuario &usuario)
{
    if (!abrir_base()) {
        return false;
    }

    if (existe_usuario_por_username(usuario.username)) {
        ultimo_error = "El username ya existe en la base local.";
        return false;
    }

    QSqlQuery consulta(base_datos);
    consulta.prepare(
        "INSERT INTO usuarios "
        "(nombre, apellido, username, mail, contrasenia, id_steam, key_openai, id_paquete_recomendacion) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    consulta.addBindValue(usuario.nombre);
    consulta.addBindValue(usuario.apellido);
    consulta.addBindValue(usuario.username);
    consulta.addBindValue(usuario.mail);
    consulta.addBindValue(usuario.contrasenia);
    consulta.addBindValue(usuario.id_steam);
    consulta.addBindValue(usuario.key_openai);
    consulta.addBindValue(usuario.id_paquete_recomendacion);

    if (!consulta.exec()) {
        ultimo_error = consulta.lastError().text();
        return false;
    }

    return true;
}

std::vector<Usuario> AdminDB::get_usuario() const
{
    std::vector<Usuario> usuarios;

    if (!base_datos.isOpen()) {
        return usuarios;
    }

    QSqlQuery consulta(base_datos);
    if (!consulta.exec(
            "SELECT id_usuario, nombre, apellido, username, mail, contrasenia, id_steam, key_openai, id_paquete_recomendacion "
            "FROM usuarios ORDER BY id_usuario ASC")) {
        return usuarios;
    }

    while (consulta.next()) {
        usuarios.emplace_back(
            consulta.value(0).toInt(),
            consulta.value(1).toString(),
            consulta.value(2).toString(),
            consulta.value(3).toString(),
            consulta.value(4).toString(),
            consulta.value(5).toString(),
            consulta.value(6).toInt(),
            consulta.value(7).toInt(),
            consulta.value(8).toInt());
    }

    return usuarios;
}

bool AdminDB::validar_credenciales(const QString &username, const QString &contrasenia, Usuario *usuario_encontrado) const
{
    if (!base_datos.isOpen()) {
        return false;
    }

    QSqlQuery consulta(base_datos);
    consulta.prepare(
        "SELECT id_usuario, nombre, apellido, username, mail, contrasenia, id_steam, key_openai, id_paquete_recomendacion "
        "FROM usuarios WHERE username = ? AND contrasenia = ?");
    consulta.addBindValue(username);
    consulta.addBindValue(contrasenia);

    if (!consulta.exec() || !consulta.next()) {
        return false;
    }

    if (usuario_encontrado != nullptr) {
        *usuario_encontrado = Usuario(
            consulta.value(0).toInt(),
            consulta.value(1).toString(),
            consulta.value(2).toString(),
            consulta.value(3).toString(),
            consulta.value(4).toString(),
            consulta.value(5).toString(),
            consulta.value(6).toInt(),
            consulta.value(7).toInt(),
            consulta.value(8).toInt());
    }

    return true;
}

QString AdminDB::get_ultimo_error() const
{
    return ultimo_error;
}

bool AdminDB::abrir_base()
{
    if (base_datos.isOpen()) {
        return true;
    }

    if (!base_datos.open()) {
        ultimo_error = base_datos.lastError().text();
        return false;
    }

    return true;
}

bool AdminDB::crear_tablas()
{
    QSqlQuery consulta(base_datos);
    if (!consulta.exec(
            "CREATE TABLE IF NOT EXISTS usuarios ("
            "id_usuario INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT NOT NULL,"
            "apellido TEXT NOT NULL,"
            "username TEXT NOT NULL UNIQUE,"
            "mail TEXT NOT NULL,"
            "contrasenia TEXT NOT NULL,"
            "id_steam INTEGER NOT NULL DEFAULT 0,"
            "key_openai INTEGER NOT NULL DEFAULT 0,"
            "id_paquete_recomendacion INTEGER NOT NULL DEFAULT 0"
            ")")) {
        ultimo_error = consulta.lastError().text();
        return false;
    }

    return true;
}

bool AdminDB::sembrar_usuario_prueba()
{
    if (existe_usuario_por_username("Admin")) {
        return true;
    }

    Usuario usuario_admin(
        0,
        "Administrador",
        "Principal",
        "Admin",
        "admin@demo.local",
        "1234",
        0,
        0,
        0);

    return guardar_usuario(usuario_admin);
}

bool AdminDB::existe_usuario_por_username(const QString &username) const
{
    if (!base_datos.isOpen()) {
        return false;
    }

    QSqlQuery consulta(base_datos);
    consulta.prepare("SELECT COUNT(*) FROM usuarios WHERE username = ?");
    consulta.addBindValue(username);

    if (!consulta.exec() || !consulta.next()) {
        return false;
    }

    return consulta.value(0).toInt() > 0;
}
