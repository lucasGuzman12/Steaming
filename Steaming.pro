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
    pantallarecomendaciones.cpp \
    chat.cpp

HEADERS += \
    manager.h \
    selectorcuentas.h \
    login.h \
    registro.h \
    principal.h \
    recomendacion.h \
    pantallarecomendaciones.h \
    chat.h

RESOURCES += \
    resources.qrc
