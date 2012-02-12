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

#include "revgpsreader.h"

#include <QDebug>
#include <QDateTime>
//#include <QMessageBox>



GPSReader::GPSReader()
{
	m_name = "RevGPSReader";
}

void GPSReader::init()
{
	currentGGATime = "";
	currentRMCTime = "";
	serialClass = new SerialClass();
	serialTimer = new QTimer(this);
	m_logging = false;
	m_logFile = 0;
	connect(serialTimer,SIGNAL(timeout()),this,SLOT(serialTimerTimeout()));
	/*
	if (!QFile::exists("gps.conf"))
	{
		passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList("No gps.conf file, not running...")));
		return;
	}
	QFile gpsSettings("gps.conf");
	gpsSettings.open(QIODevice::ReadWrite);
	QString gpsSettingsStr(gpsSettings.readAll());
	gpsSettings.close();
	if (gpsSettingsStr.isEmpty())
	{
		passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList("Unable to read gpssettings, not running...")));
		return;
	}
	
*/

}
GPSReader::~GPSReader()
{
	serialClass->closePort();
}
QString GPSReader::getName()
{
	return m_name;
}
void GPSReader::passPluginMessage(QString sender,QString message)
{

}
void GPSReader::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "throw")
		{

		}
		else if (message.getMethod() == "initstarted")
		{
			bool opened = false;
			QString port = "";
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

						m_port = value.split("=")[1];
						//serialClass->openPort(split[1]);
						QTimer::singleShot(4000,this,SLOT(openPort()));
						opened  = true;
					}
					else if (split[1] == "logging")
					{
						if (value.split("=")[1] == "true")
						{
							m_logging = true;
							m_logFile = new QFile(QDateTime::currentDateTime().toString("'gps.'MMddyy'.'hhmmss'.log'"));
							if (!m_logFile->open(QIODevice::ReadWrite))
							{
								m_logging = false;
								qDebug() << "GPSReader.cpp: Error opening GPS Log file:" << m_logFile->errorString();
							}
						}
					}
				}
			}
			if (opened)
			{
				qDebug() << "GPS Reader opened on port" << m_port;
			}
			else
			{
				qDebug() << "No GPS Com port found!";
				emit passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "GPSReader failed to find COM port setting. Not starting."));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "GPS_STATUS_STRING" << "No Com Port"));
			}
			QStringList msg;
			msg << "event:propertyset";
			msg << "8";
			msg << "GPS_LONGITUDE";
			msg << "";
			msg << "GPS_LATITUDE";
			msg << "";
			msg << "GPS_TIME";
			msg << "";
			msg << "GPS_FIXTYPE";
			msg << "";
			msg << "GPS_NUMSATS";
			msg << "";
			msg << "GPS_ALTITUDE";
			msg << "";
			msg << "GPS_FIXTYPESTRING";
			msg << "";
			msg << "GPS_STATUS_STRING";
			msg << "";
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",msg));
			QStringList list;
			list << "event:propertycomplexlistset";
			list << "GPS_SAT_INFO";
			list << "4";
			list << "SAT_PRN";
			list << "SAT_ELEVATION";
			list << "SAT_AZIMUTH";
			list << "SAT_SNR";
			list << QString::number(0);
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));

			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			if (m_logging)
			{
				m_logFile->flush();
				m_logFile->close();
				delete m_logFile;
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
void GPSReader::openPort()
{
	qDebug() << "GPSReader.cpp: Attempting to open com port" << m_port;
	if (serialClass->openPort(m_port,4800) < 0)
	{
		qDebug() << "GPSReader.cpp: GPS Port Open Failure!";
		passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "GPS_STATUS_STRING" << "No Device Found"));
		return;
	}
	badCounter = 0;
	serialTimer->start(50);
	qDebug() << "GPSReader.cpp: GPS Port Opened:" << m_port;
	passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "GPS_STATUS_STRING" << "Connected"));
	passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "GPSReader started on COM port: " + m_port));
}
void GPSReader::gpsPortFound(QString portname)
{

}

