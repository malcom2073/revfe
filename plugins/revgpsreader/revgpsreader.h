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
#include <QProcess>
#include <QDir>
#include <QMap>
#include <QList>
#include "baseinterface.h"
#include "ipcmessage.h"
#include "serialclass.h"
class GPSReader : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)
public:
	GPSReader();
	void init();
	QString getName();
	~GPSReader();
private:
	QString m_name;

	QString currentGGATime;
	QString currentRMCTime;
	SerialClass *serialClass;
	QTimer *serialTimer;
	QString m_port;
	int m_fixType;
	int badCounter;
	void gpsPortFound(QString portname);
	void location(double lon, double lat, long time,int fix,int sats, double altitude);
	void gotFix(int fixType);
	void lostFix();
	void velocity(long time,double heading, double speed);
	void satStatus(int prn, int elevation, int azimuth, int snr);
	void findGpsPort();
	QMap<int,QList<QString> > m_satPropertyMap;
	bool m_logging;
	QFile *m_logFile;
public slots:
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
	
private slots:
	void serialTimerTimeout();
	void openPort();
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
};
