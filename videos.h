#ifndef VIDEOS_H
#define VIDEOS_H

#include <QWidget>

class Videos : public QWidget
{
    Q_OBJECT

public:
    explicit Videos(QWidget *parent = nullptr);

signals:
    void volverPrincipal();

private slots:
    void volver();
};

#endif // VIDEOS_H
