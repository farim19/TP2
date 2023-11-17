#include "dialog.h"
#include "ui_dialog.h"
#include "QPixmap"

void Dialog::connected(){
  ui->list->addItem("Conectado");
   cliente.subscribe("/TP/cmd");
   cliente.subscribe("/TP/"+ui->le_ID->text()+"/cmd");
}
void Dialog::disconnected(){
  ui->list->addItem("Desconectado");
}
void Dialog::error(const QMQTT::ClientError error){
  ui->list->addItem("Error: " + QString::number(error));
}
void Dialog::subscribed(const QString& topic, const quint8 qos){
  ui->list->addItem("Subscripto: " + topic + "( Qos: " + QString::number(qos) + ")");
}
void Dialog::unsubscribed(const QString& topic){
 ui->list->addItem("Unsubscripto: " + topic);
}
void Dialog::published(const QMQTT::Message& message, quint16 msgid){
   QString mensajito= QString(message.payload());
    int cortar= mensajito.indexOf("/");
    QString mensaje=mensajito.left(cortar);
 ui->list->addItem("Publicado:" + message.topic() + ":" + mensaje); /*+ "MsgId: " +QString::number(msgid));*/
}
void Dialog::pingresp(){

}
void Dialog::received(const Message &message){
  Message msg;
  if(message.topic() == "/TP/cmd"){
      QMQTT :: Message mensID;
      mensID.setTopic( "/TP/id");
      mensID.setPayload(ui->le_ID->text().toLatin1());
      cliente.publish(mensID);
   }
  if(message.topic()=="/TP/"+ui->le_ID->text()+"/cmd"){
      if(QString(message.payload()) == "LedOn" ){
        ui->la_led->setPixmap(QPixmap(":/new/prefix1/ledOn.png"));
      }else{
         ui->la_led->setPixmap(QPixmap(":/new/prefix1/ledOff.png"));
  }}

}
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->dial->setRange(0,100);
    connect(&cliente, SIGNAL(connected()), this, SLOT(connected()));
    connect(&cliente, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(&cliente, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));
    connect(&cliente, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(&cliente, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));

    connect(&cliente, SIGNAL(published(const QMQTT::Message&, quint16 )), this, SLOT(published(const QMQTT::Message&, quint16 )));
    connect(&cliente, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(&cliente, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer_timeout()));
    timer->start(1000);
    UltValorDial=0;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_dial_valueChanged(int value)
{
   UltValorDial = value;
 publicarvalorDial(value);
}

void Dialog::on_timer_timeout()
{
    publicarvalorDial(UltValorDial);
}
void Dialog :: publicarvalorDial(int valor){
if(cliente.isConnectedToHost()){
    QMQTT::Message msg;
     msg.setTopic("/TP/data/sensor");
     msg.setPayload(QString :: number(valor).toLatin1()+"/"+ui->le_ID->text().toLatin1());
     cliente.publish(msg);
}
}


void Dialog::on_btn_Desconectar_clicked()
{
    if(cliente.isConnectedToHost()){
       cliente.disconnectFromHost();
      }
    ui->list->clear();
}

void Dialog::on_btnConnect_clicked()
{
        ui->list->clear();
    cliente.setHostName(ui->le_serv->text());
    if((ui->le_serv->text().length() > 0) && (ui->le_clave->text().length() > 0)){
        cliente.setUsername(ui->le_serv->text());
        cliente.setPassword(ui->le_clave->text().toLatin1());
      }
    cliente.setPort(ui->le_puerto->text().toInt());
    cliente.connectToHost();
}
