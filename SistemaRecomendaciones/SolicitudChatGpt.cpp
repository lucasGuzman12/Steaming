#include "SolicitudChatGpt.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace
{
QString instruccionesPorDefecto()
{
    return
        "Eres un motor de recomendaciones multimedia para una app de escritorio "
        "en Qt. Debes inferir gustos del usuario a partir de su biblioteca de "
        "Steam y devolver solo JSON.";
}
} // namespace

SolicitudChatGpt::SolicitudChatGpt(
    const QJsonObject &contextoSteam,
    int itemsPorCategoria,
    const QString &modelo,
    const QString &instrucciones,
    const QString &nombreFormato,
    bool ejecutarEnSegundoPlano,
    const QString &mensajeUsuario,
    bool usarFormatoEstructurado
)
    : contextoSteam(contextoSteam),
      itemsPorCategoria(itemsPorCategoria > 0 ? itemsPorCategoria : 40),
      modelo(modelo.trimmed().isEmpty() ? "gpt-5-mini" : modelo.trimmed()),
      instrucciones(
          instrucciones.trimmed().isEmpty()
              ? instruccionesPorDefecto()
              : instrucciones.trimmed()
      ),
      nombreFormato(
          nombreFormato.trimmed().isEmpty()
              ? "recomendaciones_usuario"
              : nombreFormato.trimmed()
      ),
      ejecutarEnSegundoPlano(ejecutarEnSegundoPlano),
      mensajeUsuario(mensajeUsuario),
      usarFormatoEstructurado(usarFormatoEstructurado)
{
}

QJsonObject SolicitudChatGpt::crearPayload() const
{
    QJsonObject payload;
    payload.insert("model", modelo);
    payload.insert("instructions", instrucciones);
    payload.insert("input", construirPrompt());

    QJsonObject text;
    if (usarFormatoEstructurado) {
        QJsonObject format;
        format.insert("type", "json_schema");
        format.insert("name", nombreFormato);
        format.insert("schema", construirSchema());
        format.insert("strict", true);
        text.insert("format", format);
    } else {
        text.insert("format", QJsonObject{{"type", "text"}});
    }
    payload.insert("text", text);
    if (ejecutarEnSegundoPlano) {
        payload.insert("background", true);
    }

    return payload;
}

QJsonObject SolicitudChatGpt::interpretarRespuesta(const QString &respuesta) const
{
    QJsonParseError error;
    const QJsonDocument documento = QJsonDocument::fromJson(
        respuesta.toUtf8(),
        &error
    );

    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        return crearErrorObjeto(
            "La respuesta de ChatGPT no es un JSON valido.",
            respuesta
        );
    }

    const QJsonObject objeto = documento.object();
    if (objeto.value("ok").isBool() && !objeto.value("ok").toBool()) {
        return objeto;
    }

    const QString status = objeto.value("status").toString();
    if (status.compare("failed", Qt::CaseInsensitive) == 0) {
        const QString detalle = objeto.value("error").toObject().value("message").toString(
            objeto.value("error").toObject().value("code").toString()
        );
        return crearErrorObjeto(
            "ChatGPT no pudo completar la respuesta.",
            detalle.isEmpty() ? status : detalle
        );
    }

    if (status.compare("incomplete", Qt::CaseInsensitive) == 0
        || status.compare("cancelled", Qt::CaseInsensitive) == 0) {
        const QString detalle = objeto.value("incomplete_details").toObject().value("reason").toString(
            status
        );
        return crearErrorObjeto(
            "ChatGPT devolvio una respuesta incompleta.",
            detalle
        );
    }

    if (!status.isEmpty()
        && status.compare("completed", Qt::CaseInsensitive) != 0) {
        return crearErrorObjeto(
            "La respuesta de ChatGPT sigue en progreso.",
            status
        );
    }

    const QString textoSalida = extraerTextoSalida(objeto);
    if (textoSalida.trimmed().isEmpty()) {
        return crearErrorObjeto(
            "ChatGPT no devolvio texto util para parsear."
        );
    }

    if (!usarFormatoEstructurado) {
        QJsonObject resultado;
        resultado.insert("api", "chatgpt");
        resultado.insert("ok", true);
        resultado.insert("modelo", modelo);
        resultado.insert("respuesta_texto", textoSalida);
        return resultado;
    }

    const QJsonDocument recomendacionesDocumento = QJsonDocument::fromJson(
        textoSalida.toUtf8(),
        &error
    );
    if (error.error != QJsonParseError::NoError
        || !recomendacionesDocumento.isObject()) {
        return crearErrorObjeto(
            "ChatGPT devolvio texto, pero no fue posible convertirlo a JSON.",
            textoSalida
        );
    }

    QJsonObject resultado;
    resultado.insert("api", "chatgpt");
    resultado.insert("ok", true);
    resultado.insert("modelo", modelo);
    resultado.insert("recomendaciones", recomendacionesDocumento.object());
    return resultado;
}

