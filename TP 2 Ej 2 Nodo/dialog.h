#ifndef DIALOG_H
#define DIALOG_H
#include <qmqtt.h>
#include <QTimer>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

using namespace QMQTT;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
   QMQTT ::  Client cliente;

    ~Dialog();

private slots:

   void connected();
   void disconnected();
   void error(const QMQTT ::  ClientError);
   void subscribed(const QString& topic, const quint8 qos);
   void unsubscribed(const QString& topic);
   void published(const QMQTT::Message& message, quint16 msgid=0);
   void pingresp();
   void received(const QMQTT::Message&);

   void on_dial_valueChanged(int value);

   void on_btnConnect_clicked();

    void on_btn_Desconectar_clicked();

    void publicarvalorDial(int valor);
    void on_timer_timeout();

private:
    Ui::Dialog *ui;
    QTimer *timer;
    int UltValorDial;

};
#endif // DIALOG_H
