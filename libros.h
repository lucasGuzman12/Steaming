#ifndef LIBROS_H
#define LIBROS_H

#include <QWidget>

class Libros : public QWidget
{
    Q_OBJECT

public:
    explicit Libros(QWidget *parent = nullptr);

signals:
    void volverPrincipal();

private slots:
    void volver();
};

#endif // LIBROS_H
