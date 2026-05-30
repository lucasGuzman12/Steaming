#include "DataManager.h"

// Implementa la capa de transporte HTTP compartida por todos los clientes.
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
using namespace std;

namespace
{
// Une una base y un endpoint respetando slashes y queries.
QString unirUrl(const QString &base, const QString &endpoint)
{
    if (base.isEmpty()) {
        return endpoint;
    }

    if (endpoint.isEmpty()) {
        return base;
    }

    QString normalizedBase = base;
    QString normalizedEndpoint = endpoint;

    if (normalizedBase.endsWith('/') && normalizedEndpoint.startsWith('/')) {
        normalizedEndpoint.remove(0, 1);
    } else if (!normalizedBase.endsWith('/')
               && !normalizedEndpoint.startsWith('/')
               && !normalizedEndpoint.startsWith('?')) {
        normalizedBase.append('/');
    }

    return normalizedBase + normalizedEndpoint;
}

// Convierte un endpoint relativo en URL absoluta o respeta una URL ya completa.
QUrl resolverUrl(const QString &base, const QString &endpoint)
{
    const QUrl endpointUrl(endpoint);
    if (endpointUrl.isValid() && !endpointUrl.scheme().isEmpty()) {
        return endpointUrl;
    }

    return QUrl(unirUrl(base, endpoint));
}

// Recorre varias rutas candidatas para encontrar archivos de configuracion
// tanto desde el repo como desde el ejecutable compilado.
QStringList rutasBasePosibles()
{
    QStringList rutas;
    rutas << QDir::currentPath();

    if (QCoreApplication::instance() != nullptr) {
        rutas << QCoreApplication::applicationDirPath();
    }

    QStringList candidatas;
    QSet<QString> visitadas;

    for (const QString &ruta : rutas) {
        QDir dir(ruta);
        for (int i = 0; i < 8; ++i) {
            const QString absoluta = dir.absolutePath();
            if (!visitadas.contains(absoluta)) {
                visitadas.insert(absoluta);
                candidatas << absoluta;
            }

            if (!dir.cdUp()) {
                break;
            }
        }
    }

    return candidatas;
}

// Busca keys.txt o keys.example.txt en la raiz o en una carpeta config/.
QString encontrarArchivoConfiguracion(const QString &fileName)
{
    for (const QString &basePath : rutasBasePosibles()) {
        const QStringList candidates = {
            QDir(basePath).filePath(fileName),
            QDir(basePath).filePath("config/" + fileName)
        };

        for (const QString &candidate : candidates) {
            if (QFileInfo::exists(candidate)) {
                return QDir::cleanPath(candidate);
            }
        }
    }

    return QString();
}

// Lee archivos key=value ignorando comentarios y lineas vacias.
QHash<QString, QString> leerArchivoClaveValor(const QString &filePath)
{
    QHash<QString, QString> values;
    if (filePath.isEmpty()) {
        return values;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return values;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString rawLine = stream.readLine().trimmed();
        if (rawLine.isEmpty() || rawLine.startsWith('#') || !rawLine.contains('=')) {
            continue;
        }

        const QStringList parts = rawLine.split('=', Qt::KeepEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        const QString key = parts.first().trimmed();
        const QString value = rawLine.mid(rawLine.indexOf('=') + 1).trimmed();
        values.insert(key, value);
    }

    return values;
}

// Cachea la configuracion para evitar releer archivos en cada request.
const QHash<QString, QString> &configuracionCacheada()
{
    static const QHash<QString, QString> config = []() {
        QHash<QString, QString> values;

        const QString exampleFile =
            encontrarArchivoConfiguracion("keys.example.txt");
        const QString keysFile =
            encontrarArchivoConfiguracion("keys.txt");

        const QList<QHash<QString, QString>> sources = {
            leerArchivoClaveValor(exampleFile),
            leerArchivoClaveValor(keysFile)
        };

        for (const auto &source : sources) {
            for (auto it = source.constBegin(); it != source.constEnd(); ++it) {
                values.insert(it.key(), it.value());
            }
        }

        return values;
    }();

    return config;
}
} // namespace

DataManager::DataManager(
    QString api,
    QString url,
    QString apiKey,
    QWidget *parent,
    bool usarBearer
)
    : QObject(parent),
      api_gestionar(api),
      link(url),
      key(apiKey),
      manager(new QNetworkAccessManager(this)),
      requestTimeoutSeconds(cargarConfiguracion("REQUEST_TIMEOUT", "180").toInt()),
      usarBearerAuth(usarBearer)
{
    // Asegura un timeout razonable aunque la configuracion sea invalida.
    if (requestTimeoutSeconds <= 0) {
        requestTimeoutSeconds = 180;
    }
}

DataManager::~DataManager() = default;

QString DataManager::cargarConfiguracion(
    const QString &nombreVariable,
    const QString &fallback
)
{
    // Las variables de entorno tienen prioridad sobre los archivos locales.
    const QByteArray envName = nombreVariable.toUtf8();
    if (qEnvironmentVariableIsSet(envName.constData())) {
        return QString::fromUtf8(qgetenv(envName.constData()));
    }

    const auto &values = configuracionCacheada();
    if (values.contains(nombreVariable)) {
        return values.value(nombreVariable);
    }

    // Si no existe valor definido, usa el fallback recibido.
    return fallback;
}

QNetworkRequest DataManager::crearRequest(QString endpoint)
{
    // Resuelve la URL final combinando la base del cliente y el endpoint.
    const QUrl url = resolverUrl(link, endpoint);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    // Para APIs como OpenAI la autenticacion se envia como Bearer token.
    if (usarBearerAuth && !key.trimmed().isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + key.toUtf8());
    }

