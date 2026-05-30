#ifndef SOLICITUDCHATGPT_H
#define SOLICITUDCHATGPT_H

#include <QJsonObject>
#include <QString>

class SolicitudChatGpt
{
public:
    SolicitudChatGpt(
        const QJsonObject &contextoSteam = QJsonObject(),
        int itemsPorCategoria = 40,
        const QString &modelo = QString(),
        const QString &instrucciones = QString(),
        const QString &nombreFormato = QString("recomendaciones_usuario"),
        bool ejecutarEnSegundoPlano = false,
        const QString &mensajeUsuario = QString(),
        bool usarFormatoEstructurado = true
    );

    QJsonObject crearPayload() const;
    QJsonObject interpretarRespuesta(const QString &respuesta) const;

    QString construirPrompt() const;
    QJsonObject construirSchema() const;
    QString extraerTextoSalida(const QJsonObject &respuesta) const;

    QJsonObject get_contexto_steam() const { return contextoSteam; }
    int get_items_por_categoria() const { return itemsPorCategoria; }
    QString get_modelo() const { return modelo; }
    QString get_instrucciones() const { return instrucciones; }
    QString get_nombre_formato() const { return nombreFormato; }
    bool get_ejecutar_en_segundo_plano() const { return ejecutarEnSegundoPlano; }
    QString get_mensaje_usuario() const { return mensajeUsuario; }
    bool get_usar_formato_estructurado() const { return usarFormatoEstructurado; }

private:
    QJsonObject crearErrorObjeto(
        const QString &mensaje,
        const QString &detalle = QString()
    ) const;
    QJsonObject crearItemSchema() const;
    QJsonObject crearArraySchema(const QJsonObject &itemSchema) const;

    QJsonObject contextoSteam;
    int itemsPorCategoria;
    QString modelo;
    QString instrucciones;
    QString nombreFormato;
    bool ejecutarEnSegundoPlano;
    QString mensajeUsuario;
    bool usarFormatoEstructurado;
};

#endif // SOLICITUDCHATGPT_H
