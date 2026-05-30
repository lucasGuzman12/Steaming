#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
using namespace std;

class QLabel;
class QListWidget;
class QListWidgetItem;
class QNetworkAccessManager;
class QTextEdit;
class QJsonArray;
class QJsonObject;
class QPushButton;
class Recomendador;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

private slots:
    void onSteamClicked();
    void onRecomendacionesClicked();
    void onSiguientesRecomendacionesClicked();
    void onSiguientesPeliculasClicked();
    void onSiguientesLibrosClicked();
    void onSiguientesJuegosClicked();
    void onSiguientesVideosClicked();
    void onDemoPaquetePeliculasClicked();
    void onChatbotDemoClicked();
    void onYoutubeClicked();
    void onPeliculasClicked();
    void onLibrosClicked();
    void onRecomendacionesCargadasSignal(const QString &steamId);

private:
    void mostrarTexto(const QString &titulo, const QString &contenido);
    void mostrarJson(const QString &titulo, const QJsonObject &objeto);
    void mostrarRecomendaciones(const QJsonObject &objeto);
    void mostrarResultadosConImagen(
        const QString &titulo,
        const QString &resumen,
        const QString &categoria,
        const QJsonArray &items
    );
    void agregarCategoria(
        const QString &titulo,
        const QJsonArray &items
    );
    void agregarImagenAItem(
        QListWidgetItem *item,
        const QString &imageUrl
    );
    QString construirResumenRecomendaciones(
        const QJsonObject &objeto
    ) const;

    QLabel *statusLabel;
    QTextEdit *outputEdit;
    QListWidget *resultList;
    QNetworkAccessManager *networkManager;
    QPushButton *siguientesButton;
    QPushButton *siguientesPeliculasBtn;
    QPushButton *siguientesLibrosBtn;
    QPushButton *siguientesJuegosBtn;
    QPushButton *siguientesVideosBtn;
    QPushButton *demoPaqueteBtn;
    QString currentSteamId;
    Recomendador *recomendador;
};

#endif // MAINWIDGET_H
