#ifndef MAINWIDGET_H
#define MAINWIDGET_H

// Ventana principal de la app: conecta la interfaz con el motor de
// recomendaciones y presenta los resultados al usuario.
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

// Orquesta toda la experiencia de escritorio: inputs, acciones, resultados,
// paginacion de recomendaciones y carga de imagenes.
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

private slots:
    // Consultas directas a servicios individuales.
    void onSteamClicked();
    void onRecomendacionesClicked();
    // Navegacion por bloques cacheados de recomendaciones.
    void onSiguientesRecomendacionesClicked();
    void onSiguientesPeliculasClicked();
    void onSiguientesLibrosClicked();
    void onSiguientesJuegosClicked();
    void onSiguientesVideosClicked();
    // Demos auxiliares para mostrar integraciones especiales.
    void onDemoPaquetePeliculasClicked();
    void onChatbotDemoClicked();
    void onYoutubeClicked();
    void onPeliculasClicked();
    void onLibrosClicked();
    // Actualiza el estado visual cuando Recomendador termina de cargar datos.
    void onRecomendacionesCargadasSignal(const QString &steamId);

private:
    // Helpers de salida para texto, JSON y listas con imagenes.
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

    // Controles visuales principales.
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
    // Recuerda el ultimo Steam ID para reutilizarlo entre acciones.
    QString currentSteamId;
    // Motor principal con toda la logica de negocio.
    Recomendador *recomendador;
};

#endif // MAINWIDGET_H
