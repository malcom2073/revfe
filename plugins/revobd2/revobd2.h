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

//#include <QApplication>
#include "gaugeitem.h"
#include "baseinterface.h"
#include "ipcmessage.h"
#include "ObdThread.h"
#include <QTimer>
#include <QList>
#include <QPair>
class OBD2 : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)

public:
	OBD2();
	void init();
	QString getName();
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);

private:
	QString m_name;
	ObdThread *obdThread;
	QList<uint> responseTimes;
	QList<QPair<QString,QString> > reqListValues;
	QList<ObdThread::RequestClass> requestList;
	uint averageMsPerPing;
	uint currentTicks;
	QTimer *responseTimer;
	QString port;
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
private slots:
	void obdValueReceived(QString pid,QString val,int set,double time);
	void responseTimerTick();
	void errorMessageFromThread(QString message);
	void obdThreadConnected(QString version);
	void obdThreadDisconnected();
};
