#include "dialog.h"
#include "ui_dialog.h"
#include <ctime>
#include <QStringList>


void Dialog::connected(){
  ui->list->addItem("Conectado");
cliente2.subscribe("/TP/id");
 cliente2.subscribe("/TP/data/sensor");
}
void Dialog::disconnected(){
  ui->list->addItem("Desconectado");
}
void Dialog::error(const QMQTT::ClientError error){
  ui->list->addItem("Error: " + QString::number(error));
}
void Dialog::subscribed(const QString& topic, const quint8 qos){
  ui->list->addItem("Subscripto:  "  + topic  + "( Qos: " +  QString::number(qos) + ")");
}
void Dialog::unsubscribed(const QString& topic){
 ui->list->addItem("Unsubscripto: " + topic);
}
void Dialog::published(const QMQTT::Message& message, quint16 msgid){

 ui->list->addItem("Publicado:" + message.topic() + ":" + message.payload()); /*+ "MsgId: " +QString::number(msgid));*/
}
void Dialog::pingresp(){

}
void Dialog::received(const Message &message){
  Message msg;
  QStringList datos;

  if(message.topic() == "/TP/data/sensor"){
      //ui->list->addItem(QString(message.payload()));
      QString dato= QString(message.payload());
      datos = dato.split("/",QString ::SkipEmptyParts);
     if(datos.count()==2){
      if(ui->comboBox->currentText()==datos[1]){
            actualizarGrafica(datos[0].toDouble());
   }
         }

   }
  if(message.topic()=="/TP/id"){
      ui->ListDispo->addItem( QString(message.payload()));
      ui->comboBox->addItem(QString(message.payload()));

  }
}

void Dialog :: actualizarGrafica(double val) {

    QCPGraph *graph = ui->QCustom_Graf->graph(0);

    graph->addData(tiempo, val); // Añadir punto al gráfico
    tiempo++;

    // Ajustar el rango del eje X para mostrar solo un intervalo de tiempo
    ui->QCustom_Graf->xAxis->setRange(tiempo - 1, tiempo);  // Mostrar el último segundo

    // acomodar el eje Y para adaptarse a los nuevos datos
    ui->QCustom_Graf->rescaleAxes();

     double Rangoinferior = tiempo - 1;
     int eliminarcont = 0;
     while (graph->data()->at(eliminarcont)->key < Rangoinferior) {
         eliminarcont++;
     }
     graph->data()->removeBefore(eliminarcont);

    ui->QCustom_Graf->replot(); // Redibujar el gráfico
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(&cliente2, SIGNAL(connected()), this, SLOT(connected()));
    connect(&cliente2, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(&cliente2, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));
    connect(&cliente2, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(&cliente2, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));

    connect(&cliente2, SIGNAL(published(const QMQTT::Message&, quint16 )), this, SLOT(published(const QMQTT::Message&, quint16 )));
    connect(&cliente2, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(&cliente2, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));


 ui->QCustom_Graf->addGraph();
 ui->QCustom_Graf->xAxis->setLabel("Tiempo");
 ui->QCustom_Graf->yAxis->setLabel("Valores del sensor");
 tiempo = 0;
ui->QCustom_Graf->xAxis->setRange(0,1000);
ui->QCustom_Graf->yAxis->setRange(0,100);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_btn_connect_clicked()
{
    ui->list->clear();
    cliente2.setHostName(ui->le_servidor->text());
    if((ui->le_servidor->text().length() > 0) && (ui->le_clave->text().length() > 0)){
        cliente2.setUsername(ui->le_servidor->text());
        cliente2.setPassword(ui->le_clave->text().toLatin1());
      }
    cliente2.setPort(ui->le_puerto->text().toInt());
    cliente2.connectToHost();
}

void Dialog::on_btn_desconectar_clicked()
{
    if(cliente2.isConnectedToHost()){
       cliente2.disconnectFromHost();
      }
    ui->list->clear();
    ui->ListDispo->clear();
    ui->QCustom_Graf->removeGraph(0);
    ui->QCustom_Graf->addGraph();
    ui->QCustom_Graf->replot();
}


void Dialog::on_btn_led_on_clicked()
{

QMQTT ::Message ledon;
QList<QListWidgetItem*> selected;
QList<QListWidgetItem*> :: iterator item;
selected = ui->ListDispo->selectedItems();
for(item = selected.begin(); item != selected.end(); item++){
    ledon.setTopic("/TP/"+(*item)->text()+"/cmd");
    ledon.setPayload("LedOn");
    cliente2.publish(ledon);
}

}

void Dialog::on_btn_GETID_clicked()
{
    QMQTT ::Message mensaje;
    mensaje.setTopic("/TP/cmd");
    mensaje.setPayload("getId: ");
    cliente2.publish(mensaje);

    ui->comboBox->clear();
    ui->list->clear();
    ui->ListDispo->clear();
    ui->QCustom_Graf->removeGraph(0);
    ui->QCustom_Graf->addGraph();
    ui->QCustom_Graf->replot();

}

void Dialog::on_btn_led_Off_clicked()
{
    QMQTT ::Message leoff; 
    QList<QListWidgetItem*> selected;
    QList<QListWidgetItem*> :: iterator item;
    selected = ui->ListDispo->selectedItems();
    for(item = selected.begin(); item != selected.end(); item++){
        leoff.setTopic("/TP/"+(*item)->text()+"/cmd");
        leoff.setPayload("LedOff");
        cliente2.publish(leoff);
}
}

void Dialog::on_comboBox_currentIndexChanged(const QString &arg1)
{
    ui->QCustom_Graf->removeGraph(0);
    ui->QCustom_Graf->addGraph();
    ui->QCustom_Graf->replot();
    ui->list->clear();
}

void Dialog::on_pushButton_clicked()
{
    QApplication ::exit();
}
