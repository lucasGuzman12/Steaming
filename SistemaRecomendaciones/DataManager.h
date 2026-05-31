#ifndef DATAMANAGER_H
#define DATAMANAGER_H

// Base comun para todos los clientes HTTP del proyecto.
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QWidget>
using namespace std;

class QNetworkReply;

// Encapsula configuracion, armado de requests, ejecucion y normalizacion de
// respuestas para que cada API concreta solo defina sus particularidades.
class DataManager : public QObject
{
    Q_OBJECT

protected:
    QString api_gestionar;
    QString link;
    QString key;
    QNetworkAccessManager *manager;
    int requestTimeoutSeconds;
    bool usarBearerAuth;

public:
    explicit DataManager(
        QString api,
        QString url,
        QString apiKey,
        QWidget *parent = nullptr,
        bool usarBearer = false
    );

    virtual ~DataManager();

    // Cada subclase define como resuelve autenticacion y endpoint final.
    virtual QString get(QString endpoint) = 0;
    // Cada subclase define como resuelve autenticacion y endpoint final.
    virtual QString post(QString endpoint, QJsonObject data) = 0;

    // Lee configuracion desde variables de entorno o archivos keys.txt.
    static QString cargarConfiguracion(
        const QString &nombreVariable,
        const QString &fallback = QString()
    );

signals:
    // Expone la respuesta final ya normalizada por la capa de transporte.
    void respuestaRecibida(const QString &api, const QString &respuesta);
    // Expone errores de red o validacion para integrarlos con la UI.
    void errorRecibido(const QString &api, const QString &mensaje);

protected:
    // Arma el request con headers comunes y autenticacion opcional.
    QNetworkRequest crearRequest(QString endpoint);
    // Ejecuta un GET sin que la subclase tenga que repetir la mecanica.
    QString ejecutarGet(QString endpoint);
    // Ejecuta un POST JSON sin que la subclase tenga que repetir la mecanica.
    QString ejecutarPost(QString endpoint, const QJsonObject &data);
    // Inserta parametros en la query respetando URLs absolutas o relativas.
    QString agregarParametroQuery(
        const QString &endpoint,
        const QString &nombre,
        const QString &valor
    ) const;
    // Presenta la respuesta de forma legible si llega JSON valido.
    QString formatearRespuesta(const QByteArray &data) const;
    // Espera el reply y lo transforma en una salida uniforme.
    QString procesarRespuesta(QNetworkReply *reply);
    // Genera un error JSON consistente con el resto de la app.
    QString crearErrorJson(
        const QString &mensaje,
        const QString &endpoint = QString()
    );
};

#endif // DATAMANAGER_H
