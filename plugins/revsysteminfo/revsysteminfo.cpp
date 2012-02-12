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

#include "revsysteminfo.h"



SystemInfo::SystemInfo()
{
	m_name = "RevSystemInfo";
}

void SystemInfo::init()
{
	systemInfo = new QSystemInfo(this);
	di = new QSystemDeviceInfo(this);
	ni = new QSystemNetworkInfo(this);
	sti = new QSystemStorageInfo(this);

	//QTimer *timer = new QTimer();
	//connect(timer,SIGNAL(timeout()),this,SLOT(updateSensors()));
	//timer->start(5000);
	sensorsTimer = new QTimer(this);

	connect(sensorsTimer,SIGNAL(timeout()),this,SLOT(updateSensors()));
}
QString SystemInfo::getName()
{
	return m_name;
}
void SystemInfo::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender)
	Q_UNUSED(message)
}
void SystemInfo::batteryLevelChanged(int level)
{
	//Level is 0-100
	passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_BATT" << QString::number(level)));
}
void SystemInfo::logicalDriveChanged(bool added,QString vol)
{
	if (added)
	{
		passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:system_newdrive" << vol));
	}
}
void SystemInfo::updateSensors()
{
  /*
	FILE *file = fopen("/etc/sensors.conf","r");
	sensors_init(file);
	const sensors_chip_name *chipname = 0;
	int i=0;

	while ((chipname = sensors_get_detected_chips(&i)))
	{
		const char *name = sensors_get_adapter_name(i);
		QString cname = QString(name);
		cname.detach();
		int sensornum = -1;
		for (int i=0;i<sensorsList.size();i++)
		{
			if (sensorsList[i].name == cname)
			{
				//This is our sensor;
				sensornum = i;
				break;
			}
		}
		if (sensornum == -1)
		{
			//No sensor yet. Add one;
			Sensor newsensor;
			sensorsList.append(newsensor);
			sensornum = sensorsList.size()-1;
			sensorsList[sensornum].name = cname;
		}

		int nr1=0;
		int nr2=0;
		const sensors_feature_data *data = 0;
		while ((data = sensors_get_all_features(*chipname,&nr1,&nr2)))
		{
			char *str;
			sensors_get_label(*chipname,data->number,&str);
			QString sname = QString(str);
			sname.detach();
			delete str;
			double result = 0;
			sensors_get_feature(*chipname,data->number,&result);
			if (sname.contains("alarm"))
			{
				sensorsList[sensornum].alarm = result;
			}
			else if (sname.contains("crit"))
			{
				sensorsList[sensornum].crit = result;
			}
			else
			{
				//Primary label.
				QString dname = QString(data->name);
				dname.detach();
				sensorsList[sensornum].label = sname;
				sensorsList[sensornum].temp = result;
			}
			sensorsList[sensornum].lastUpdate = QDateTime::currentDateTime();
		}
	}
	sensors_cleanup();
	fclose(file);

	qDebug() << "Temp Chip Update";
	for (int i=0;i<sensorsList.size();i++)
	{
		qDebug() << "Sensor:" << sensorsList[i].name;
		qDebug() << "Temp:" << sensorsList[i].temp;
		qDebug() << "Crit:" << sensorsList[i].crit;
		qDebug() << "Alarm:" << sensorsList[i].alarm;
		qDebug() << "Label:" << sensorsList[i].label;
	}
*/
}

void SystemInfo::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender)
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			//Connect signals here, so we're not trying to pass messages off of events before the system is ready.
			connect(di,SIGNAL(batteryLevelChanged(int)),this,SLOT(batteryLevelChanged(int)));
			connect(sti,SIGNAL(logicalDriveChanged(bool,QString)),this,SLOT(logicalDriveChanged(bool,QString)));
			passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
			int battery = di->batteryLevel();
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_BATT" << QString::number(battery)));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_VERSION_OS" << systemInfo->version(QSystemInfo::Os)));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_VERSION_QT" << systemInfo->version(QSystemInfo::QtCore)));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_VERSION_FIRMWARE" << systemInfo->version(QSystemInfo::Firmware)));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_MFG" << di->manufacturer()));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_MODEL" << di->model()));
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_PROD_NAME" << di->productName()));

			QStringList drives = sti->logicalDrives();
			QString msg = "";
			foreach (QString drive,drives)
			{
				msg.append(drive + " " + QString::number(sti->availableDiskSpace(drive) / 1000000.0) + "MB free  \n");
			}
			passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_DRIVE_FREE" << msg));
			//passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_WLAN_MAC" << ni->macAddress(QSystemNetworkInfo::WlanMode)));

			/*for (int i=0;i<ni->interfaceForMode(QSystemNetworkInfo::WlanMode).allInterfaces().size();i++)
			{
				qDebug() << ni->interfaceForMode(QSystemNetworkInfo::WlanMode).allInterfaces()[i].name() << ni->interfaceForMode(QSystemNetworkInfo::WlanMode).allInterfaces()[i].allAddresses()[0].toString();
			}*/

			//passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "SYSINFO_WLAN_IPV4" << ni->interfaceForMode(QSystemNetworkInfo::WlanMode).allAddresses()[2].toString()));




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
	else if (message.getClass() == "sensors")
	{
		if (message.getMethod() == "start")
		{
			if (message.getArgs().size() > 0)
			{
				int time = message.getArgs()[0].toInt();
				if (time == 0)
				{
					//This is an invalid time
				}
				sensorsTimer->start(time * 1000);
			}
		}
		else if (message.getMethod() == "stop")
		{
			sensorsTimer->stop();
		}
		else if (message.getMethod() == "update")
		{
			updateSensors();
		}
	}
}


Q_EXPORT_PLUGIN2(RevSystemInfoPlugin, SystemInfo)
