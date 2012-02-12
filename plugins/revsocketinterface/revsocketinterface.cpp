/***************************************************************************
*   Copyright (C) 2008 by Michael Carpenter (malcom2073)                  *
*   mcarpenter@interforcesystems.com                                      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "revsocketinterface.h"




SocketInterface::SocketInterface()
{
	m_name = "RevSocketInterface";
}

void SocketInterface::init()
{
}
QString SocketInterface::getName()
{
	return m_name;
}
void SocketInterface::passPluginMessage(QString sender,QString message)
{

}
void SocketInterface::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			tcpListener = new QTcpServer(this);
			connect(tcpListener,SIGNAL(newConnection()),this,SLOT(tcpListenerNewConnection()));
			tcpListener->listen(QHostAddress::LocalHost,2073);
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
void SocketInterface::tcpListenerNewConnection()
{
	QTcpSocket *sock = tcpListener->nextPendingConnection();
	connect(sock,SIGNAL(disconnected()),this,SLOT(socketDisconnected()));
	connect(sock,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(sockerError(QAbstractSocket::SocketError)));
	m_bufferMap[sock] = QString();
	m_socketList.append(sock);
}
void SocketInterface::socketDisconnected()
{
	QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
	if (!sock)
	{
		return;
	}
	m_socketList.removeAll(sock);
	m_bufferMap.remove(sock);
}

void SocketInterface::sockerError(QAbstractSocket::SocketError err)
{

}

void SocketInterface::socketReadyRead()
{
	QTcpSocket *sock = qobject_cast<QTcpSocket*>(sender());
	if (!sock)
	{
		return;
	}
	if (!m_bufferMap.contains(sock))
	{
		return;
	}
	m_bufferMap[sock].append(sock->readAll());
	parseBuffer(sock);
}
void SocketInterface::parseBuffer(QTcpSocket *sock)
{
	if (m_bufferMap[sock].contains("\n"))
	{
		QString line = m_bufferMap[sock].mid(0,m_bufferMap[sock].indexOf("\n"));
		m_bufferMap[sock] = m_bufferMap[sock].mid(m_bufferMap[sock].indexOf("\n")+1);
		passCoreMessage(m_name,IPCMessage(line));
	}
	if (m_bufferMap[sock].contains("\n"))
	{
		parseBuffer(sock);
	}
}

Q_EXPORT_PLUGIN2(RevSocketInterfacePlugin, SocketInterface)
