#include "techdata.h"
#include "ui_techdata.h"

#include <QDebug>

Techdata::Techdata(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Techdata)
{
    ui->setupUi(this);
}

void Techdata::showBeacon(QString beacon)
{
    ui->beaconBox->setText(beacon);


}

void Techdata::showRx1Parms(QString parms)
{
    ui->rx1ParmsBox->setText(parms);

}

void Techdata::showRx2Parms(QString parms)
{
    ui->rx2ParmsBox->setText(parms);

}

Techdata::~Techdata()
{
    delete ui;
}