void GPSReader::location(double lon, double lat, long time,int fix,int sats, double altitude)
{
	QStringList msg;
	msg << "event:propertyset";
	msg << "7";
	msg << "GPS_LONGITUDE";
	msg << QString::number(lon);
	msg << "GPS_LATITUDE";
	msg << QString::number(lat);
	msg << "GPS_TIME";
	msg << QString::number(time);
	msg << "GPS_FIXTYPE";
	msg << QString::number(fix);
	msg << "GPS_NUMSATS";
	msg << QString::number(sats);
	msg << "GPS_ALTITUDE";
	msg << QString::number(altitude);
	msg << "GPS_FIXTYPESTRING";
	msg << (fix == 2 ? "3D Fix" : fix == 1 ? "2D Fix" : "None");
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",msg));
}

void GPSReader::gotFix(int fixType)
{
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "2" << "GPS_FIXTYPE" << QString::number(fixType) << "GPS_FIXTYPESTRING" << (fixType == 2 ? "3D Fix" : fixType== 1 ? "2D Fix" : "None")));
}

void GPSReader::lostFix()
{
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "GPS_FIXTYPE" << QString::number(0) << "GPS_FIXTYPESTRING" << "None"));
}

void GPSReader::velocity(long time,double heading, double speed)
{

}

void GPSReader::satStatus(int prn, int elevation, int azimuth, int snr)
{
	if (!m_satPropertyMap.contains(prn))
	{
		m_satPropertyMap[prn] = QList<QString>();
		m_satPropertyMap[prn].append(QString::number(elevation));
		m_satPropertyMap[prn].append(QString::number(azimuth));
		m_satPropertyMap[prn].append(QString::number(snr));
	}
	else
	{
		m_satPropertyMap[prn][0] = QString::number(elevation);
		m_satPropertyMap[prn][0] = QString::number(azimuth);
		m_satPropertyMap[prn][0] = QString::number(snr);
	}
	QStringList list;
	list << "event:propertycomplexlistset";
	list << "GPS_SAT_INFO";
	list << "4";
	list << "SAT_PRN";
	list << "SAT_ELEVATION";
	list << "SAT_AZIMUTH";
	list << "SAT_SNR";
	list << QString::number(0);
	int counter = 0;
	QMap<int, QList<QString> >::const_iterator i = m_satPropertyMap.constBegin();
	while (i != m_satPropertyMap.constEnd())
	{
	    //cout << i.key() << ": " << i.value() << endl;
		list << QString::number(i.key());
		list << i.value()[0];
		list << i.value()[1];
		list << i.value()[2];
		counter++;
		++i;
	}
	list[7] = QString::number(counter);
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));

}
void GPSReader::findGpsPort()
{
	QStringList portList;
	/*portList << "/dev/pts/0";
	portList << "/dev/pts/1";
	portList << "/dev/pts/2";
	portList << "/dev/pts/3";*/
	portList << "/dev/ttyUSB0";
	bool found = false;
	QString goodport = "";
	foreach(QString port,portList)
	{
		if (serialClass->openPort(port,4800) >= 0)
		{
			//Port Opened, try to read gps information.

			bool tryit = true;
			int count = 0;
			while (tryit)
			{
				QString line = serialClass->readLine();
				if (line != "")
				{
					if (line.startsWith("$"))
					{
						//This is probably a GPS message
						qDebug() << "GPS Message:" << line;
						goodport = port;
						found = true;
						tryit = false;
					}
					else
					{

					}
				}
				count++;
				if (count > 10)
				{
					tryit = false;
				}
			}
			serialClass->closePort();
		}
		else
		{
			qDebug() << "Error opening port:" << port;
		}
	}
	serialClass->closePort();
	if (found)
	{
		gpsPortFound(goodport);
	}
}

