#ifndef IICTHREAD_H
#define IICTHREAD_H

#include <QObject>
#include <QThread>

#include "myiic.h"
#include "bmp280.h"
#include "scl300.h"
#include "ads118.h"
class iicThread : public QThread
{
    Q_OBJECT
public:
    explicit iicThread(QObject *parent = nullptr);

    myIIc *myiic;

    MYBMP280     *whybmp280;

    ADS118       *myads115;
     scl300 *myscl300;
     int iicfd;
     bool inittimer;

     void run() override;
signals:

public slots:
};

#endif // IICTHREAD_H
