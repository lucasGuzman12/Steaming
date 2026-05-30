#include "MainWidget.h"
#include <QApplication>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWidget window;
    window.show();

    return app.exec();
}
