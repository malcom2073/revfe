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

#include "revhdradio.h"
//#include <QMessageBox>
#include <QFile>
#include <QDebug>


HDRadio::HDRadio()
{
	m_name = "RevHDRadio";
}

void HDRadio::init()
{
	totalSubChannels = 0;
	currentSubChannel = 0;
	hdCodes = new HdRadioCodes();
	serial = new SerialClass();
	serialTimer = new QTimer(this);
	connect(serialTimer,SIGNAL(timeout()),this,SLOT(serialReadTimer()));
	subChannelTimer = new QTimer(this);
	connect(subChannelTimer,SIGNAL(timeout()),this,SLOT(subChannelTimerTimeout()));
	inMessage = false;
	isFirst = false;
	messageCount = 0;
	currentCount = 0;
	//for (int i=0;i<7;i++)
	//{
		//presetFreqList.append("");
	//}
	nextOnSet = false;
	/*
	QFile hdSettings("hdradio.conf");
	hdSettings.open(QIODevice::ReadWrite);
	QString hdSettingsStr(hdSettings.readAll());
	hdSettings.close();
	if (hdSettingsStr.isEmpty())
	{
		passCoreMessage("GPSReader",IPCMessage("core","debug","debug",QStringList("Unable to read hdradio.conf, not running...")));
		return;
	}
	QStringList list = hdSettingsStr.split("\r\n");
	//hdPort = list[0];
	for (int i=1;i<list.count();i++)
	{
		if (!list[i].isEmpty())
		{
			presetFreqList.append(list[i]);
		}
	}
	*/

}
QString HDRadio::getName()
{
	return m_name;
}
void HDRadio::passPluginMessage(QString sender,QString message)
{

}
void HDRadio::tune(QString freqStr)
{
	if (freqStr == "currentFreq")
	{
		freqStr = currentFreq;
	}
	subChannelTitle.clear();
	subChannelArtist.clear();
	currentSubChannel = 0;
	subChannelTimer->stop();
	totalSubChannels =0;
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "2" << "HDRADIO_ARTIST" << "" << "HDRADIO_TITLE" << ""));
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-ARTIST%" << ""));
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-TITLE%" << ""));
	double value = freqStr.toDouble();
	QString valueStr = QString::number(value);
	valueStr.replace(".","");
	QString valueFinalStr = "";
	int valueInt = valueStr.toInt();
	char top = (valueInt >> 8) & 0xFF;
	char bottom = valueInt & 0xFF;
	QByteArray cmd;
	cmd.append(hdCodes->getCommandBytesFromString("tune"));
	cmd.append(hdCodes->getOperationBytesFromString("set"));
	cmd.append(hdCodes->getConstantBytesFromString("fm"));
	cmd.append(bottom);
	cmd.append(top);
	cmd.append((char)0x00);
	cmd.append((char)0x00);
	cmd.append(hdCodes->getConstantBytesFromString("zero"));
	sendMessage(cmd);
	cmd.clear();
	cmd.append((char)0x01);
	cmd.append((char)0x02);
	cmd.append((char)0x01);
	cmd.append((char)0x00);
	sendMessage(cmd);
	cmd.clear();
	cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
	cmd.append(hdCodes->getOperationBytesFromString("get"));
	sendMessage(cmd);
	currentFreq = freqStr;
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CURRENTFREQ" << freqStr));
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << freqStr));
}
void HDRadio::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "radio")
	{
		if (message.getMethod() == "preset")
		{
			if (message.getArgs()[0] == "tune")
			{
				if (nextOnSet)
				{
					nextOnSet = false;
					int num = message.getArgs()[1].toInt();
					if ((num-1) < presetFreqList.count())
					{
						presetFreqList[num-1] = currentFreq;
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << QString("%HDRADIO-PRESET").append(QString::number(num+1)).append("%") << currentFreq));
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << QString("HDRADIO-PRESET").append(QString::number(num+1)) << currentFreq));
						
					}
				}
				else
				{
					int num = message.getArgs()[1].toInt();
					if ((num-1) < presetFreqList.count())
					{
						//tune(presetFreqList[num]);
						//Temporary hack to make it look usable.
						currentFreq = presetFreqList[num-1];
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CURRENTFREQ" << presetFreqList[num]));
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << presetFreqList[num]));
						tune(currentFreq);
					}
				}
			}
			else if (message.getArgs()[0] == "set")
			{
				nextOnSet = true;
			}
		}
		else if (message.getMethod() == "seekup")
		{
			currentFreq = QString::number(currentFreq.toDouble() + 0.2);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << currentFreq));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CURRENTFREQ" << currentFreq));
			tune(currentFreq);
		}
		else if (message.getMethod() == "seekdown")
		{
			currentFreq = QString::number(currentFreq.toDouble() - 0.2);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << currentFreq));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CURRENTFREQ" << currentFreq));
			tune(currentFreq);
		}
		else if (message.getMethod() == "tune")
		{
			if (message.getArgs().count() > 0)
			{
				tune(message.getArgs()[0]);
			}
		}
		else if (message.getMethod() == "setsubchannel")
		{
			if (message.getArgs().count() > 0)
			{
				if (totalSubChannels != 0)
				{
					//currentSubChannel++;
					if ((1 << message.getArgs()[0].toInt()) >= totalSubChannels)
					{
						return;
					}
					else
					{
						currentSubChannel = message.getArgs()[0].toInt();
					}
					if (subChannelTitle.count() > currentSubChannel)
					{
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:hdradio_hdtitle" << subChannelTitle[currentSubChannel]));
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-TITLE%" << "97.9FM"));
						//ui.hdTitleLineEdit->setText(subChannelTitle[currentSubChannel]);
					}
					if (subChannelArtist.count() > currentSubChannel)
					{
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:hdradio_hdartist" << subChannelArtist[currentSubChannel]));
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-ARTIST%" << "97.9FM"));
						//ui.hdArtistLineEdit->setText(subChannelArtist[currentSubChannel]);
					}
					QByteArray ba;
					ba.append(hdCodes->getCommandBytesFromString("hdsubchannel"));
					ba.append(hdCodes->getOperationBytesFromString("set"));
					ba.append((1 << currentSubChannel));
					sendMessage(ba);
				}
			}
		}
		else if (message.getMethod() == "connect")
		{
			serial->openPort(hdPort);
			QByteArray ba;
			ba.append(hdCodes->getCommandBytesFromString("power"));
			ba.append(hdCodes->getOperationBytesFromString("set"));
			ba.append(hdCodes->getConstantBytesFromString("up"));
			sendMessage(ba);
			serialTimer->start(10);
		}
	}
	else if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			bool opened = false;
			QString port = "";
			if (message.getArgs().count() > 0)
			{
				for (int i=0;i<message.getArgs().count()-2;i++)
				{
					QString key = message.getArgs()[i];
					QString value = message.getArgs()[i+1];
					QString type = message.getArgs()[i+2];
					QString id = message.getArgs()[i+3];
					i+=3;
					//qDebug() << key << value << type;
					if (key.split("=")[1] == "comport")
					{
						hdPort = value.split("=")[1];
						opened = true;
					}
					else if (key.split("=")[1].startsWith("preset"))
					{
						presetFreqList.append(value.split("=")[1]);
					}
					/*QStringList split = message.getArgs()[i].split('=');
					if (split[0] == "comport")
					{
						hdPort = split[1];
						opened  = true;
					}
					else if (split[0].startsWith("preset"))
					{
						presetFreqList.append(split[1]);
					}*/
				}
			}
			if (opened)
			{
				passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "RevHDRadio started on COM port: " + port));
			}
			else
			{
				passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "RevHDRadio failed to find COM port setting. Not starting."));
			}
							QStringList evt;
			evt << "event:propertyset";
			evt << "10";
			for (int i=0;i<10;i++)
			{
				evt << "HDRADIO_PRESET" + QString::number(i+1);
				evt << "";
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));

			passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			if (presetFreqList.count() > 0)
			{
				currentFreq = presetFreqList[0];
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << currentFreq));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CURRENTFREQ" << currentFreq));
				QStringList evt;
				evt << "event:propertyset";
				evt << QString::number(presetFreqList.count());
				for (int i=0;i<presetFreqList.count();i++)
				{
					evt << "HDRADIO_PRESET" + QString::number(i+1);
					evt << presetFreqList[i];
					//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET" + QString::number(i + 1) + "%" << presetFreqList[i]));
				}
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
			}

			/*
			currentFreq = "97.9";
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-CURRENTFREQ%" << "97.9"));
			presetFreqList[0] = "93.7";
			presetFreqList[1] = "97.9";
			presetFreqList[2] = "98.5";
			presetFreqList[3] = "99.9";
			presetFreqList[4] = "100.7";
			presetFreqList[5] = "101.1";
			presetFreqList[6] = "101.3";
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET1%" << "93.7"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET2%" << "97.9"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET3%" << "98.5"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET4%" << "99.9"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET5%" << "100.7"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET6%" << "101.1"));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-PRESET7%" << "101.3"));
			*/
			passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
