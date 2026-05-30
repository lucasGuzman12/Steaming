#include "MainWidget.h"
// Punto de entrada de la aplicacion Qt.
#include <QApplication>
using namespace std;

int main(int argc, char *argv[])
{
    // Inicializa el runtime de Qt y el loop principal de eventos.
    QApplication app(argc, argv);

    // Crea y muestra la ventana principal de la aplicacion.
    MainWidget window;
    window.show();

    // Entrega el control al loop de eventos.
    return app.exec();
}
