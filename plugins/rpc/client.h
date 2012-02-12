#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>
#include "rpc.h"
class Client : public QThread
{
	Q_OBJECT
public:
	Client(QObject *parent = 0);
	Client(QString name,QObject *parent=0);
private:
	Rpc *rpc;
protected:
	void run();
signals:

public slots:

};

#endif // CLIENT_H
