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

#include "revobd2.h"
//#include <QMessageBox>
#include <QMetaType>
#include <QFile>
#include <QDateTime>
#include <QXmlStreamReader>
//


OBD2::OBD2()
{
	m_name = "RevOBD2";
}

void OBD2::init()
{
	qmlRegisterType<GaugeItem>("GaugeItem",0,1,"GaugeItem");
	/*reqListValues.append(QPair<QString,QString>("obdinfo:rpm","010C"));
	reqListValues.append(QPair<QString,QString>("obdinfo:vehspeed","010D"));
	reqListValues.append(QPair<QString,QString>("obdinfo:watertemp","0105"));
	reqListValues.append(QPair<QString,QString>("obdinfo:load","0104"));
	reqListValues.append(QPair<QString,QString>("obdinfo:maf","0110"));
	*/

	/*QFile xmlfile("obd.xml");
	xmlfile.open(QIODevice::ReadOnly);
	QString xmlstring = xmlfile.readAll();
	xmlfile.close();

	QXmlStreamReader xml(xmlstring);
	while (!xml.atEnd())
	{
		if (xml.name() == "value" && xml.isStartElement())
		{
			ObdThread::RequestClass req;
			QString mode = xml.attributes().value("mode").toString();
			req.mode = ObdInfo::intFromHex(mode);
			QString pid = xml.attributes().value("pid").toString();
			req.pid = ObdInfo::intFromHex(pid);
			int priority = xml.attributes().value("priority").toString().toInt();
			int wait = xml.attributes().value("wait").toString().toInt();
			req.priority = priority;
			req.wait = wait;
			req.type = ObdThread::MODE_PID;
			req.repeat = true;
			requestList.append(req);
			qDebug() << "OBD Pid added to ObdMate:" << mode << req.mode << pid << req.pid;
		}
		xml.readNext();
	}
	*/
	responseTimer = new QTimer(this);
	connect(responseTimer,SIGNAL(timeout()),this,SLOT(responseTimerTick()));

	qRegisterMetaType<variantStruct>("variantStruct");
	obdThread = new ObdThread();

	connect(obdThread,SIGNAL(pidReceived(QString,QString,int,double)),this,SLOT(obdValueReceived(QString,QString,int,double)));
	connect(obdThread,SIGNAL(connected(QString)),this,SLOT(obdThreadConnected(QString)));
	connect(obdThread,SIGNAL(disconnected()),this,SLOT(obdThreadDisconnected()));

}
void OBD2::errorMessageFromThread(QString message)
{
	passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList(message)));
}
void OBD2::obdThreadConnected(QString version)
{
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_STATUS_TEXT" << "Connected"));
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_TOOL_VERSION" << version));

	qDebug() << "revobd2.cpp: OBD Tool connected:" << version;
}
void OBD2::obdThreadDisconnected()
{
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_STATUS_TEXT" << "Not Connected"));
	qDebug() << "ObdMate.cpp: OBD Tool Disconnected";
}

