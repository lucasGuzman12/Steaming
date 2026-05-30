# QT project file for ApiManagerQt

QT += widgets network # Usa los módulos Qt Widgets y Qt Network

CONFIG += c++17 # Compila con estándar C++17

TARGET = ApiManagerQt # Nombre del ejecutable

TEMPLATE = app # Tipo de proyecto: aplicación

SOURCES += \
    main.cpp \
    DataManager.cpp \
    ApiSteamBiblioteca.cpp \
    ApiSteamJuegos.cpp \
    ApiYoutube.cpp \
    ApiPeliculas.cpp \
    ApiLibros.cpp \
    ApiChatGpt.cpp \
    SolicitudChatGpt.cpp \
    Recomendador.cpp \
    MainWidget.cpp # Archivos fuente

HEADERS += \
    DataManager.h \
    ApiSteamBiblioteca.h \
    ApiSteamJuegos.h \
    ApiYoutube.h \
    ApiPeliculas.h \
    ApiLibros.h \
    ApiChatGpt.h \
    SolicitudChatGpt.h \
    Libro.h \
    Pelicula.h \
    Video.h \
    Juego.h \
    PaqueteRecomendaciones.h \
    Recomendador.h \
    MainWidget.h # Archivos de encabezado
