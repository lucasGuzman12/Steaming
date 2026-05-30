#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QWidget>
using namespace std;

class QNetworkReply;

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

    virtual QString get(QString endpoint) = 0;
    virtual QString post(QString endpoint, QJsonObject data) = 0;

    static QString cargarConfiguracion(
        const QString &nombreVariable,
        const QString &fallback = QString()
    );

signals:
    void respuestaRecibida(const QString &api, const QString &respuesta);
    void errorRecibido(const QString &api, const QString &mensaje);

protected:
    QNetworkRequest crearRequest(QString endpoint);
    QString ejecutarGet(QString endpoint);
    QString ejecutarPost(QString endpoint, const QJsonObject &data);
    QString agregarParametroQuery(
        const QString &endpoint,
        const QString &nombre,
        const QString &valor
    ) const;
    QString formatearRespuesta(const QByteArray &data) const;
    QString procesarRespuesta(QNetworkReply *reply);
    QString crearErrorJson(
        const QString &mensaje,
        const QString &endpoint = QString()
    );
};

#endif // DATAMANAGER_H
