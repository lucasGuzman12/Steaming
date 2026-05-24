#ifndef JUEGOS_H
#define JUEGOS_H

#include <QWidget>

class Juegos : public QWidget
{
    Q_OBJECT

public:
    explicit Juegos(QWidget *parent = nullptr);

signals:
    void volverPrincipal();

private slots:
    void volver();
};

#endif // JUEGOS_H
