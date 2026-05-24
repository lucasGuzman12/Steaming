#ifndef PELICULAS_H
#define PELICULAS_H

#include <QWidget>

class Peliculas : public QWidget
{
    Q_OBJECT

public:
    explicit Peliculas(QWidget *parent = nullptr);

signals:
    void volverPrincipal();

private slots:
    void volver();
};

#endif // PELICULAS_H
