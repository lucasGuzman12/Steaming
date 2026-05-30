#include "MainWidget.h"

#include "Recomendador.h"

#include <QApplication>
#include <QDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QTextEdit>
#include <QVBoxLayout>
using namespace std;

namespace
{
void mostrarDialogoVector(
    QWidget *parent,
    const QString &titulo,
    const QString &contenido
)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(titulo);
    dialog.resize(800, 600);

    auto *layout = new QVBoxLayout(&dialog);
    auto *texto = new QTextEdit(&dialog);
    auto *cerrarBtn = new QPushButton("Cerrar", &dialog);

    texto->setReadOnly(true);
    texto->setPlainText(contenido);

    layout->addWidget(texto);
    layout->addWidget(cerrarBtn);

    QObject::connect(cerrarBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}
} // namespace

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      statusLabel(new QLabel(this)),
      outputEdit(new QTextEdit(this)),
      resultList(new QListWidget(this)),
      networkManager(new QNetworkAccessManager(this)),
      demoPaqueteBtn(new QPushButton("Demo paquete peliculas", this)),
      currentSteamId(QString()),
      recomendador(new Recomendador(this))
{
    setWindowTitle("Recomendador multimedia con Steam y ChatGPT");
    resize(1100, 700);

    auto *titleLabel = new QLabel(
        "Arquitectura Qt/C++: Steam -> ChatGPT -> APIs de detalle",
        this
    );
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *descriptionLabel = new QLabel(
        "Ingresa un Steam ID y el recomendador obtendra la biblioteca, "
        "pedira sugerencias a ChatGPT y luego buscara libros, peliculas, "
        "juegos y videos en sus APIs correspondientes. Tambien puedes abrir "
        "Chatbot Demo para ver un ejemplo de uso de SolicitudChatGpt.",
        this
    );
    descriptionLabel->setWordWrap(true);

    auto *steamButton = new QPushButton("Biblioteca Steam", this);
    auto *recomendacionesButton = new QPushButton("Recomendador", this);
    auto *youtubeButton = new QPushButton("YouTube", this);
    auto *peliculasButton = new QPushButton("Peliculas", this);
    auto *librosButton = new QPushButton("Libros", this);
    auto *chatbotDemoButton = new QPushButton("Chatbot Demo", this);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(steamButton);
    buttonLayout->addWidget(recomendacionesButton);
    buttonLayout->addWidget(youtubeButton);
    buttonLayout->addWidget(peliculasButton);
    buttonLayout->addWidget(librosButton);
    buttonLayout->addWidget(chatbotDemoButton);

    siguientesButton = new QPushButton("Siguientes Recomendaciones", this);
    siguientesButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #ff5555; }"
    );
    buttonLayout->addStretch();
    buttonLayout->addWidget(siguientesButton);

    siguientesPeliculasBtn = new QPushButton("Sig. PelÃ­culas", this);
    siguientesLibrosBtn = new QPushButton("Sig. Libros", this);
    siguientesJuegosBtn = new QPushButton("Sig. Juegos", this);
    siguientesVideosBtn = new QPushButton("Sig. Videos", this);

    auto btnStyle = "QPushButton { background-color: #2196F3; color: white; padding: 5px; }"
                    "QPushButton:hover { background-color: #0b7dda; }";
    siguientesPeliculasBtn->setStyleSheet(btnStyle);
    siguientesLibrosBtn->setStyleSheet(btnStyle);
    siguientesJuegosBtn->setStyleSheet(btnStyle);
    siguientesVideosBtn->setStyleSheet(btnStyle);

    auto *categoryButtonLayout = new QHBoxLayout();
    categoryButtonLayout->addWidget(siguientesPeliculasBtn);
    categoryButtonLayout->addWidget(siguientesLibrosBtn);
    categoryButtonLayout->addWidget(siguientesJuegosBtn);
    categoryButtonLayout->addWidget(siguientesVideosBtn);
    demoPaqueteBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; padding: 5px; }"
        "QPushButton:hover { background-color: #f57c00; }"
    );
    categoryButtonLayout->addWidget(demoPaqueteBtn);
    categoryButtonLayout->addStretch();

    outputEdit->setReadOnly(true);
    outputEdit->setPlaceholderText(
        "Aqui se mostraran los resumenes y respuestas JSON."
    );

    resultList->setVisible(false);
    resultList->setIconSize(QSize(128, 128));
    resultList->setSpacing(8);
    resultList->setWordWrap(true);

    statusLabel->setWordWrap(true);
    statusLabel->setText(
        "Selecciona una accion para consultar la biblioteca o generar recomendaciones."
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(categoryButtonLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(outputEdit);
    mainLayout->addWidget(resultList);

    connect(steamButton, &QPushButton::clicked, this, &MainWidget::onSteamClicked);
    connect(
        recomendacionesButton,
        &QPushButton::clicked,
        this,
        &MainWidget::onRecomendacionesClicked
    );
    connect(youtubeButton, &QPushButton::clicked, this, &MainWidget::onYoutubeClicked);
    connect(
        peliculasButton,
        &QPushButton::clicked,
        this,
        &MainWidget::onPeliculasClicked
    );
    connect(librosButton, &QPushButton::clicked, this, &MainWidget::onLibrosClicked);
    connect(
        siguientesButton,
        &QPushButton::clicked,
        this,
        &MainWidget::onSiguientesRecomendacionesClicked
    );
    connect(
        siguientesPeliculasBtn,
        &QPushButton::clicked,
        this,
        &MainWidget::onSiguientesPeliculasClicked
    );
    connect(
        siguientesLibrosBtn,
        &QPushButton::clicked,
        this,
        &MainWidget::onSiguientesLibrosClicked
    );
    connect(
        siguientesJuegosBtn,
        &QPushButton::clicked,
        this,
        &MainWidget::onSiguientesJuegosClicked
    );
    connect(
        siguientesVideosBtn,
        &QPushButton::clicked,
        this,
        &MainWidget::onSiguientesVideosClicked
    );
    connect(
        demoPaqueteBtn,
        &QPushButton::clicked,
        this,
        &MainWidget::onDemoPaquetePeliculasClicked
    );
    connect(
        chatbotDemoButton,
        &QPushButton::clicked,
        this,
        &MainWidget::onChatbotDemoClicked
    );
    connect(
        recomendador,
        &Recomendador::recomendacionesCargadas,
        this,
        &MainWidget::onRecomendacionesCargadasSignal
    );
}

MainWidget::~MainWidget() = default;

void MainWidget::onSteamClicked()
{
    bool ok = false;
    const QString steamId = QInputDialog::getText(
        this,
        "Biblioteca Steam",
        "Ingresa el Steam ID64:",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || steamId.isEmpty()) {
        return;
    }

    mostrarJson(
        "Biblioteca de Steam",
        recomendador->obtenerBibliotecaSteam(steamId, 5)
    );
}

void MainWidget::onRecomendacionesClicked()
{
    bool ok = false;
    const QString steamId = QInputDialog::getText(
        this,
        "Recomendador",
        "Ingresa el Steam ID64 del usuario:",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || steamId.isEmpty()) {
        return;
    }

    currentSteamId = steamId;
    statusLabel->setText(
        "Generando recomendaciones. Esto puede tardar mientras se consulta Steam, ChatGPT y las APIs de detalle."
    );
    outputEdit->setVisible(true);
    outputEdit->setPlainText(
        "Armando perfil del usuario y buscando recomendaciones..."
    );
    QApplication::processEvents();
    mostrarRecomendaciones(recomendador->recomendar(steamId, 5, 5));
    siguientesButton->setEnabled(true);
}

void MainWidget::onYoutubeClicked()
{
    bool ok = false;
    const QString texto = QInputDialog::getText(
        this,
        "YouTube",
        "Texto a buscar en YouTube:",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || texto.isEmpty()) {
        return;
    }

    const QJsonObject respuesta = recomendador->buscarVideosYoutube(texto, 5);
    if (respuesta.value("ok").isBool() && !respuesta.value("ok").toBool()) {
        mostrarJson("Error en YouTube", respuesta);
        return;
    }

    mostrarResultadosConImagen(
        "Resultados de YouTube con portada.",
        "Se muestran los videos encontrados con su miniatura, titulo y canal.",
        "Videos de YouTube",
        respuesta.value("resultados").toArray()
    );
}

void MainWidget::onPeliculasClicked()
{
    bool ok = false;
    const QString nombre = QInputDialog::getText(
        this,
        "Peliculas",
        "Nombre de la pelicula:",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || nombre.isEmpty()) {
        return;
    }

    mostrarTexto(
        "Busqueda de peliculas",
        recomendador->buscarPeliculas(nombre)
    );
}

void MainWidget::onLibrosClicked()
{
    bool ok = false;
    const QString titulo = QInputDialog::getText(
        this,
        "Libros",
        "Titulo del libro:",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok || titulo.isEmpty()) {
        return;
    }

    const QJsonObject respuesta = recomendador->buscarLibros(titulo, 5);
    if (respuesta.value("ok").isBool() && !respuesta.value("ok").toBool()) {
        mostrarJson("Error en libros", respuesta);
        return;
    }

    mostrarResultadosConImagen(
        "Resultados de libros con portada.",
        "Se muestran los libros encontrados con su portada, titulo y autor.",
        "Libros",
        respuesta.value("resultados").toArray()
    );
}

void MainWidget::onSiguientesRecomendacionesClicked()
{
    if (currentSteamId.isEmpty()) {
        statusLabel->setText("Por favor genera recomendaciones primero.");
        return;
    }

    statusLabel->setText("Cargando siguientes recomendaciones...");
    const QJsonObject resultado = recomendador->obtenerSiguientesRecomendaciones(currentSteamId, 5);
    
    if (!resultado.value("ok").toBool()) {
        mostrarJson("Error al obtener recomendaciones", resultado);
        statusLabel->setText("Error: revisa el panel de salida para detalles.");
        return;
    }
    
    mostrarRecomendaciones(resultado);
}

void MainWidget::onSiguientesPeliculasClicked()
{
    if (currentSteamId.isEmpty()) {
        statusLabel->setText("Por favor genera recomendaciones primero.");
        return;
    }

    const QJsonArray peliculas = recomendador->obtenerSiguientesPeliculas(currentSteamId, 5);
    
    if (peliculas.isEmpty()) {
        statusLabel->setText("No hay pelÃ­culas disponibles.");
        return;
    }

    resultList->clear();
    resultList->setVisible(true);
    outputEdit->setVisible(true);
    statusLabel->setText("Siguientes pelÃ­culas recomendadas");
    outputEdit->setPlainText("Mostrando siguientes 5 pelÃ­culas recomendadas.");
    agregarCategoria("PelÃ­culas Recomendadas", peliculas);
}

void MainWidget::onSiguientesLibrosClicked()
{
    if (currentSteamId.isEmpty()) {
        statusLabel->setText("Por favor genera recomendaciones primero.");
        return;
    }

    const QJsonArray libros = recomendador->obtenerSiguientesLibros(currentSteamId, 5);
    
    if (libros.isEmpty()) {
        statusLabel->setText("No hay libros disponibles.");
        return;
    }

    resultList->clear();
    resultList->setVisible(true);
    outputEdit->setVisible(true);
    statusLabel->setText("Siguientes libros recomendados");
    outputEdit->setPlainText("Mostrando siguientes 5 libros recomendados.");
    agregarCategoria("Libros Recomendados", libros);
}

void MainWidget::onSiguientesJuegosClicked()
{
    if (currentSteamId.isEmpty()) {
        statusLabel->setText("Por favor genera recomendaciones primero.");
        return;
    }

    const QJsonArray juegos = recomendador->obtenerSiguientesJuegos(currentSteamId, 5);
    
    if (juegos.isEmpty()) {
        statusLabel->setText("No hay juegos disponibles.");
        return;
    }

    resultList->clear();
    resultList->setVisible(true);
    outputEdit->setVisible(true);
    statusLabel->setText("Siguientes juegos recomendados");
    outputEdit->setPlainText("Mostrando siguientes 5 juegos recomendados.");
    agregarCategoria("Juegos Recomendados", juegos);
}

void MainWidget::onSiguientesVideosClicked()
{
    if (currentSteamId.isEmpty()) {
        statusLabel->setText("Por favor genera recomendaciones primero.");
        return;
    }

    const QJsonArray videos = recomendador->obtenerSiguientesVideos(currentSteamId, 5);
    
    if (videos.isEmpty()) {
        statusLabel->setText("No hay videos disponibles.");
        return;
    }

    resultList->clear();
    resultList->setVisible(true);
    outputEdit->setVisible(true);
    statusLabel->setText("Siguientes videos recomendados");
    outputEdit->setPlainText("Mostrando siguientes 5 videos recomendados.");
    agregarCategoria("Videos Recomendados", videos);
}

void MainWidget::onDemoPaquetePeliculasClicked()
{
    QString steamId = currentSteamId;

    if (steamId.isEmpty()) {
        bool ok = false;
        steamId = QInputDialog::getText(
            this,
            "Demo paquete peliculas",
            "Ingresa el Steam ID64 del usuario:",
            QLineEdit::Normal,
            QString(),
            &ok
        ).trimmed();

        if (!ok || steamId.isEmpty()) {
            return;
        }
    }

    currentSteamId = steamId;
    statusLabel->setText("Ejecutando demo de get_paquete_recomendaciones()...");
    outputEdit->setVisible(true);
    outputEdit->setPlainText(
        "Generando el paquete completo de recomendaciones..."
    );
    QApplication::processEvents();

    const PaqueteRecomendaciones paquete =
        recomendador->get_paquete_recomendaciones(steamId, 5);
    const std::vector<Pelicula> peliculas = paquete.get_peliculas();
    const QString contenidoVector =
        recomendador->construirSalidaVectorPeliculas(paquete);

    mostrarTexto("Contenido del vector de peliculas", contenidoVector);
    mostrarDialogoVector(this, "Vector de peliculas", contenidoVector);

    if (!paquete.get_ok() || peliculas.empty()) {
        statusLabel->setText(
            paquete.get_error().isEmpty()
                ? "El vector de peliculas esta vacio."
                : paquete.get_error()
        );
        return;
    }

    statusLabel->setText(
        "Mostrando solo el contenido del vector de peliculas."
    );
}

void MainWidget::onChatbotDemoClicked()
{
    bool ok = false;
    const QString mensajeUsuario = QInputDialog::getMultiLineText(
        this,
        "Chatbot Demo",
        "Escribe un mensaje para el chatbot de ejemplo:",
        "Hola, podrias explicarme para que sirve esta app?",
        &ok
    ).trimmed();

    if (!ok || mensajeUsuario.isEmpty()) {
        return;
    }

    statusLabel->setText(
        "Ejecutando demo de chatbot con SolicitudChatGpt..."
    );
    outputEdit->setVisible(true);
    outputEdit->setPlainText(
        "Armando la solicitud del chatbot y consultando ChatGPT..."
    );
    resultList->clear();
    resultList->setVisible(false);
    QApplication::processEvents();

    const QJsonObject respuesta = recomendador->ejecutarChatbotDemo(
        mensajeUsuario
    );
    if (respuesta.value("ok").isBool() && !respuesta.value("ok").toBool()) {
        mostrarJson("Error en chatbot demo", respuesta);
        return;
    }

    const QString payloadPretty = QString::fromUtf8(
        QJsonDocument(respuesta.value("payload").toObject()).toJson(
            QJsonDocument::Indented
        )
    );

    const QString modelo = respuesta.value("modelo").toString("gpt");
    QString ejemploCodigo;
    ejemploCodigo += "SolicitudChatGpt solicitud(\n";
    ejemploCodigo += "    QJsonObject(),\n";
    ejemploCodigo += "    1,\n";
    ejemploCodigo += "    \"" + modelo + "\",\n";
    ejemploCodigo += "    \"Eres un chatbot de ejemplo para una app Qt...\",\n";
    ejemploCodigo += "    \"chat_demo\",\n";
    ejemploCodigo += "    true,\n";
    ejemploCodigo += "    mensajeUsuario,\n";
    ejemploCodigo += "    false\n";
    ejemploCodigo += ");\n";

    QString salida;
    salida += "Demo de uso de SolicitudChatGpt para un chatbot\n\n";
    salida += "Mensaje del usuario:\n";
    salida += respuesta.value("mensaje_usuario").toString();
    salida += "\n\n";
    salida += "Codigo de ejemplo:\n";
    salida += ejemploCodigo;
    salida += "\n";
    salida += "Prompt generado:\n";
    salida += respuesta.value("prompt").toString();
    salida += "\n\n";
    salida += "Payload enviado:\n";
    salida += payloadPretty;
    salida += "\n";
    salida += "Respuesta del asistente:\n";
    salida += respuesta.value("respuesta_texto").toString();

    mostrarTexto("Demo chatbot con SolicitudChatGpt", salida);
}

void MainWidget::onRecomendacionesCargadasSignal(const QString &steamId)
{
    statusLabel->setText("âœ“ Recomendaciones cargadas para " + steamId + 
                         " - Usa los botones para navegar por cada categorÃ­a");
}

void MainWidget::mostrarTexto(const QString &titulo, const QString &contenido)
{
    statusLabel->setText(titulo);
    resultList->clear();
    resultList->setVisible(false);
    outputEdit->setVisible(true);
    outputEdit->setPlainText(contenido);
}

void MainWidget::mostrarJson(
    const QString &titulo,
    const QJsonObject &objeto
)
{
    statusLabel->setText(titulo);
    resultList->clear();
    resultList->setVisible(false);
    outputEdit->setVisible(true);
    outputEdit->setPlainText(
        QString::fromUtf8(
            QJsonDocument(objeto).toJson(QJsonDocument::Indented)
        )
    );
}

void MainWidget::mostrarRecomendaciones(const QJsonObject &objeto)
{
    if (objeto.value("ok").isBool() && !objeto.value("ok").toBool()) {
        mostrarJson("Error al generar recomendaciones", objeto);
        return;
    }

    statusLabel->setText(
        "Recomendaciones generadas y separadas por categoria."
    );
    outputEdit->setVisible(true);
    outputEdit->setPlainText(construirResumenRecomendaciones(objeto));

    resultList->clear();
    resultList->setVisible(true);
    agregarCategoria("Peliculas", objeto.value("peliculas").toArray());
    agregarCategoria("Libros", objeto.value("libros").toArray());
    agregarCategoria("Juegos", objeto.value("juegos").toArray());
    agregarCategoria(
        "Videos de YouTube",
        objeto.value("videos_youtube").toArray()
    );
}

void MainWidget::mostrarResultadosConImagen(
    const QString &titulo,
    const QString &resumen,
    const QString &categoria,
    const QJsonArray &items
)
{
    statusLabel->setText(titulo);
    outputEdit->setVisible(true);
    outputEdit->setPlainText(resumen);

    resultList->clear();
    resultList->setVisible(true);
    agregarCategoria(categoria, items);
}

void MainWidget::agregarCategoria(
    const QString &titulo,
    const QJsonArray &items
)
{
    QListWidgetItem *header = new QListWidgetItem(titulo, resultList);
    QFont headerFont = header->font();
    headerFont.setBold(true);
    headerFont.setPointSize(headerFont.pointSize() + 1);
    header->setFont(headerFont);
    header->setFlags(Qt::ItemIsEnabled);

    if (items.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem(
            "Sin resultados para esta categoria.",
            resultList
        );
        emptyItem->setFlags(Qt::ItemIsEnabled);
        return;
    }

    for (const QJsonValue &value : items) {
        const QJsonObject itemObject = value.toObject();

        const QString tituloItem = itemObject.value("titulo").toString("Sin titulo");
        QString detalle;

        if (itemObject.contains("autor")) {
            detalle = itemObject.value("autor").toString();
            if (itemObject.value("anio").toInt() > 0) {
                detalle += " | " + QString::number(itemObject.value("anio").toInt());
            }
        } else if (itemObject.contains("canal")) {
            detalle = itemObject.value("canal").toString();
        } else if (itemObject.contains("empresa")) {
            detalle = itemObject.value("empresa").toString();
            if (itemObject.value("precio").isDouble()) {
                detalle += " | $" + QString::number(
                    itemObject.value("precio").toDouble(),
                    'f',
                    2
                );
            }
        }

        QString textoItem = tituloItem;
        if (!detalle.trimmed().isEmpty()) {
            textoItem += "\n" + detalle;
        }

        const QString motivo = itemObject.value("motivo").toString();
        if (!motivo.trimmed().isEmpty()) {
            textoItem += "\n" + motivo;
        }

        QListWidgetItem *item = new QListWidgetItem(textoItem, resultList);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setSizeHint(QSize(0, 110));

        agregarImagenAItem(item, itemObject.value("portada").toString());
    }
}

void MainWidget::agregarImagenAItem(
    QListWidgetItem *item,
    const QString &imageUrl
)
{
    if (item == nullptr || imageUrl.trimmed().isEmpty()) {
        return;
    }

    QNetworkRequest request{QUrl(imageUrl)};
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
    );
    request.setRawHeader("User-Agent", "Mozilla/5.0");

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, item]() {
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pix;
            pix.loadFromData(reply->readAll());
            if (!pix.isNull()) {
                item->setIcon(QIcon(pix));
            }
        }
        reply->deleteLater();
    });
}

QString MainWidget::construirResumenRecomendaciones(
    const QJsonObject &objeto
) const
{
    return recomendador->construirResumenRecomendaciones(objeto);
}
