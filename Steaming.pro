QT += widgets

CONFIG += c++17

TEMPLATE = app
TARGET = Steaming

SOURCES += \
    main.cpp \
    manager.cpp \
    selectorcuentas.cpp \
    login.cpp \
    registro.cpp \
    principal.cpp \
    juegos.cpp \
    peliculas.cpp \
    libros.cpp \
    videos.cpp \
    chat.cpp

HEADERS += \
    manager.h \
    selectorcuentas.h \
    login.h \
    registro.h \
    principal.h \
    juegos.h \
    peliculas.h \
    libros.h \
    videos.h \
    chat.h

RESOURCES += \
    resources.qrc
