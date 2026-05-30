#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class DebugDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugDialog(const QString &username, QWidget *parent = nullptr);

private:
    QLabel *mensaje_label;
    QPushButton *aceptar_button;
};

#endif // DEBUGDIALOG_H