QString OBD2::getName()
{
	return m_name;
}
void OBD2::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}
void OBD2::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender);
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "2" << "OBD_STATUS_TEXT" << "" << "OBD_TOOL_VERSION" << ""));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_STATUS_TEXT" << "Not Connected"));
			bool opened = false;
			bool start = false;
			if (message.getArgs().count() > 0)
			{
				qDebug() << message.toString();
				for (int i=0;i<message.getArgs().count();i++)
				{
					QString key = message.getArgs()[i];
					QString value = message.getArgs()[i+1];
					QString type = message.getArgs()[i+2];
					QString id = message.getArgs()[i+3];
					qDebug() << key << value << type;
					i++;
					i++;
					i++;
					QStringList split = key.split('=');
					if (split[1] == "comport")
					{
						port = value.split("=")[1];
						obdThread->setPort(port);
						opened  = true;
					}
					if (split[1] == "baud")
					{
						obdThread->setBaud(value.split("=")[1].toInt());
						//obdThread->setBaud(split[1]);
					}
					if (split[1] == "start")
					{
						if (value.split("=")[1] == "true")
						{
							start = true;
							qDebug() << "Starting OBD Thread";
							emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_STATUS_TEXT" << "Starting OBD Thread"));
							obdThread->start();
							obdThread->connect();
						}
					}
					if (split[1] == "value")
					{
						//qDebug() << key << type << value << id;
						//QString pid = value.split("=")[1];
						ObdThread::RequestClass req;
						req.mode = ObdInfo::intFromHex(value.split("=")[1].mid(0,2));
						req.pid = ObdInfo::intFromHex(value.split("=")[1].mid(2,2));
						req.priority = 1;
						req.wait = 1;
						req.type = ObdThread::MODE_PID;
						req.repeat = true;
						requestList.append(req);
						qDebug() << "ObdMate.cpp: OBD Pid added to ObdMate:" << req.mode << req.pid;
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "OBD_" + value.split("=")[1] << "0"));

						//QString mode = xml.attributes().value("mode").toString();
						//req.mode = ObdInfo::intFromHex(mode);
						//QString pid = xml.attributes().value("pid").toString();
						//req.pid = ObdInfo::intFromHex(pid);
						//int priority = xml.attributes().value("priority").toString().toInt();
						//int wait = xml.attributes().value("wait").toString().toInt();
						//req.priority = priority;
						//req.wait = wait;
						//req.type = ObdThread::MODE_PID;
						//req.repeat = true;
						//requestList.append(req);
					//	qDebug() << "OBD Pid added to ObdMate:" << mode << req.mode << pid << req.pid;
					}
				}
				if ((start) && (opened))
				{
					for (int i=0;i<requestList.count();i++)
					{
						//obdThread->addRequest(reqListValues[i].second));
						obdThread->addRequest(requestList[i].mode,requestList[i].pid,requestList[i].priority,requestList[i].wait);
						//obdThread->addRequest(reqListValues[i].second);
					}
				}
			}
			passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
		else if (message.getMethod() == "throw")
		{
			
		}
	}
	else if (message.getClass() == "obd")
	{
		if (message.getMethod() == "add")
		{			
		}
		else if (message.getMethod() == "start")
		{
			qDebug() << "Starting OBD2 Request Loop";
			obdThread->start();
			obdThread->connect();
			for (int i=0;i<requestList.count();i++)
			{
				obdThread->addRequest(requestList[i].mode,requestList[i].pid,requestList[i].priority,requestList[i].wait);
			}
			//obdThread->start();
		}
		else if (message.getMethod() == "stop")
		{
			for (int i=0;i<requestList.count();i++)
			{
				obdThread->removeRequest(requestList[i].mode,requestList[i].pid,requestList[i].priority);
			}
			obdThread->disconnect();
		}
		if (message.getMethod() == "openport")
		{
			if (message.getArgs().count() > 0)
			{
				/*if (obdThread->openPort(message.getArgs()[0]) < 0)
				{
					QMessageBox::information(0,"Error","Error opening port");
					//Error here
				}*/
				/*if (obdThread->initPort(1,1) < 0)
				{
					//Error here
				}*/
			}
		}
	}
}
void OBD2::responseTimerTick()
{
	averageMsPerPing = 0;
	for (int i=0;i<responseTimes.count();i++)
	{
		averageMsPerPing += responseTimes[i];
	}
	if (responseTimes.count() > 0)
	{
		averageMsPerPing = (averageMsPerPing / responseTimes.count()) / 10000;
	}
}
void OBD2::obdValueReceived(QString pid,QString val,int set,double time)
{
	Q_UNUSED(time);
	//qDebug() << "Pid received" << pid << val;
	responseTimes.append(QDateTime::currentDateTime().toTime_t() - currentTicks);
	currentTicks = QDateTime::currentDateTime().toTime_t();
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << QString("OBD_") + pid << val));
	if (responseTimes.count() > 10)
	{
		responseTimes.removeAt(0);
	}
}


Q_EXPORT_PLUGIN2(RevOBD2Plugin, OBD2)