void GPSReader::serialTimerTimeout()
{
	QString line = serialClass->readLine();
	if (m_logging)
	{
		m_logFile->write(line.toStdString().c_str());
	}
	if (line != "")
	{
		badCounter = 0;
		line = line.replace("\n","");
		if (line.startsWith("$"))
		{
			if (line.startsWith("$GPGGA"))
			{
				QStringList split = line.split(",");
				if (split.count() > 6)
				{
					//location
					int fix = split[6].toInt();
					if (fix == 0)
					{
						//invalid fix
						if (m_fixType != 0)
						{
							m_fixType = 0;
							lostFix();
						}
						long time = split[1].toDouble();
						location(0,0,time,0,0,0);
					}
					else
					{
						if (m_fixType != fix)
						{
							m_fixType = fix;
							gotFix(fix);
						}

						//This is a horribly inefficient way of doing things. I should fix this at some point
						//But hell... if we have CPU to waste...
						double latdeg = split[2].split(".")[0].mid(0,2).toDouble();
						double latmin = split[2].split(".")[0].mid(2,2).toDouble();
						double latdec = split[2].split(".")[1].toDouble();
						QString num = QString::number(latmin) + "." + QString::number(latdec);
						double latfin = num.toDouble();
						double lat = latdeg + (latfin * (1.0/60.0));

						double londeg = split[4].split(".")[0].mid(0,3).toDouble();
						double lonmin = split[4].split(".")[0].mid(3,2).toDouble();
						double londec = split[4].split(".")[1].toDouble();
						QString lonnum = QString::number(lonmin) + "." + QString::number(londec);
						double lonfin = lonnum.toDouble();
						double lon = londeg + (lonfin * (1.0/60.0));


						//Lon/Lat is in the opposite notation, not normal N/E
						if (split[3] == "S")
						{
							lat = 0-lat;
						}
						if (split[5] == "W")
						{
							lon = 0-lon;
						}
						long time = split[1].toDouble();
						int sats = split[7].toInt();
						double alt = split[9].toDouble();
						location(lon,lat,time,fix,sats,alt);
					}
				}
			}
			else if (line.startsWith("$GPVTG"))
			{
				QStringList split = line.split(",");
				if (split.count() > 7)
				{
					double heading = split[1].toDouble();
					double speed = split[7].toDouble();
					velocity(0,heading,speed);
				}
			}
			else if (line.startsWith("$GPRMC"))
			{
				QStringList split = line.split(",");
				if (split.count() > 7)
				{
					long time = split[1].toDouble();
					double heading = split[8].toDouble();
					double speed = split[7].toDouble() * 1.85200;
					velocity(time,heading,speed);
				}
			}
			else if (line.startsWith("$GPGSV"))
			{
				//Satellite data
				//satStatus(int prn, int elevation, int azimuth, int snr);
				QStringList split = line.split(",");
				for (int i=4;i<split.length()-4;i++)
				{
					int prn = split[i].toInt();
					int elevation = split[i+1].toInt();
					int azimuth = split[i+2].toInt();
					int snr = split[i+3].toInt();
					satStatus(prn,elevation,azimuth,snr);
					i += 4;
				}
			}
			//qDebug() << "Line:" << line;
		}
		/*
		if (line.startsWith("$GPGGA"))
		{
			//Positional update
			QStringList split = line.split(",");
			if (split.count() > 5)
			{
				if (currentGGATime != split[1])
				{
					//$GPGGA,182144.000,3927.7342,N,07648.5991,W,2,06,1.7,175.6,M,-33.5,M,0.8,0000*48
					currentGGATime = split[1];
					QString lat = split[2];
					double dlat = (lat.toDouble() / 100.0) * ((split[5] == "S") ? -1 : 1);
					QString lon = split[4];
					double dlon = (lon.toDouble() / 100.0) * ((split[5] == "W") ? -1 : 1);
					//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:gps_timeupdate" << currentGGATime));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:gps_posupdate" << QString::number(dlat) << QString::number(dlon)));
				}
			}
			else
			{
				//Not a good gpgga message
			}
		}
		else if (line.startsWith("$GPRMC"))
		{
			QStringList split = line.split(",");
			if (split.count() > 9)
			{
				//$GPRMC,182142.000,A,3927.7347,N,07648.5991,W,0.78,165.20,230509,,*1E
				if (currentRMCTime != split[1])
				{
					currentRMCTime = split[1];
					//7 == knots
					//8 == track angle
					//9 == date
					QString mph = QString::number(split[7].toDouble() * 1.15077945);
					QString track = split[9];
					QString date = split[9];
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:gps_speedupdate" << mph << track));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:gps_datetime" << currentRMCTime << date));
					
				}
			}
		}
		*/
		//passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << line));
	}
	if (badCounter++ > 5)
	{
		qDebug() << "Error in serial";
		serialTimer->stop();
		serialClass->closePort();
		QTimer::singleShot(10000,this,SLOT(openPort()));
		passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "GPS_STATUS_STRING" << "Error in port. Reconnecting"));
	}
}

Q_EXPORT_PLUGIN2(GPSReaderPlugin, GPSReader)
