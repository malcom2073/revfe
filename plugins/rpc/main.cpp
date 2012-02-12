#include <QtCore/QCoreApplication>
#include <QDebug>
#include "server.h"
#include "client.h"
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	Server s;
	s.start();
	Client c(QString("ClientOne"));
	Client c2(QString("ClientTwo"));
	Client c3(QString("ClientThree"));
	return a.exec();
}
