/* libVLC and Qt sample code
 * Copyright © 2009 Alexander Maringer <maringer@maringer-it.de>
 */
#include "videodialog.h"
#include "ui_videodialog.h"

#include <QTimer>

#include <QDebug>

VideoDialog::VideoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoDialog)
{
    ui->setupUi(this);

//    qDebug() << "Pop up setup";

    //preparation of the vlc command
    const char * const vlc_args[] = {
              "--verbose=2"};//be much more verbose then normal for debugging purpose

    _isPlaying = false;
    poller = new QTimer(this);

    //Initialize an instance of vlc

    //create a new libvlc instance
    _vlcinstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);  //tricky calculation of the char space used

    // Create a media player playing environement
    _mp = libvlc_media_player_new (_vlcinstance);

    //connect the slider and timer to the corresponding slots (uses Qt's signal / slots technology)

    connect(poller, SIGNAL(timeout()), this, SLOT(updateInterface()));
    connect(ui->_volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeVolume(int)));
    poller->start(1000); //start timer to trigger every 1000 ms the updateInterface slot
}

void VideoDialog::playFile(QString file)
{

     /* Create a new LibVLC media descriptor */
    _m = libvlc_media_new_location(_vlcinstance, file.toLatin1());
 //   _m = libvlc_media_new_location(_vlcinstance, "udp://@:9941");

    libvlc_media_player_set_media (_mp, _m);

    // /!\ Please note /!\
    //
    // passing the widget to the lib shows vlc at which position it should show up
    // vlc automatically resizes the video to the ´given size of the widget
    // and it even resizes it, if the size changes at the playing

    /* Get our media instance to use our window */

    /* again note X11 handle on Linux is needed
        winID() returns X11 handle when QX11EmbedContainer us used */

    int windid = ui->_videoWidget->winId();

//    qDebug() << "Window ID " << windid;

    libvlc_media_player_set_xwindow (_mp, windid );

    /* Play */
    libvlc_media_player_play (_mp);

    _isPlaying = true;
}

void VideoDialog::changeVolume(int newVolume)
{
    int volRes = libvlc_audio_set_volume (_mp, newVolume);
//    qDebug() << "In change volume " << newVolume << " Result = " << volRes;

}

void VideoDialog::updateInterface()
{
    if(!_isPlaying)
        return;

    // It's possible that the vlc doesn't play anything
    // so check before
    libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp);

    if (curMedia == NULL)
        return;

    int volume=libvlc_audio_get_volume (_mp); 
//    qDebug() << "In update volume " << volume;
    ui->_volumeSlider->setValue(volume);

}

void VideoDialog::updateDetails(std::string rx, std::string call, std::string freq, std::string mer, std::string der)
{
    ui->rxNum->setText(QString::fromStdString(rx));
    ui->callsign->setText(QString::fromStdString(call));
    ui->freq->setText(QString::fromStdString(freq));
    ui->dErr->setText(QString::fromStdString(der));
    ui->mer->setText(QString::fromStdString(mer));

    this->setWindowTitle(QString::fromStdString("Receiver " + rx));

}

VideoDialog::~VideoDialog()
{
    delete ui;

    /* Stop playing */
    libvlc_media_player_stop (_mp);

    /* Free the media_player */
    libvlc_media_player_release (_mp);

    libvlc_release (_vlcinstance);

}
void VideoDialog::closeEvent(QCloseEvent *event)
{
// Stop the stream if close X is clicked
    libvlc_media_player_stop (_mp);

}

