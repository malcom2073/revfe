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
#include <sensors/sensors.h>

#include <QApplication>
#include <QTimer>
#include <QSystemInfo>
#include <QSystemDeviceInfo>
#include <QSystemNetworkInfo>
#include <QSystemStorageInfo>
#include <QDateTime>

#include "baseinterface.h"
#include "ipcmessage.h"

QTM_USE_NAMESPACE


class Sensor
{
	public:
	QString name;
	double temp;
	double crit;
	double alarm;
	QString label;
	QDateTime lastUpdate;
};



class SystemInfo : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)

public:
	SystemInfo();
	void init();
	QString getName();
private:
	QString m_name;
	QSystemInfo *systemInfo;
	QSystemDeviceInfo *di;
	QSystemNetworkInfo *ni;
	QSystemStorageInfo *sti;
	QList<Sensor> sensorsList;
	QTimer *sensorsTimer;
private slots:
	void batteryLevelChanged(int level);
	void logicalDriveChanged(bool added,QString vol);
	void updateSensors();
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
public slots:
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
};