void HDRadio::serialReadTimer()
{
	char c;
	int recv = serial->read(&c,1);
	if (recv > 0)
	{
		if ((unsigned char)c == 0xA4)
		{
			if (inMessage)
			{
				//need to restart message
				currentCount = 0;
				recvArray.clear();
				isFirst = true;
				recvArray.append(c);
			}
			else
			{
				currentCount = 0;
				recvArray.clear();
				inMessage = true;
				isFirst = true;
				recvArray.append(c);
			}
		}
		else
		{
			if (inMessage)
			{
				if (isFirst)
				{
					messageCount = (int)c;
					recvArray.append(c);
					isFirst = false;
				}
				else
				{
					if (currentCount >= messageCount)
					{
						recvArray.append(c);
						parseMessage(recvArray);
						inMessage = false;
						isFirst = false;
						
					}
					else
					{
						recvArray.append(c);
						currentCount ++;
					}
				}
			}
			else
			{
				//Something's wrong here?
			}
		}
		//recvArray.append(c);
	}

}
void HDRadio::subChannelTimerTimeout()
{
	QByteArray cmd;
	cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
	cmd.append(hdCodes->getOperationBytesFromString("get"));
	sendMessage(cmd);
}
void HDRadio::parseMessage(QByteArray arr)
{
	HdRadioMessage tmp(arr,hdCodes);
	if (tmp.valid)
	{
		if (tmp.messageClass == "power")
		{
			if (tmp.messageDirection == "reply")
			{
				if (tmp.messageArg == "up")
				{
					//Power on!
					int stop = 1;
				}
				else
				{
					//QMessageBox::information(0,"Wee",tmp.messageArg);
				}
			}
		}
		else if (tmp.messageClass == "tune")
		{
			if (tmp.messageDirection == "reply")
			{
				if (tmp.messageArg != "")
				{
					//this->setWindowTitle(tmp.messageArg);
				}
			}
			
		}
		else if (tmp.messageClass == "hdcallsign")
		{
			//ui.hdCallSignLineEdit->setText(QString("Call: ").append(tmp.messageArg));
		}
		else if (tmp.messageClass == "hdtitle")
		{
			int num = tmp.messageArg.split(0x24)[0].toInt();
			QString mes = tmp.messageArg.split(0x24)[1];
			if (subChannelTitle.count() > (num / 2))
			{
				subChannelTitle[num / 2] = mes;
			}
			else
			{
				subChannelTitle.append(mes);
			}
			if ((1 << currentSubChannel) == num)
			{
				//ui.hdTitleLineEdit->setText(QString("Title: ").append(mes));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-TITLE%" << mes));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_TITLE" << mes));
			}
			//if (1 << current
		}
		else if (tmp.messageClass == "hdartist")
		{
			int num = tmp.messageArg.split(0x24)[0].toInt();
			QString mes = tmp.messageArg.split(0x24)[1];
			if (subChannelArtist.count() > (num / 2))
			{
				subChannelArtist[num / 2] = mes;
			}
			else
			{
				subChannelArtist.append(mes);
			}
			if ((1 << currentSubChannel) == num)
			{
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdartist" << mes));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%HDRADIO-ARTIST%" << mes));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_ARTIST" << mes));
				//ui.hdArtistLineEdit->setText(QString("Artist: ").append(mes));
			}
		}
		else if (tmp.messageClass == "hdactive")
		{
			//this->setWindowTitle(QString("HD Active: ").append(tmp.messageArg));
			if (tmp.messageArg == "true")
			{
				QByteArray cmd;
				cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
				cmd.append(hdCodes->getOperationBytesFromString("get"));
				sendMessage(cmd);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdactive" << "true"));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_HDACTIVE" << "true"));
			}
			else if (tmp.messageArg == "false")
			{
				QByteArray cmd;
				cmd.append(hdCodes->getCommandBytesFromString("rdsenable"));
				cmd.append(hdCodes->getOperationBytesFromString("set"));
				cmd.append(hdCodes->getConstantBytesFromString("one"));
				sendMessage(cmd);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdactive" << "false"));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_HDACTIVE" << "false"));
			}
		}
		else if (tmp.messageClass == "rdsradiotext")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				//ui.radioTextLineEdit->setText(tmp.messageArg);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_rdsradiotext" << tmp.messageArg));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_RDSRADIOTEXT" << tmp.messageArg));
			}
		}
		else if (tmp.messageClass == "hdstationname")
		{
			//ui.hdStationNameLineEdit->setText(tmp.messageArg);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdstationname" << tmp.messageArg));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_STATIONNAME" << tmp.messageArg));
		}
		else if (tmp.messageClass == "hdsubchannelcount")
		{
			totalSubChannels = tmp.messageArg.replace("0","").toInt();
			//ui.subChannelLineEdit->setText(QString("Sub Channels: ").append(tmp.messageArg));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdsubchannelcount" << tmp.messageArg));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_CHANNELCOUNT" << tmp.messageArg));
		}
		else if (tmp.messageClass == "rdsprogramservice")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				//ui.radioProgramLineEdit->setText(tmp.messageArg);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_rdsprogramservice" << tmp.messageArg));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_RDSSERVICE" << tmp.messageArg));
			}
		}
		else if (tmp.messageClass == "rdsgenre")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				//ui.genreLineEdit->setText(tmp.messageArg);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_rdsgenre" << tmp.messageArg));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_RDSGENRE" << tmp.messageArg));
			}
		}
		else if (tmp.messageClass == "hdstreamlock")
		{
			if (tmp.messageArg == "true")
			{
				QByteArray cmd;
				cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
				cmd.append(hdCodes->getOperationBytesFromString("get"));
				sendMessage(cmd);
				//ui.lineEdit->setText("Stream Lock: 1");
				subChannelTimer->start(5000);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdstreamlock" << "true"));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_STREAMLOCK" << "true"));
			}
			else if (tmp.messageArg == "false")
			{
				//ui.lineEdit->setText("Stream Lock: 0");
				subChannelTimer->stop();
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:HDRADIO_hdstreamlock" << "false"));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "HDRADIO_STREAMLOCK" << "false"));
			}
		}
	}

}

void HDRadio::sendMessage(QByteArray arr)
{
	int csum = 0;
	for (int i=0;i<arr.length();i++)
	{
		csum += arr[i];
	}
	csum += arr.length();
	csum += 0xA4;
	arr.prepend(arr.length());
	arr.prepend(0xA4);
	arr.append((csum % 256));
	serial->write(arr);
}

Q_EXPORT_PLUGIN2(RevHDRadioPlugin, HDRadio)
