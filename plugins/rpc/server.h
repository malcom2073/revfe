#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QThread
{
	Q_OBJECT
public:
	Server(QObject *parent = 0);
protected:
	void run();
private:
	QList<QTcpSocket*> m_socketList;
	QMap<QTcpSocket*,QByteArray> m_bufferMap;
	void parseBuffers();
	void fillBuffers();
	void parseBuffers(QTcpSocket *sender, QByteArray *buffer);
	QMap<QString,QTcpSocket*> m_clientMap;
	QTcpServer *server;
signals:
private slots:
	void serverNewConnection();
public slots:
	void socketReadyRead();
};

#endif // SERVER_H
