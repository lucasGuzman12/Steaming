#include "usuario.h"

Usuario::Usuario()
    : id_usuario(0),
      id_steam(0),
      key_openai(0),
      id_paquete_recomendacion(0)
{
}

Usuario::Usuario(int id_usuario,
                 const QString &nombre,
                 const QString &apellido,
                 const QString &username,
                 const QString &mail,
                 const QString &contrasenia,
                 int id_steam,
                 int key_openai,
                 int id_paquete_recomendacion)
    : id_usuario(id_usuario),
      nombre(nombre),
      apellido(apellido),
      username(username),
      mail(mail),
      contrasenia(contrasenia),
      id_steam(id_steam),
      key_openai(key_openai),
      id_paquete_recomendacion(id_paquete_recomendacion)
{
}

int Usuario::get_id() const
{
    return id_usuario;
}

int Usuario::get_key_openai() const
{
    return key_openai;
}

int Usuario::get_id_paquete_recomendacion() const
{
    return id_paquete_recomendacion;
}

QString Usuario::get_username() const
{
    return username;
}
