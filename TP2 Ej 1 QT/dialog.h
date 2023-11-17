#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QList>
#include <QListWidget>
#include <QDebug>
QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_hslider1_valueChanged(int value);

    void on_hslider2_valueChanged(int value);

    void on_pb_conectar_clicked();

public slots:
    void conectar();
private:
    Ui::Dialog *ui;
    QSerialPort *serie;
};


#endif // DIALOG_H
