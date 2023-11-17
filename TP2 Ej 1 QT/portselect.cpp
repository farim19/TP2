#include "portselect.h"
#include "ui_portselect.h"

portselect::portselect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::portselect)
{
    ui->setupUi(this);
}

portselect::~portselect()
{
    delete ui;
}

void portselect::on_pb_aceptar_clicked()
{
    accept();
}
