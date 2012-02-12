#include "rpc.h"

Rpc::Rpc(QObject *parent) : QObject(parent)
{
}
Rpc::Rpc(QString name,QObject *parent) : QObject(parent)
{
	m_name = name;
	socket = new QTcpSocket(this);
	connect(socket,SIGNAL(connected()),this,SLOT(socketConnected()));
	connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()));
	connect(socket,SIGNAL(readyRead()),this,SLOT(socketReadyRead()));
	connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
	socket->connectToHost("127.0.0.1",2074);
}

void Rpc::socketConnected()
{
	qDebug() << m_name << "Socket Connected";
	QByteArray packet;
	packet.append((char)0xAF);
	packet.append((char)0x00);
	packet.append((char)0x00);
	packet.append((char)0x00);
	packet.append((char)0x00);
	packet.append((char)0x00);
	packet.append((m_name.length()) & 0xFF);
	packet.append((m_name.length() >> 8) & 0xFF);
	packet.append((m_name.length() >> 16) & 0xFF);
	packet.append((m_name.length() >> 24) & 0xFF);
	packet.append(m_name);
	socket->write(packet);
	socket->flush();
}

void Rpc::socketDisconnected()
{
	qDebug() << m_name << "Socket Disconnected";
}

void Rpc::socketReadyRead()
{
	qDebug() << "Client ready read";
	m_buffer.append(socket->readAll());
	parseBuffer();
}
void Rpc::sendString(QString target, QString message)
{
	QByteArray packet;
	packet.append((char)0x01);
	packet.append((char)0x01);
	packet.append((target.length()) & 0xFF);
	packet.append((target.length() << 8) & 0xFF);
	packet.append((target.length() << 16) & 0xFF);
	packet.append((target.length() << 24) & 0xFF);
	packet.append((message.length()) & 0xFF);
	packet.append((message.length() >> 8) & 0xFF);
	packet.append((message.length() >> 16) & 0xFF);
	packet.append((message.length() >> 24) & 0xFF);
}

void Rpc::parseBuffer()
{
	if (m_buffer.length() >= 10)
	{
		int tlength=0;
		tlength += m_buffer.at(2);
		tlength += m_buffer.at(3) << 8;
		tlength += m_buffer.at(4) << 16;
		tlength += m_buffer.at(5) << 24;
		int mlength=0;
		mlength += m_buffer.at(6);
		mlength += m_buffer.at(7) << 8;
		mlength += m_buffer.at(8) << 16;
		mlength += m_buffer.at(9) << 24;
		if (m_buffer.length() +2 > tlength + mlength)
		{
			QByteArray packet = m_buffer.mid(0,tlength+mlength+2);
			QString target = packet.mid(10,tlength);
			QByteArray message = packet.mid(10+tlength,mlength);
			m_buffer.remove(0,tlength+mlength+2);
		if (packet[1] == (char)0x01)
		{
			//Plain string packet
			emit stringMessage(message);
		}
		else if (packet[1] == (char)0x02)
		{
			//
		}
		}
	}
	if (m_buffer.length() >= 10)
	{
		parseBuffer();
	}
}

void Rpc::socketError(QAbstractSocket::SocketError error)
{
	qDebug() << m_name << "Socket error:" << socket->errorString();
}
