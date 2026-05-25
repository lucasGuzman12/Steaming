#ifndef SELECTORCUENTAS_H
#define SELECTORCUENTAS_H

#include <QWidget>

class QEvent;
class QPushButton;

class SelectorCuentas : public QWidget
{
    Q_OBJECT

public:
    explicit SelectorCuentas(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void cuentaSeleccionada();
    void agregarCuenta();

private slots:
    void seleccionarCuenta();
    void solicitarAgregarCuenta();

private:
    QPushButton *crearBotonCuenta(const QString &nombre, const QString &email);
    void actualizarHoverCuenta(QPushButton *boton, bool activo);
};

#endif // SELECTORCUENTAS_H
