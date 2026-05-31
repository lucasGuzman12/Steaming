QT += widgets network sql

CONFIG += c++17

TEMPLATE = app
TARGET = Steaming

INCLUDEPATH += \
    Frontend \
    GuardadoLocal \
    SistemaRecomendaciones

SOURCES += \
    Frontend/main.cpp \
    Frontend/manager.cpp \
    Frontend/selectorcuentas.cpp \
    Frontend/login.cpp \
    Frontend/registro.cpp \
    Frontend/principal.cpp \
    Frontend/conversorrecomendaciones.cpp \
    Frontend/pantallarecomendaciones.cpp \
    Frontend/chat.cpp \
    GuardadoLocal/admindb.cpp \
    GuardadoLocal/authwidget.cpp \
    GuardadoLocal/debugdialog.cpp \
    GuardadoLocal/usuario.cpp \
    SistemaRecomendaciones/ApiChatGpt.cpp \
    SistemaRecomendaciones/ApiLibros.cpp \
    SistemaRecomendaciones/ApiPeliculas.cpp \
    SistemaRecomendaciones/ApiSteamBiblioteca.cpp \
    SistemaRecomendaciones/ApiSteamJuegos.cpp \
    SistemaRecomendaciones/ApiYoutube.cpp \
    SistemaRecomendaciones/DataManager.cpp \
    SistemaRecomendaciones/MainWidget.cpp \
    SistemaRecomendaciones/Recomendador.cpp \
    SistemaRecomendaciones/ServiciosApi.cpp \
    SistemaRecomendaciones/SolicitudChatGpt.cpp

HEADERS += \
    Frontend/manager.h \
    Frontend/selectorcuentas.h \
    Frontend/login.h \
    Frontend/registro.h \
    Frontend/principal.h \
    Frontend/conversorrecomendaciones.h \
    Frontend/recomendacion.h \
    Frontend/pantallarecomendaciones.h \
    Frontend/chat.h \
    GuardadoLocal/admindb.h \
    GuardadoLocal/authwidget.h \
    GuardadoLocal/debugdialog.h \
    GuardadoLocal/usuario.h \
    SistemaRecomendaciones/ApiChatGpt.h \
    SistemaRecomendaciones/ApiLibros.h \
    SistemaRecomendaciones/ApiPeliculas.h \
    SistemaRecomendaciones/ApiSteamBiblioteca.h \
    SistemaRecomendaciones/ApiSteamJuegos.h \
    SistemaRecomendaciones/ApiYoutube.h \
    SistemaRecomendaciones/DataManager.h \
    SistemaRecomendaciones/Juego.h \
    SistemaRecomendaciones/Libro.h \
    SistemaRecomendaciones/MainWidget.h \
    SistemaRecomendaciones/PaqueteRecomendaciones.h \
    SistemaRecomendaciones/Pelicula.h \
    SistemaRecomendaciones/Recomendador.h \
    SistemaRecomendaciones/ServiciosApi.h \
    SistemaRecomendaciones/SolicitudChatGpt.h \
    SistemaRecomendaciones/Video.h

RESOURCES += \
    resources.qrc
