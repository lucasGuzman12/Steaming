// Incluye la declaracion de la clase SolicitudChatGpt.
#include "SolicitudChatGpt.h"

// Incluye soporte para arreglos JSON.
#include <QJsonArray>
// Incluye soporte para serializar y deserializar documentos JSON.
#include <QJsonDocument>
// Incluye informacion detallada de errores de parseo JSON.
#include <QJsonParseError>

// Abre un namespace anonimo para helpers locales al archivo.
namespace
{
// Define las instrucciones por defecto cuando el llamador no pasa un rol.
QString instruccionesPorDefecto()
{
    // Devuelve el texto base que describe el comportamiento esperado del modelo.
    return
        //  instruccion del sistema.
        "Eres un motor de recomendaciones multimedia para una app de escritorio "
        "en Qt. Debes inferir gustos del usuario a partir de su biblioteca de "
        "Steam y devolver solo JSON.";
}
// Cierra el namespace anonimo.
} // namespace

// Implementa el constructor principal de la solicitud a ChatGPT.
SolicitudChatGpt::SolicitudChatGpt(
    // Recibe el contexto resumido de Steam.
    const QJsonObject &contextoSteam,
    // Recibe la cantidad de items por categoria.
    int itemsPorCategoria,
    // Recibe el nombre del modelo a usar.
    const QString &modelo,
    // Recibe instrucciones opcionales para el modelo.
    const QString &instrucciones,
    // Recibe el nombre del formato estructurado.
    const QString &nombreFormato,
    // Indica si la respuesta se debe generar en background.
    bool ejecutarEnSegundoPlano,
    // Recibe un mensaje libre del usuario para el modo chatbot.
    const QString &mensajeUsuario,
    // Indica si se debe pedir salida estructurada o texto libre.
    bool usarFormatoEstructurado
)
    // Guarda el contexto de Steam recibido por parametro.
    : contextoSteam(contextoSteam),
      // Usa la cantidad recibida o 40 si llega un valor no valido.
      itemsPorCategoria(itemsPorCategoria > 0 ? itemsPorCategoria : 40),
      // Usa el modelo recibido o un modelo por defecto si llega vacio.
      modelo(modelo.trimmed().isEmpty() ? "gpt-5-mini" : modelo.trimmed()),
      // Usa las instrucciones recibidas o las instrucciones por defecto.
      instrucciones(
          instrucciones.trimmed().isEmpty()
              ? instruccionesPorDefecto()
              : instrucciones.trimmed()
      ),
      // Usa el nombre de formato recibido o uno por defecto.
      nombreFormato(
          nombreFormato.trimmed().isEmpty()
              ? "recomendaciones_usuario"
              : nombreFormato.trimmed()
      ),
      // Guarda si la respuesta debe correrse en segundo plano.
      ejecutarEnSegundoPlano(ejecutarEnSegundoPlano),
      // Guarda el mensaje libre del usuario.
      mensajeUsuario(mensajeUsuario),
      // Guarda si el payload debe usar schema JSON.
      usarFormatoEstructurado(usarFormatoEstructurado)
{
}

// Construye el payload que se enviara a la API Responses de OpenAI.
QJsonObject SolicitudChatGpt::crearPayload() const
{
    // Crea el objeto JSON principal del payload.
    QJsonObject payload;
    // Agrega el modelo que debe usar OpenAI.
    payload.insert("model", modelo);
    // Agrega las instrucciones del sistema.
    payload.insert("instructions", instrucciones);
    // Agrega el prompt generado a partir del contexto o del mensaje libre.
    payload.insert("input", construirPrompt());

    // Crea el bloque que describe el formato esperado en la respuesta.
    QJsonObject text;
    // Si se pidio salida estructurada, define un schema JSON estricto.
    if (usarFormatoEstructurado) {
        // Crea el objeto de formato estructurado.
        QJsonObject format;
        // Indica que la salida debe cumplir un json_schema.
        format.insert("type", "json_schema");
        // Asigna un nombre al formato para la API.
        format.insert("name", nombreFormato);
        // Inserta el schema que describe la respuesta esperada.
        format.insert("schema", construirSchema());
        // Fuerza a que el modelo respete estrictamente el schema.
        format.insert("strict", true);
        // Guarda el formato estructurado dentro del bloque text.
        text.insert("format", format);
    } else {
        // Si no hay estructura, pide una salida de texto libre.
        text.insert("format", QJsonObject{{"type", "text"}});
    }
    // Inserta el bloque text completo en el payload final.
    payload.insert("text", text);
    // Si el llamador lo pidio, marca la ejecucion en segundo plano.
    if (ejecutarEnSegundoPlano) {
        // Activa la bandera background en el payload.
        payload.insert("background", true);
    }

    // Devuelve el payload ya listo para enviar.
    return payload;
}

