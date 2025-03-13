/*
 * Copyright (c) 2025 Martin Stubbs <martinstubbs@gmail.com>
 *
 * Usbfttx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Usbfmtx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimer>

std::string destIPs;
std::map<std::string, std::string> parmsRx1;
std::map<std::string, std::string> parmsRx2;

int ipSet;
int ipKnown = 0;
int rx1Set = 0;
int rx2Set = 0;
int rx1Offset = 0;
int rx2Offset = 0;
std::string rx1Freq;
std::string rx2Freq;
std::string rx1Rate;
std::string rx2Rate;
std::string rx1Ant;
std::string rx2Ant;
bool rx1WinOpen;
bool rx2WinOpen;
bool techDataOpen;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    poller = new QTimer(this);
//start timer to trigger every 1000 ms the updateInterface slot
    connect(poller, SIGNAL(timeout()), this, SLOT(updateVideoDetails()));
    poller->start(1000);

    connect(ui->lnb_a_off, SIGNAL(toggled(bool)), this, SLOT(lnbaoff(bool)));
    connect(ui->lnb_a_12, SIGNAL(toggled(bool)), this, SLOT(lnba12(bool)));
    connect(ui->lnb_a_18, SIGNAL(toggled(bool)), this, SLOT(lnba18(bool)));
    connect(ui->rx1SubBut, SIGNAL(toggled(bool)), this, SLOT(on_rx1SubBut_clicked()));
    connect(ui->rx2SubBut, SIGNAL(toggled(bool)), this, SLOT(on_rx2SubBut_clicked()));
    connect(ui->rx1LNB, SIGNAL(toggled(bool)), this, SLOT(on_rx1LNB_clicked(bool)));
    connect(ui->rx2LNB, SIGNAL(toggled(bool)), this, SLOT(on_rx2LNB_clicked(bool)));
    connect(ui->rx1Top, SIGNAL(toggled(bool)), this, SLOT(on_rx1Top_clicked(bool)));
    connect(ui->rx2Top, SIGNAL(toggled(bool)), this, SLOT(on_rx2Top_clicked(bool)));

// Start to receive Broadcast
    _broadcastSocket.bind(QHostAddress::AnyIPv4, 9997);
    connect(&_broadcastSocket, SIGNAL(readyRead()), this, SLOT(bcastReadyRead()));

    _cmdSocket.bind(QHostAddress::AnyIPv4, 9920);
    connect(&_cmdSocket, SIGNAL(readyRead()), this, SLOT(cmdReadyRead()));

    _p9901Socket.bind(QHostAddress::AnyIPv4, 9901);
    connect(&_p9901Socket, SIGNAL(readyRead()), this, SLOT(p9901ReadyRead()));

    _p9902Socket.bind(QHostAddress::AnyIPv4, 9902);
    connect(&_p9902Socket, SIGNAL(readyRead()), this, SLOT(p9902ReadyRead()));

    _p9903Socket.bind(QHostAddress::AnyIPv4, 9903);
    connect(&_p9903Socket, SIGNAL(readyRead()), this, SLOT(p9903ReadyRead()));

    _p9904Socket.bind(QHostAddress::AnyIPv4, 9904);
    connect(&_p9904Socket, SIGNAL(readyRead()), this, SLOT(p9904ReadyRead()));


    initialiseRates();
}

void MainWindow::SendUDP(QHostAddress destIP, quint16 port,  std::string msg)
{
    // convert string to QByteArray
    QByteArray Data(msg.c_str(), msg.length());

    // Sends the datagram datagram
    // to the host address and at port.
    // qint64 QUdpSocket::writeDatagram(const QByteArray & datagram,
    //                      const QHostAddress & host, quint16 port)
    _socket.writeDatagram(Data, destIP, port);
}

void MainWindow::wakeUpPico()
{
// Need to send a frequency message to associate Pico with sending IP and start streaming
    std::string msg = "[to@wh] rcv=1 freq=10491500 offset=9750000 srate=1500 fplug=A";
    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);

    msg = "[to@wh] rcv=2 freq=10491500 offset=9750000 srate=1500 fplug=A";
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent to wake up Pico");

}

void MainWindow::initialiseRates()
{
    QStringList list=(QStringList()<<"333"<<"500"<<"1000"<<"1500"<<"4000");
    ui->rx1Rate->addItems(list);
    ui->rx2Rate->addItems(list);

}

void MainWindow::lnbaoff(bool checked)
{
 //   std::string msg = "[to@wh] rcv=1 freq=10491500 offset=9750000 srate=1500 fplug=A vgx=off";
    std::string msg = "[to@wh] vgx=off";

    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent LNA off");
}

void MainWindow::lnba12(bool checked)
{
    std::string msg = "[to@wh] vgx=lo";

    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent LNA 12v");
}

void MainWindow::lnba18(bool checked)
{
    std::string msg = "[to@wh] vgx=hi";

    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent LNA 18v");
}


void MainWindow::cmdReadyRead()
{
    // when data comes in
    QByteArray buffer;
    buffer.resize(_cmdSocket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    // qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize,
    //                 QHostAddress * address = 0, quint16 * port = 0)
    // Receives a datagram no larger than maxSize bytes and stores it in data.
    // The sender's host address and port is stored in *address and *port
    // (unless the pointers are 0).

    _cmdSocket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

 //   qDebug() << "Message from: " << sender.toString();
 //   qDebug() << "Message port: " << senderPort;
 //   qDebug() << "Message: " << buffer;
    ui->cmdListBox->addItem(buffer);
}

void MainWindow::bcastReadyRead()
{
    // when data comes in
    QByteArray buffer;
    buffer.resize(_broadcastSocket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    // qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize,
    //                 QHostAddress * address = 0, quint16 * port = 0)
    // Receives a datagram no larger than maxSize bytes and stores it in data.
    // The sender's host address and port is stored in *address and *port
    // (unless the pointers are 0).

    _broadcastSocket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

//    qDebug() << "Message from: " << sender.toString();
//    qDebug() << "Message port: " << senderPort;
//    qDebug() << "Message: " << buffer;
//    ui->bcastBox->setText(buffer);
    if (techDataOpen)
    {
        techdata->showBeacon(buffer);
    }

    if (ipKnown == 0) {   
        QString destIPq = sender.toString();
        destIPs = destIPq.toStdString();
        ui->bcastIPAddr->setText(sender.toString());

        std::string lnb ("LNB supply X   ");
        std::size_t found = buffer.toStdString().find(lnb);
        // position after LNB title
        found = found + 15;
        std::string lStat = buffer.toStdString().substr(found,2);
        ui->cmdListBox->addItem(QString::fromStdString(lStat));

        if (lStat == "HI"){
            ui->lnb_a_18->setChecked(true);
        } else if (lStat == "LO"){
            ui->lnb_a_12->setChecked(true);
        } else {
            ui->lnb_a_off->setChecked(true);
        }
// send message to wake up the tuner after IP determined
        wakeUpPico();
         ipKnown = 1;
    }

}

void MainWindow::p9901ReadyRead()
{
    QByteArray buffer;
    buffer.resize(_p9901Socket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    _p9901Socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    std::string p9901;
    p9901 = buffer.toStdString();

    std::stringstream ss(p9901);

    std::string sLine;
    std::string sIndex;
    std::string sValue;

    int rx = 0;

    if (p9901.length() != 0)
    {
        while(std::getline(ss,sLine,'\n')){
             std::stringstream sBit(sLine);
            // Dummy getline to get rid of leading $
            std::getline(sBit, sIndex,'$');
            std::getline(sBit, sIndex,',');
            std::getline(sBit, sValue,'\n');
// Work out which receive data it is
            if (sIndex == "0") {
                rx = std::stoi(sValue);
//                qDebug() << "RX value " << rx;
            }
            if (rx == 1) {
// Trim string before storing
                parmsRx1[sIndex] = sValue.erase(sValue.find_last_not_of(" \n\r\t")+1);
            } else if (rx == 2) {
                parmsRx2[sIndex] = sValue.erase(sValue.find_last_not_of(" \n\r\t")+1);
            }
//            ui->cmdListBox->addItem(QString::fromStdString(sIndex + " = " + sValue));
        }
    }

    if (rx == 1) {

        if (techDataOpen)
        {
            techdata->showRx1Parms(buffer);
        }
// First time through set the screen vales to match
        if (rx1Set == 0) {
            ui->rx1Freq->setText(QString::fromStdString(parmsRx1["6"]));

            ui->rx1Rate->setCurrentText(QString::fromStdString(parmsRx1["9"]));
// Check first character because extra chars at end of string
            if (parmsRx1["89"].front() == '0') {
                ui->rx1LNB->setChecked(false);
                rx1Offset = 0;
            } else {
                ui->rx1LNB->setChecked(true);
                rx1Offset = 9750000;
            }
            if (parmsRx1["33"].front() == 'T')
            {
                ui->rx1Top->setChecked(true);
                rx1Ant = "A";
            } else {
                ui->rx1Bot->setChecked(true);
                rx1Ant = "B";
            }
            rx1Set = 1;
        }
    }

    if (rx == 2) {
//        ui->rx2ParmsBox->setText(buffer);
        if (techDataOpen)
        {
            techdata->showRx2Parms(buffer);
        }

        if (rx2Set == 0) {
            ui->rx2Freq->setText(QString::fromStdString(parmsRx2["6"]));
            ui->rx2Rate->setCurrentText(QString::fromStdString(parmsRx2["9"]));
            if (parmsRx2["89"].front() == '0') {
                ui->rx2LNB->setChecked(false);
                rx2Offset = 0;
            } else {
                ui->rx2LNB->setChecked(true);
                rx2Offset = 9750000;
            }
            if (parmsRx2["33"].front() == 'T')
            {
                ui->rx2Top->setChecked(true);
                rx2Ant = "A";
            } else {
                ui->rx2Bot->setChecked(true);
                rx2Ant = "B";
            }
            rx2Set = 1;
        }
    }
}

void MainWindow::p9902ReadyRead()
{
    QByteArray buffer;
    buffer.resize(_p9902Socket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    _p9902Socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    ui->p9902Box->setText(buffer);
}

void MainWindow::p9903ReadyRead()
{
   return;
/*

    QByteArray buffer;
    buffer.resize(_p9903Socket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    _p9903Socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    ui->p9903Box->setText(buffer);

    std::string p9903;
    p9903 = buffer.toStdString();

    std::stringstream ss(p9903);

    std::string sLine;
    std::string sIndex;
    std::string sValue;

    if (p9903.length() != 0)
    {
        while(std::getline(ss,sLine,'\n')){
 //           ui->cmdListBox->addItem(QString::fromStdString(sLine));
            std::stringstream sBit(sLine);
            // Dummy getline to get rid of leading $
            std::getline(sBit, sIndex,'$');
            std::getline(sBit, sIndex,',');
            std::getline(sBit, sValue,'\n');

            parmsRx2[sIndex] = sValue;
        }
    }

    ui->callRx2->setText(QString::fromStdString(parmsRx2["13"]));

    if (rx2Set == 0) {
        ui->rx2Freq->setText(QString::fromStdString(parmsRx2["6"]));
        if (parmsRx2["86"] == "0") {
            ui->rx2LNB->setChecked(false);
            rx2Offset = 0;
        } else {
            ui->rx2LNB->setChecked(true);
            rx2Offset = 9750000;
        }

        rx2Set = 1;
    }

    */
}

