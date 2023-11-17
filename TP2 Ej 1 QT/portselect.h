#ifndef PORTSELECT_H
#define PORTSELECT_H

#include <QDialog>

namespace Ui {
class portselect;
}

class portselect : public QDialog
{
    Q_OBJECT

public:
    explicit portselect(QWidget *parent = 0);
    ~portselect();
    Ui::portselect *ui;
private slots:
    void on_pb_aceptar_clicked();

private:
};

#endif // PORTSELECT_H
