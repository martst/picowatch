#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "videodialog.h"
#include "techdata.h"
//#include "myudp.h"
#include <QUdpSocket>

#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <map>

#include <arpa/inet.h> // htons, inet_addr
#include <netinet/in.h> // sockaddr_in
#include <sys/types.h> // uint16_t
#include <sys/socket.h> // socket, sendto
#include <unistd.h> // close


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SendUDP(QHostAddress, quint16, std::string);
    QTimer *poller;
    void initialiseRates();
    void wakeUpPico();
    void startRx1Video();
    void startRx2Video();
    void closeEvent(QCloseEvent *event);
    void rx1VideoDestroy();
    void rx2VideoDestroy();
    void techDataDestroy();


private slots:

    void lnbaoff(bool);
    void lnba12(bool);
    void lnba18(bool);

    void bcastReadyRead();
    void cmdReadyRead();
    void p9901ReadyRead();
    void p9902ReadyRead();
    void p9903ReadyRead();
    void p9904ReadyRead();

    void on_rx1SubBut_clicked();

    void on_rx2SubBut_clicked();

    void on_rx1LNB_clicked(bool checked);
    void on_rx2LNB_clicked(bool checked);

    void on_rx1Freq_textChanged(const QString &arg1);

    void on_rx2Freq_textChanged(const QString &arg1);

    void on_rx1Rate_currentIndexChanged(int index);

    void on_rx2Rate_currentIndexChanged(int index);

    void on_rx1Top_clicked(bool checked);

    void on_rx2Top_clicked(bool checked);
    void on_rx2Bot_clicked(bool checked);

    void on_pushButton_clicked();
    void updateVideoDetails();

    void on_actionExit_triggered();
    void on_actionDebug_triggered();




private:
    QUdpSocket _socket;
    QHostAddress destIP;
    Ui::MainWindow *ui;
    VideoDialog *vidDialog1;
    VideoDialog *vidDialog2;
    Techdata *techdata;

    QUdpSocket _cmdSocket;
    QUdpSocket _broadcastSocket;
    QUdpSocket _p9901Socket;
    QUdpSocket _p9902Socket;
    QUdpSocket _p9903Socket;
    QUdpSocket _p9904Socket;
};
#endif // MAINWINDOW_H