// Interpreta la respuesta cruda devuelta por OpenAI y la normaliza.
QJsonObject SolicitudChatGpt::interpretarRespuesta(const QString &respuesta) const
{
    // Declara una estructura para capturar errores de parseo JSON.
    QJsonParseError error;
    // Intenta convertir la respuesta de texto a documento JSON.
    const QJsonDocument documento = QJsonDocument::fromJson(
        // Convierte la respuesta de QString a UTF-8.
        respuesta.toUtf8(),
        // Guarda en error cualquier problema de parseo.
        &error
    );

    // Si la respuesta no es JSON valido o no es un objeto, devuelve error.
    if (error.error != QJsonParseError::NoError || !documento.isObject()) {
        // Construye y devuelve un objeto de error descriptivo.
        return crearErrorObjeto(
            // Mensaje principal del error.
            "La respuesta de ChatGPT no es un JSON valido.",
            // Guarda la respuesta original para diagnostico.
            respuesta
        );
    }

    // Extrae el objeto raiz del documento JSON.
    const QJsonObject objeto = documento.object();
    // Si la capa de red ya devolvio un error normalizado, lo propaga tal cual.
    if (objeto.value("ok").isBool() && !objeto.value("ok").toBool()) {
        // Devuelve el objeto de error sin reinterpretarlo.
        return objeto;
    }

    // Lee el estado de la respuesta de OpenAI.
    const QString status = objeto.value("status").toString();
    // Si el estado es failed, arma un error mas amigable.
    if (status.compare("failed", Qt::CaseInsensitive) == 0) {
        // Toma el mensaje detallado del error o, si no existe, el codigo.
        const QString detalle = objeto.value("error").toObject().value("message").toString(
            objeto.value("error").toObject().value("code").toString()
        );
        // Devuelve un error explicando que OpenAI no pudo completar la tarea.
        return crearErrorObjeto(
            // Mensaje principal.
            "ChatGPT no pudo completar la respuesta.",
            // Usa el detalle encontrado o, si esta vacio, reutiliza el status.
            detalle.isEmpty() ? status : detalle
        );
    }

    // Si el estado indica incompleto o cancelado, tambien se devuelve error.
    if (status.compare("incomplete", Qt::CaseInsensitive) == 0
        || status.compare("cancelled", Qt::CaseInsensitive) == 0) {
        // Extrae la razon especifica desde incomplete_details.
        const QString detalle = objeto.value("incomplete_details").toObject().value("reason").toString(
            status
        );
        // Devuelve el objeto de error normalizado.
        return crearErrorObjeto(
            // Mensaje principal.
            "ChatGPT devolvio una respuesta incompleta.",
            // Motivo puntual del estado incompleto o cancelado.
            detalle
        );
    }

    // Si hay un estado intermedio distinto de completed, se informa al llamador.
    if (!status.isEmpty()
        && status.compare("completed", Qt::CaseInsensitive) != 0) {
        // Devuelve un error indicando que la respuesta aun no termino.
        return crearErrorObjeto(
            // Mensaje principal.
            "La respuesta de ChatGPT sigue en progreso.",
            // Estado exacto reportado por la API.
            status
        );
    }

    // Extrae el texto util desde los distintos campos posibles de la respuesta.
    const QString textoSalida = extraerTextoSalida(objeto);
    // Si no se pudo extraer texto, devuelve error.
    if (textoSalida.trimmed().isEmpty()) {
        // Devuelve un objeto de error porque no hay contenido aprovechable.
        return crearErrorObjeto(
            // Mensaje principal.
            "ChatGPT no devolvio texto util para parsear."
        );
    }

    // Si el modo es libre, no se intenta interpretar el texto como JSON.
    if (!usarFormatoEstructurado) {
        // Crea el objeto final de respuesta simple.
        QJsonObject resultado;
        // Marca el origen de la respuesta.
        resultado.insert("api", "chatgpt");
        // Marca que la operacion fue exitosa.
        resultado.insert("ok", true);
        // Guarda el modelo que genero la salida.
        resultado.insert("modelo", modelo);
        // Guarda el texto plano devuelto por el modelo.
        resultado.insert("respuesta_texto", textoSalida);
        // Devuelve el objeto de resultado simple.
        return resultado;
    }

    // Intenta parsear el texto extraido como un objeto JSON de recomendaciones.
    const QJsonDocument recomendacionesDocumento = QJsonDocument::fromJson(
        // Convierte el texto extraido a UTF-8.
        textoSalida.toUtf8(),
        // Reutiliza la estructura de error para detectar fallos de parseo.
        &error
    );
    // Si el texto no es JSON valido o no es un objeto, devuelve error.
    if (error.error != QJsonParseError::NoError
        || !recomendacionesDocumento.isObject()) {
        // Devuelve un error explicando que el modelo no respeto el formato esperado.
        return crearErrorObjeto(
            // Mensaje principal.
            "ChatGPT devolvio texto, pero no fue posible convertirlo a JSON.",
            // Guarda el texto problematico para diagnostico.
            textoSalida
        );
    }

    // Crea el objeto final de respuesta estructurada.
    QJsonObject resultado;
    // Marca el origen de la respuesta.
    resultado.insert("api", "chatgpt");
    // Marca que la operacion fue exitosa.
    resultado.insert("ok", true);
    // Guarda el modelo utilizado.
    resultado.insert("modelo", modelo);
    // Inserta el objeto de recomendaciones ya parseado.
    resultado.insert("recomendaciones", recomendacionesDocumento.object());
    // Devuelve la respuesta estructurada final.
    return resultado;
}