QString SolicitudChatGpt::construirPrompt() const
{
    if (!usarFormatoEstructurado) {
        return mensajeUsuario.trimmed().isEmpty()
            ? QString("Presentate brevemente como un chatbot de ejemplo.")
            : mensajeUsuario.trimmed();
    }

    const QString contextoFormateado = QString::fromUtf8(
        QJsonDocument(contextoSteam).toJson(QJsonDocument::Indented)
    );

    return
        "Analiza rapidamente la biblioteca Steam y genera EXACTAMENTE "
        + QString::number(itemsPorCategoria)
        + " recomendaciones en cada categoria: peliculas, libros, juegos, "
          "videos YouTube. "
          "Criterios: Deben ser distintas entre si, relacionadas con los "
          "gustos detectados, sin repeticiones. "
          "Formato JSON con: titulo, motivo (1-2 lineas breves), query "
          "(1-3 palabras). "
          "Solo hechos, no especulaciones.\n\n"
          "Biblioteca:\n"
        + contextoFormateado;
}

QJsonObject SolicitudChatGpt::construirSchema() const
{
    const QJsonObject itemSchema = crearItemSchema();

    QJsonObject perfilProperties;
    perfilProperties.insert("resumen", QJsonObject{{"type", "string"}});
    perfilProperties.insert(
        "generos_probables",
        QJsonObject{
            {"type", "array"},
            {"items", QJsonObject{{"type", "string"}}}
        }
    );
    perfilProperties.insert(
        "franquicias_detectadas",
        QJsonObject{
            {"type", "array"},
            {"items", QJsonObject{{"type", "string"}}}
        }
    );

    QJsonArray perfilRequired;
    perfilRequired.append("resumen");
    perfilRequired.append("generos_probables");
    perfilRequired.append("franquicias_detectadas");

    QJsonObject perfilSchema;
    perfilSchema.insert("type", "object");
    perfilSchema.insert("properties", perfilProperties);
    perfilSchema.insert("required", perfilRequired);
    perfilSchema.insert("additionalProperties", false);

    QJsonObject properties;
    properties.insert("perfil", perfilSchema);
    properties.insert("peliculas", crearArraySchema(itemSchema));
    properties.insert("libros", crearArraySchema(itemSchema));
    properties.insert("juegos", crearArraySchema(itemSchema));
    properties.insert("videos_youtube", crearArraySchema(itemSchema));

    QJsonArray required;
    required.append("perfil");
    required.append("peliculas");
    required.append("libros");
    required.append("juegos");
    required.append("videos_youtube");

    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", properties);
    schema.insert("required", required);
    schema.insert("additionalProperties", false);
    return schema;
}

QString SolicitudChatGpt::extraerTextoSalida(const QJsonObject &respuesta) const
{
    const QString outputText = respuesta.value("output_text").toString();
    if (!outputText.trimmed().isEmpty()) {
        return outputText;
    }

    QString acumulado;
    const QJsonArray output = respuesta.value("output").toArray();
    for (const QJsonValue &outputValue : output) {
        const QJsonArray contenido =
            outputValue.toObject().value("content").toArray();
        for (const QJsonValue &contenidoValue : contenido) {
            const QJsonObject contenidoObjeto = contenidoValue.toObject();
            if (contenidoObjeto.value("type").toString() == "output_text") {
                acumulado += contenidoObjeto.value("text").toString();
            }
        }
    }

    return acumulado;
}

QJsonObject SolicitudChatGpt::crearErrorObjeto(
    const QString &mensaje,
    const QString &detalle
) const
{
    QJsonObject error;
    error.insert("api", "chatgpt");
    error.insert("ok", false);
    error.insert("error", mensaje);
    if (!detalle.trimmed().isEmpty()) {
        error.insert("detalle", detalle);
    }
    return error;
}

QJsonObject SolicitudChatGpt::crearItemSchema() const
{
    QJsonObject propiedades;
    propiedades.insert("titulo", QJsonObject{{"type", "string"}});
    propiedades.insert("motivo", QJsonObject{{"type", "string"}});
    propiedades.insert("query", QJsonObject{{"type", "string"}});

    QJsonArray requeridos;
    requeridos.append("titulo");
    requeridos.append("motivo");
    requeridos.append("query");

    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", propiedades);
    schema.insert("required", requeridos);
    schema.insert("additionalProperties", false);
    return schema;
}

QJsonObject SolicitudChatGpt::crearArraySchema(
    const QJsonObject &itemSchema
) const
{
    QJsonObject schema;
    schema.insert("type", "array");
    schema.insert("items", itemSchema);
    return schema;
}