    request.setTransferTimeout(requestTimeoutSeconds * 1000);
    return request;
}

QString DataManager::ejecutarGet(QString endpoint)
{
    // Dispara el GET y delega el tratamiento uniforme del reply.
    QNetworkReply *reply = manager->get(crearRequest(endpoint));
    return procesarRespuesta(reply);
}

QString DataManager::ejecutarPost(QString endpoint, const QJsonObject &data)
{
    // Serializa el cuerpo como JSON compacto antes de enviarlo.
    const QJsonDocument document(data);
    QNetworkReply *reply = manager->post(
        crearRequest(endpoint),
        document.toJson(QJsonDocument::Compact)
    );
    return procesarRespuesta(reply);
}

QString DataManager::agregarParametroQuery(
    const QString &endpoint,
    const QString &nombre,
    const QString &valor
) const
{
    // No modifica la URL si el valor esta vacio.
    if (valor.trimmed().isEmpty()) {
        return endpoint;
    }

    QUrl url = resolverUrl(link, endpoint);
    QUrlQuery query(url);

    // Evita duplicar parametros si la URL ya los contiene.
    if (!query.hasQueryItem(nombre)) {
        query.addQueryItem(nombre, valor);
    }

    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

QString DataManager::formatearRespuesta(const QByteArray &data) const
{
    // Si no vino cuerpo, devuelve un JSON minimo pero consistente.
    if (data.trimmed().isEmpty()) {
        QJsonObject emptyResponse;
        emptyResponse.insert("api", api_gestionar);
        emptyResponse.insert("ok", true);
        emptyResponse.insert("mensaje", "La API devolvio una respuesta vacia.");
        return QString::fromUtf8(
            QJsonDocument(emptyResponse).toJson(QJsonDocument::Indented)
        );
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    // Si no se pudo parsear como JSON, preserva el texto crudo.
    if (parseError.error != QJsonParseError::NoError) {
        return QString::fromUtf8(data);
    }

    if (document.isObject()) {
        return QString::fromUtf8(
            QJsonDocument(document.object()).toJson(QJsonDocument::Indented)
        );
    }

    if (document.isArray()) {
        return QString::fromUtf8(
            QJsonDocument(document.array()).toJson(QJsonDocument::Indented)
        );
    }

    return QString::fromUtf8(data);
}

QString DataManager::procesarRespuesta(QNetworkReply *reply)
{
    // Protege contra errores previos a la solicitud real.
    if (reply == nullptr) {
        return crearErrorJson("No se pudo crear la solicitud de red.");
    }

    // Espera de forma sincronica para simplificar el flujo del resto del codigo.
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute
    ).toInt();
    const QByteArray body = reply->readAll();

    // Si la capa de red informa error, arma un JSON detallado y homogeno.
    if (reply->error() != QNetworkReply::NoError) {
        QJsonObject errorObject;
        errorObject.insert("api", api_gestionar);
        errorObject.insert("ok", false);
        errorObject.insert("url", reply->url().toString());
        errorObject.insert("error", reply->errorString());

        if (statusCode > 0) {
            errorObject.insert("status", statusCode);
        }

        if (!body.isEmpty()) {
            QJsonParseError parseError;
            const QJsonDocument detailDoc = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (detailDoc.isObject()) {
                    errorObject.insert("detalle", detailDoc.object());
                } else if (detailDoc.isArray()) {
                    errorObject.insert("detalle", detailDoc.array());
                }
            } else {
                errorObject.insert("detalle_texto", QString::fromUtf8(body));
            }
        }

        const QString response = QString::fromUtf8(
            QJsonDocument(errorObject).toJson(QJsonDocument::Indented)
        );

        emit errorRecibido(api_gestionar, reply->errorString());
        emit respuestaRecibida(api_gestionar, response);
        reply->deleteLater();
        return response;
    }

    // Si la respuesta fue exitosa, solo se formatea y propaga.
    const QString response = formatearRespuesta(body);
    emit respuestaRecibida(api_gestionar, response);
    reply->deleteLater();
    return response;
}

QString DataManager::crearErrorJson(
    const QString &mensaje,
    const QString &endpoint
)
{
    // Construye un error comun para que la UI no tenga que distinguir origenes.
    QJsonObject errorObject;
    errorObject.insert("api", api_gestionar);
    errorObject.insert("ok", false);
    errorObject.insert("error", mensaje);
    errorObject.insert("base_url", link);

    if (!endpoint.isEmpty()) {
        errorObject.insert("endpoint", endpoint);
    }

    const QString response = QString::fromUtf8(
        QJsonDocument(errorObject).toJson(QJsonDocument::Indented)
    );

    emit errorRecibido(api_gestionar, mensaje);
    emit respuestaRecibida(api_gestionar, response);
    return response;
}
