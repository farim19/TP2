#include "dialog.h"
#include "ui_dialog.h"
#include "portselect.h"
#include "ui_portselect.h"
#include <QListWidgetItem>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <maquinaestados.h>
#include <string>
#include <QDebug>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->hslider1->setRange(0,2047);
    ui->hslider2->setRange(0,2047);
    serie = new QSerialPort();
    serie->setBaudRate(QSerialPort::Baud115200);
    serie->setDataBits(QSerialPort::Data8);
    serie->setParity(QSerialPort::NoParity);
    serie->setStopBits(QSerialPort::OneStop);
    serie->setFlowControl(QSerialPort::NoFlowControl);
    connect(serie,
            SIGNAL(readyRead()),
            this,
            SLOT(conectar()));
}

Dialog::~Dialog()
{
    delete ui;
    if(serie->isOpen())
      serie->close();
    delete serie;
}
void Dialog::conectar(){
    static fcnState listStates[4] = {
        stateHeader,
        stateGetExit,
        stateGetSum,
        stateGetend,
        };

    static states vStates = {stHeader, 0};

    char newData;
    while(serie->bytesAvailable() > 0)
    {
       serie->read(static_cast<char *>(&newData), 1);
       qDebug()<<"se leyo: %c"<<newData;
       if(listStates[vStates.actState](&vStates, newData)){
             ui->listWidget->addItem("exito");
            qDebug()<<"se leyeron bien los datos";
       }
       }
    }
void Dialog::on_pb_conectar_clicked()
{
    portselect cfg(this);
    QList<QSerialPortInfo>::iterator it;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for(it = ports.begin(); it != ports.end(); it++)
      if((*it).portName() != "")
        cfg.ui->cmbPuertos->addItem((*it).portName());
    if(cfg.exec())
    {
        if(serie->isOpen())
            serie->close();
        serie->setPortName(cfg.ui->cmbPuertos->currentText());
        serie->open(QIODevice::ReadWrite);
    }
}


void Dialog::on_hslider1_valueChanged(int value)
{
    char head,valo1,valo2,valof1,valof2,suma,end;
    int sum;

    ui->lb_valor1->setText(QString::number(value));
    head=0x0B;
    valo1=(value& 0xFF);
    valo2=((value>>8)& 0xFF);
    valof1=(ui->hslider2->value() & 0xFF);
    valof2=((ui->hslider2->value()>>8) & 0xFF);
    sum=value+ui->hslider2->value();
    suma=sum%256;
    end=0xB0;
    if (serie->isOpen()){
     serie->write((const char *)&head, 1);
     serie->write((const char *)&valo1, 1);
     serie->write((const char *)&valo2, 1);
     serie->write((const char *)&valof1, 1);
     serie->write((const char *)&valof2, 1);
     serie->write((const char *)&suma, 1);
     serie->write((const char *)&end, 1);
    qDebug()<<"se enviaron datos";
    }else
      QMessageBox::warning(this,
                           "Dispositivo no conectado",
                           "Debe establecer conexión con el dispositivo antes de establecer el valor de milisegundos de encendido");
}

void Dialog::on_hslider2_valueChanged(int value)
{
    char head,valo1,valo2,valof1,valof2,suma,end;
    int sum;

    ui->lb_valor2->setText(QString::number(value));
    head=0x0B;
    valof1=(value& 0xFF);
    valof2=((value>>8)& 0xFF);
    valo1=(ui->hslider1->value() & 0xFF);
    valo2=((ui->hslider1->value()>>8) & 0xFF);
    sum=value+ui->hslider1->value();
    suma=sum%256;
    end=0xB0;
    if (serie->isOpen()){
     serie->write((const char *)&head, 1);
     serie->write((const char *)&valo1, 1);
     serie->write((const char *)&valo2, 1);
     serie->write((const char *)&valof1, 1);
     serie->write((const char *)&valof2, 1);
     serie->write((const char *)&suma, 1);
     serie->write((const char *)&end, 1);
    qDebug()<<"se enviaron datos";
    }else
      QMessageBox::warning(this,
                           "Dispositivo no conectado",
                           "Debe establecer conexión con el dispositivo antes de establecer el valor de milisegundos de encendido");

}


