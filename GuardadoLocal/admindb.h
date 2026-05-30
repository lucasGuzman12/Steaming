#ifndef ADMINDB_H
#define ADMINDB_H

#include "usuario.h"

#include <QSqlDatabase>
#include <QString>
#include <vector>

class AdminDB
{
public:
    AdminDB();
    ~AdminDB();

    bool guardar_usuario(const Usuario &usuario);
    void guardar_paquete_recomendacion(int id_usuario, const QString &paquete_recomendacion);
    std::vector<Usuario> get_usuario() const;
    QString get_paquete_recomendacion(int id) const;

    bool validar_credenciales(const QString &username, const QString &contrasenia, Usuario *usuario_encontrado = nullptr) const;
    bool inicializar_base();
    QString get_ultimo_error() const;

private:
    QSqlDatabase base_datos;
    QString ultimo_error;
    QString ruta_sqlite_cli;

    bool abrir_base();
    bool crear_tablas();
    bool sembrar_usuario_prueba();
    bool existe_usuario_por_username(const QString &username) const;
};

#endif // ADMINDB_H
