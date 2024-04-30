#ifndef ANATHREAD_H
#define ANATHREAD_H

#include <QObject>
#include <QThread>

class AnaThread : public QThread
{
public:
     explicit AnaThread(QObject *parent = nullptr);
    //AnaThread();
};

#endif // ANATHREAD_H
