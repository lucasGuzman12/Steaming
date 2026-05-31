#include "authwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication aplicacion(argc, argv);

    AuthWidget ventana;
    ventana.show();

    return aplicacion.exec();
}
