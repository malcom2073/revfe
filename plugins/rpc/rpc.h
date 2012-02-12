#ifndef RPC_H
#define RPC_H

#include <QObject>
#include <QTcpSocket>
class Rpc : public QObject
{
	Q_OBJECT
public:
	Rpc(QObject *parent = 0);
	Rpc(QString name,QObject *parent=0);
private:
	QString m_name;
	QTcpSocket *socket;
	QByteArray m_buffer;
	void parseBuffer();
signals:
	void stringMessage(QString message);
public slots:
	void sendString(QString target,QString message);
private slots:
	void socketConnected();
	void socketDisconnected();
	void socketReadyRead();
	void socketError(QAbstractSocket::SocketError error);
};

#endif // RPC_H
