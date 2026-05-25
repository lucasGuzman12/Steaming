QT += widgets

CONFIG += c++17

TEMPLATE = app
TARGET = Steaming

SOURCES += \
    Frontend/main.cpp \
    Frontend/manager.cpp \
    Frontend/selectorcuentas.cpp \
    Frontend/login.cpp \
    Frontend/registro.cpp \
    Frontend/principal.cpp \
    Frontend/pantallarecomendaciones.cpp \
    Frontend/chat.cpp

HEADERS += \
    Frontend/manager.h \
    Frontend/selectorcuentas.h \
    Frontend/login.h \
    Frontend/registro.h \
    Frontend/principal.h \
    Frontend/recomendacion.h \
    Frontend/pantallarecomendaciones.h \
    Frontend/chat.h

RESOURCES += \
    resources.qrc