// Construye el prompt que se manda al modelo.
QString SolicitudChatGpt::construirPrompt() const
{
    // Si no se usa formato estructurado, se comporta como un chatbot libre.
    if (!usarFormatoEstructurado) {
        // Devuelve un mensaje por defecto o el mensaje escrito por el usuario.
        return mensajeUsuario.trimmed().isEmpty()
            ? QString("Presentate brevemente como un chatbot de ejemplo.")
            : mensajeUsuario.trimmed();
    }

    // Convierte el contexto de Steam a JSON legible para insertarlo en el prompt.
    const QString contextoFormateado = QString::fromUtf8(
        // Serializa el objeto contextoSteam con sangria.
        QJsonDocument(contextoSteam).toJson(QJsonDocument::Indented)
    );

    // Devuelve el prompt completo para generar recomendaciones estructuradas.
    return
        // Pide el analisis de la biblioteca y el numero exacto de recomendaciones.
        "Analiza rapidamente la biblioteca Steam y genera EXACTAMENTE "
        // Inserta dinamicamente la cantidad de items por categoria.
        + QString::number(itemsPorCategoria)
        // Completa las categorias y las reglas de salida.
        + " recomendaciones en cada categoria: peliculas, libros, juegos, "
          "videos YouTube. "
          "Criterios: Deben ser distintas entre si, relacionadas con los "
          "gustos detectados, sin repeticiones. "
          "Formato JSON con: titulo, motivo (1-2 lineas breves), query "
          "(1-3 palabras). "
          "Solo hechos, no especulaciones.\n\n"
          "Biblioteca:\n"
        // Agrega al final el contexto de Steam ya formateado.
        + contextoFormateado;
}

