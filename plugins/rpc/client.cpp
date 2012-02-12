#include "client.h"

Client::Client(QObject *parent) : QThread(parent)
{
}
Client::Client(QString name, QObject *parent) : QThread(parent)
{
	rpc = new Rpc(name);
}

void Client::run()
{

}
