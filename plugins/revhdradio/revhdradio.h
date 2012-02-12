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

#include <QApplication>
#include <QTimer>
#include <QList>
#include <QString>
#include "baseinterface.h"
#include "ipcmessage.h"
#include "serialclass.h"
#include "hdradiocodes.h"
#include "hdradiomessage.h"
class HDRadio : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)

public:
	HDRadio();
	void init();
	QString getName();
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
private:
	QString m_name;
	SerialClass *serial;
	QTimer *serialTimer;
	QString currentFreq;
	QString hdPort;
	QByteArray recvArray;
	bool inMessage;
	bool isFirst;
	int messageCount;
	int currentCount;
	void parseMessage(QByteArray arr);
	bool nextOnSet;
	void sendMessage(QByteArray arr);
	void tune(QString freqStr);
	HdRadioCodes *hdCodes;
	int totalSubChannels;
	int currentSubChannel;
	QTimer *subChannelTimer;
	QList<QString> subChannelArtist;
	QList<QString> subChannelTitle;
	QList<QString> presetFreqList;
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
private slots:
	void subChannelTimerTimeout();
	void serialReadTimer();
};