// Construye el schema JSON que debe respetar la respuesta del modelo.
QJsonObject SolicitudChatGpt::construirSchema() const
{
    // Crea el schema base para cada item recomendado.
    const QJsonObject itemSchema = crearItemSchema();

    // Declara las propiedades del objeto perfil.
    QJsonObject perfilProperties;
    // Agrega el campo resumen como string.
    perfilProperties.insert("resumen", QJsonObject{{"type", "string"}});
    // Agrega el campo generos_probables como arreglo de strings.
    perfilProperties.insert(
        "generos_probables",
        QJsonObject{
            // Define el tipo del campo como array.
            {"type", "array"},
            // Define que cada elemento del array es string.
            {"items", QJsonObject{{"type", "string"}}}
        }
    );
    // Agrega el campo franquicias_detectadas como arreglo de strings.
    perfilProperties.insert(
        "franquicias_detectadas",
        QJsonObject{
            // Define el tipo del campo como array.
            {"type", "array"},
            // Define que cada elemento del array es string.
            {"items", QJsonObject{{"type", "string"}}}
        }
    );

    // Declara la lista de campos obligatorios del perfil.
    QJsonArray perfilRequired;
    // Exige que exista el campo resumen.
    perfilRequired.append("resumen");
    // Exige que exista el campo generos_probables.
    perfilRequired.append("generos_probables");
    // Exige que exista el campo franquicias_detectadas.
    perfilRequired.append("franquicias_detectadas");

    // Crea el schema del objeto perfil.
    QJsonObject perfilSchema;
    // Indica que perfil es un objeto JSON.
    perfilSchema.insert("type", "object");
    // Inserta las propiedades permitidas dentro de perfil.
    perfilSchema.insert("properties", perfilProperties);
    // Inserta los campos obligatorios de perfil.
    perfilSchema.insert("required", perfilRequired);
    // Prohibe propiedades extra no declaradas.
    perfilSchema.insert("additionalProperties", false);

    // Declara las propiedades del objeto raiz de recomendaciones.
    QJsonObject properties;
    // Inserta el objeto perfil dentro del schema raiz.
    properties.insert("perfil", perfilSchema);
    // Inserta el arreglo de peliculas usando el schema de item.
    properties.insert("peliculas", crearArraySchema(itemSchema));
    // Inserta el arreglo de libros usando el schema de item.
    properties.insert("libros", crearArraySchema(itemSchema));
    // Inserta el arreglo de juegos usando el schema de item.
    properties.insert("juegos", crearArraySchema(itemSchema));
    // Inserta el arreglo de videos usando el schema de item.
    properties.insert("videos_youtube", crearArraySchema(itemSchema));

    // Declara los campos obligatorios del objeto raiz.
    QJsonArray required;
    // Exige el campo perfil.
    required.append("perfil");
    // Exige el campo peliculas.
    required.append("peliculas");
    // Exige el campo libros.
    required.append("libros");
    // Exige el campo juegos.
    required.append("juegos");
    // Exige el campo videos_youtube.
    required.append("videos_youtube");

    // Crea el schema final del objeto raiz.
    QJsonObject schema;
    // Define que la respuesta completa debe ser un objeto.
    schema.insert("type", "object");
    // Asigna las propiedades permitidas del objeto raiz.
    schema.insert("properties", properties);
    // Asigna los campos obligatorios del objeto raiz.
    schema.insert("required", required);
    // Prohibe propiedades extra en el objeto raiz.
    schema.insert("additionalProperties", false);
    // Devuelve el schema completo.
    return schema;
}

