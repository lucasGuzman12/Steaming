QT += widgets sql

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = AdminDB

SOURCES += \
    admindb.cpp \
    authwidget.cpp \
    debugdialog.cpp \
    main.cpp \
    usuario.cpp

HEADERS += \
    admindb.h \
    authwidget.h \
    debugdialog.h \
    usuario.h
