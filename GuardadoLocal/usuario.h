#ifndef USUARIO_H
#define USUARIO_H

#include <QString>

class Usuario
{
public:
    int id_usuario;
    QString nombre;
    QString apellido;
    QString username;
    QString mail;
    QString contrasenia;
    int id_steam;
    int key_openai;
    int id_paquete_recomendacion;

    Usuario();
    Usuario(int id_usuario,
            const QString &nombre,
            const QString &apellido,
            const QString &username,
            const QString &mail,
            const QString &contrasenia,
            int id_steam,
            int key_openai,
            int id_paquete_recomendacion);

    int get_id() const;
    int get_key_openai() const;
    int get_id_paquete_recomendacion() const;
    QString get_username() const;
};

#endif // USUARIO_H