void MainWindow::p9904ReadyRead()
{
/*
    QByteArray buffer;
    buffer.resize(_p9904Socket.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    _p9904Socket.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    ui->p9904Box->setText(buffer);
*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_rx1SubBut_clicked()
{
//   std::string msg = "[to@wh] rcv=1 freq=10491500 offset=9750000 srate=1500 fplug=A vgx=off";
    std::string msg = "[to@wh] rcv=1 freq=" + rx1Freq + " offset=" + std::to_string(rx1Offset) +
            " srate=" + rx1Rate + " fplug=" + rx1Ant;

    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent to rx1");
    ui->cmdListBox->addItem(QString::fromStdString(msg));

// Reset stream
    startRx1Video();

}

void MainWindow::on_rx2SubBut_clicked()
{
    std::string msg = "[to@wh] rcv=2 freq=" + rx2Freq + " offset=" + std::to_string(rx2Offset) +
            " srate=" + rx2Rate + " fplug=" + rx2Ant;

    QHostAddress addrq;
    addrq.setAddress(QString::fromStdString(destIPs));
    SendUDP(addrq, 9920,  msg);
    ui->cmdListBox->addItem("Message sent to rx2");
    ui->cmdListBox->addItem(QString::fromStdString(msg));

// Reset stream
    startRx2Video();
}

void MainWindow::on_rx1LNB_clicked(bool checked)
{
    if (checked)
    {
        rx1Offset = 9750000;
    } else {
        rx1Offset = 0;
    }
}

void MainWindow::on_rx2LNB_clicked(bool checked)
{
    if (checked)
    {
        rx2Offset = 9750000;
    } else {
        rx2Offset = 0;
    }
}

void MainWindow::on_rx1Freq_textChanged(const QString &arg1)
{
    rx1Freq = ui->rx1Freq->text().toStdString();
// remove decimal point
    rx1Freq.erase(std::remove(rx1Freq.begin(), rx1Freq.end(), '.'), rx1Freq.end());
}

void MainWindow::on_rx2Freq_textChanged(const QString &arg1)
{
    rx2Freq = ui->rx2Freq->text().toStdString();
// remove decimal point
    rx2Freq.erase(std::remove(rx2Freq.begin(), rx2Freq.end(), '.'), rx2Freq.end());
}

void MainWindow::on_rx1Rate_currentIndexChanged(int index)
{
    rx1Rate = ui->rx1Rate->currentText().toStdString();
}

void MainWindow::on_rx2Rate_currentIndexChanged(int index)
{
    rx2Rate = ui->rx2Rate->currentText().toStdString();
}

void MainWindow::on_rx1Top_clicked(bool checked)
{
    if (checked) {
        rx1Ant = "A";
    } else {
        rx1Ant = "B";
    }
}

void MainWindow::on_rx2Top_clicked(bool checked)
{
    if (checked) {
        rx2Ant = "A";
    } else {
        rx2Ant = "B";
    }
}

void MainWindow::on_rx2Bot_clicked(bool checked)
{

}

void MainWindow::on_pushButton_clicked()
{
// use this as parent to have a popup. Null to have a window
    vidDialog1 = new VideoDialog();
    vidDialog1->show();
    rx1WinOpen = true;

    vidDialog2 = new VideoDialog();
    vidDialog2->show();
    rx2WinOpen = true;

    startRx1Video();
    startRx2Video();
}

void MainWindow::startRx1Video()
{
    if (rx1WinOpen)
    {
        QString pFile = "udp://@:9941";
        vidDialog1->playFile(pFile);
    }
}

void MainWindow::startRx2Video()
{
    if (rx2WinOpen)
    {
        QString pFile = "udp://@:9942";
        vidDialog2->playFile(pFile);
    }
}


void MainWindow::updateVideoDetails()
{
//    qDebug() << "In update video details ";
    if (rx1WinOpen)
    {
//        qDebug() << "In update video details after if 1 ";
        vidDialog1->updateDetails("rx " + parmsRx1["0"], parmsRx1["13"], rx1Freq, parmsRx1["30"], parmsRx1["12"]);
    }

    if (rx2WinOpen)
    {
//        qDebug() << "In update video details after if 2";
        vidDialog2->updateDetails("rx " + parmsRx2["0"], parmsRx2["13"], rx2Freq, parmsRx2["30"], parmsRx2["12"]);
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::closeAllWindows();
    QApplication::quit();
}

void MainWindow::on_actionDebug_triggered()
{
// show the debug/tech data window
    techdata = new Techdata();
    techdata->setModal(false);
    techdata->show();

    techDataOpen = true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    QApplication::closeAllWindows();
    QApplication::quit();
}
