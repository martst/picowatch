#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <vlc/vlc.h>
#include <QDialog>
#include <QCloseEvent>

namespace Ui {class VideoDialog;}

#include <QWidget>

class QTimer;

#define POSITION_RESOLUTION 10000

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoDialog(QWidget *parent = nullptr);
    ~VideoDialog();
    QTimer *poller;
    bool _isPlaying;
    libvlc_instance_t *_vlcinstance;
    libvlc_media_player_t *_mp;
    libvlc_media_t *_m;
    void updateDetails(std::string, std::string, std::string, std::string, std::string);
    void closeEvent(QCloseEvent *event);

public slots:
    void playFile(QString file);
    void updateInterface();
    void changeVolume(int newVolume);

private:
    Ui::VideoDialog *ui;
};

#endif // VIDEODIALOG_H