// Extrae el texto generado por OpenAI desde los posibles formatos de salida.
QString SolicitudChatGpt::extraerTextoSalida(const QJsonObject &respuesta) const
{
    // Intenta primero leer el campo directo output_text.
    const QString outputText = respuesta.value("output_text").toString();
    // Si output_text ya trae contenido, lo devuelve directamente.
    if (!outputText.trimmed().isEmpty()) {
        // Devuelve el texto directo entregado por la API.
        return outputText;
    }

    // Declara un acumulador para reconstruir el texto desde output/content.
    QString acumulado;
    // Obtiene el arreglo output de la respuesta.
    const QJsonArray output = respuesta.value("output").toArray();
    // Recorre cada item de output.
    for (const QJsonValue &outputValue : output) {
        // Obtiene el arreglo content del item actual.
        const QJsonArray contenido =
            outputValue.toObject().value("content").toArray();
        // Recorre cada fragmento dentro de content.
        for (const QJsonValue &contenidoValue : contenido) {
            // Convierte el fragmento actual a objeto JSON.
            const QJsonObject contenidoObjeto = contenidoValue.toObject();
            // Si el fragmento es de tipo output_text, lo concatena.
            if (contenidoObjeto.value("type").toString() == "output_text") {
                // Agrega el texto del fragmento al acumulador.
                acumulado += contenidoObjeto.value("text").toString();
            }
        }
    }

    // Devuelve todo el texto reconstruido.
    return acumulado;
}

// Crea un objeto de error uniforme para el resto de la app.
QJsonObject SolicitudChatGpt::crearErrorObjeto(
    // Recibe el mensaje principal del error.
    const QString &mensaje,
    // Recibe un detalle opcional del error.
    const QString &detalle
) const
{
    // Crea el objeto JSON de error.
    QJsonObject error;
    // Indica que el error proviene del modulo chatgpt.
    error.insert("api", "chatgpt");
    // Marca que la operacion fallo.
    error.insert("ok", false);
    // Guarda el mensaje principal del error.
    error.insert("error", mensaje);
    // Si existe detalle, tambien lo agrega al objeto.
    if (!detalle.trimmed().isEmpty()) {
        // Inserta el detalle complementario del error.
        error.insert("detalle", detalle);
    }
    // Devuelve el objeto de error ya construido.
    return error;
}

// Crea el schema base para una recomendacion individual.
QJsonObject SolicitudChatGpt::crearItemSchema() const
{
    // Declara las propiedades del item recomendado.
    QJsonObject propiedades;
    // Agrega el campo titulo como string.
    propiedades.insert("titulo", QJsonObject{{"type", "string"}});
    // Agrega el campo motivo como string.
    propiedades.insert("motivo", QJsonObject{{"type", "string"}});
    // Agrega el campo query como string.
    propiedades.insert("query", QJsonObject{{"type", "string"}});

    // Declara la lista de propiedades obligatorias del item.
    QJsonArray requeridos;
    // Exige que exista el campo titulo.
    requeridos.append("titulo");
    // Exige que exista el campo motivo.
    requeridos.append("motivo");
    // Exige que exista el campo query.
    requeridos.append("query");

    // Crea el schema final del item.
    QJsonObject schema;
    // Define que el item debe ser un objeto.
    schema.insert("type", "object");
    // Inserta las propiedades permitidas del item.
    schema.insert("properties", propiedades);
    // Inserta la lista de campos obligatorios.
    schema.insert("required", requeridos);
    // Prohibe propiedades extra dentro del item.
    schema.insert("additionalProperties", false);
    // Devuelve el schema del item.
    return schema;
}

// Envuelve un itemSchema dentro de un schema de arreglo.
QJsonObject SolicitudChatGpt::crearArraySchema(
    // Recibe el schema del tipo de elemento del arreglo.
    const QJsonObject &itemSchema
) const
{
    // Crea el objeto schema para el arreglo.
    QJsonObject schema;
    // Indica que el tipo es array.
    schema.insert("type", "array");
    // Indica que los elementos del array deben respetar itemSchema.
    schema.insert("items", itemSchema);
    // Devuelve el schema del arreglo.
    return schema;
}
