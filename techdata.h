#ifndef TECHDATA_H
#define TECHDATA_H

#include <QDialog>

namespace Ui {class Techdata;}

class Techdata : public QDialog
{
    Q_OBJECT

public:
    explicit Techdata(QWidget *parent = nullptr);
    ~Techdata();

    void showBeacon(QString);
    void showRx1Parms(QString);
    void showRx2Parms(QString);

private:
    Ui::Techdata *ui;
};

#endif // TECHDATA_H
