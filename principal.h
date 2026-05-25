#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QString>
#include <QWidget>

class QPushButton;

class Principal : public QWidget
{
    Q_OBJECT

public:
    explicit Principal(QWidget *parent = nullptr);

signals:
    void abrirJuegos();
    void abrirPeliculas();
    void abrirLibros();
    void abrirVideos();

private slots:
    void seleccionarJuegos();
    void seleccionarPeliculas();
    void seleccionarLibros();
    void seleccionarVideos();

private:
    QPushButton *crearBloque(const QString &titulo, const QString &rutaImagen);
};

#endif // PRINCIPAL_H
