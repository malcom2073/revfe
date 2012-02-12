#include "server.h"
#include <QDebug>
#include <QCoreApplication>
Server::Server(QObject *parent) : QThread(parent)
{
}

void Server::run()
{
	server = new QTcpServer();
	connect(server,SIGNAL(newConnection()),this,SLOT(serverNewConnection()));
	server->listen(QHostAddress("127.0.0.1"),2074);
	exec();
	while (true)
	{
		QCoreApplication::processEvents();
		if (server->hasPendingConnections())
		{
		}
		//fillBuffers();
		//parseBuffers();
		this->msleep(500);
	}
}
void Server::serverNewConnection()
{
	QTcpSocket *socket = server->nextPendingConnection();
	if (!connect(socket,SIGNAL(readyRead()),this,SLOT(socketReadyRead())))
	{
		qDebug() << "Unable to connect socket readyRead()";
	}
	m_bufferMap[socket] = QByteArray();
	m_socketList.append(socket);
	qDebug() << "Server: Socket connected" << socket << socket->bytesAvailable();

}

void Server::socketReadyRead()
{
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
	{
		qDebug() << "Error, non QTcpSocket* called socketReadyRead!!";
		return;
	}
	qDebug() << "Incoming on socket" << socket;
	m_bufferMap[socket].append(socket->readAll());
	parseBuffers();
	//m_bufferMap[m_socketList[i]].append(m_socketList[i]->readAll());
}

void Server::fillBuffers()
{
	for (int i=0;i<m_socketList.size();i++)
	{

	}
}
void Server::parseBuffers(QTcpSocket *sender, QByteArray *buffer)
{
	qDebug() << "Parsing Buffer for" << sender;
	if (buffer->length() >= 10)
	{
		int tlength=0;
		tlength += buffer->at(2);
		tlength += buffer->at(3) << 8;
		tlength += buffer->at(4) << 16;
		tlength += buffer->at(5) << 24;
		int mlength=0;
		mlength += buffer->at(6);
		mlength += buffer->at(7) << 8;
		mlength += buffer->at(8) << 16;
		mlength += buffer->at(9) << 24;
		if (buffer->length() +10 > tlength + mlength)
		{
			QByteArray packet = buffer->mid(0,tlength+mlength+10);
			QString target = packet.mid(10,tlength);
			QByteArray message = packet.mid(10+tlength,mlength);
			buffer->remove(0,tlength+mlength+10);
			//Parse Packet here. Should be [Typex2][Lengthx4][PacketxN]
			if (packet[0] == (char)0xAF)
			{
				//Registration Packet. Message will be the string name of the client.
				qDebug() << "Server. Client registered:" << message;
				m_clientMap[message] = sender;

			}
			else if (packet[0] == (char)0x01)
			{
				//Packet is a unicast packet
				if (m_clientMap.contains(target))
				{
					m_clientMap[target]->write(packet);
				}
				else
				{
					qDebug() << "INVALID target for message";
				}
			}
		}
	}
	if (buffer->length() >= 10)
	{
		parseBuffers(sender,buffer);
	}

}

void Server::parseBuffers()
{
	for (int i=0;i<m_socketList.size();i++)
	{
		parseBuffers(m_socketList[i],&m_bufferMap[m_socketList[i]]);
	}
}
