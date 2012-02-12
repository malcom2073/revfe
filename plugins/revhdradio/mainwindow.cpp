/***************************************************************************
*   Copyright (C) 2009 by Michael Carpenter (malcom2073)                  *
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

#include "MainWindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
	totalSubChannels = 0;
	currentSubChannel = 0;
	hdCodes = new HdRadioCodes();
	ui.setupUi(this);
	serial = new SerialClass();
	connect(ui.connectPushButton,SIGNAL(clicked()),this,SLOT(connectPushButtonPushed()));
	connect(ui.disconnectPushButton,SIGNAL(clicked()),this,SLOT(disconnectPushButtonPushed()));
	connect(ui.powerPushButton,SIGNAL(clicked()),this,SLOT(powerPushButtonPushed()));
	connect(ui.getInfoPushButton,SIGNAL(clicked()),this,SLOT(getInfoPushButtonPushed()));
	connect(ui.subChannelPushButton,SIGNAL(clicked()),this,SLOT(subChannelPushButtonPushed()));
	serialTimer = new QTimer(this);
	connect(serialTimer,SIGNAL(timeout()),this,SLOT(serialReadTimer()));
	subChannelTimer = new QTimer(this);
	connect(subChannelTimer,SIGNAL(timeout()),this,SLOT(subChannelTimerTimeout()));
	inMessage = false;
	isFirst = false;
	messageCount = 0;
	currentCount = 0;
}

void MainWindow::connectPushButtonPushed()
{
	serial->openPort("COM1");
	int num = 0;
	num += 8;
	num += 1;
	num += 1;
	QByteArray ba;
	//ba.append((char)0xA4);
	//ba.append((char)0x08);
	ba.append(hdCodes->getCommandBytesFromString("power"));
	ba.append(hdCodes->getOperationBytesFromString("set"));
	ba.append(hdCodes->getConstantBytesFromString("up"));

	//ba.append((char)(num & 256));
	//serial->write(ba);
	sendMessage(ba);
	serialTimer->start(10);
}
void MainWindow::disconnectPushButtonPushed()
{
	serial->closePort();
}
void MainWindow::serialReadTimer()
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
void MainWindow::parseMessage(QByteArray arr)
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
					this->setWindowTitle(tmp.messageArg);
				}
			}
			
		}
		else if (tmp.messageClass == "hdcallsign")
		{
			ui.hdCallSignLineEdit->setText(QString("Call: ").append(tmp.messageArg));
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
				ui.hdTitleLineEdit->setText(QString("Title: ").append(mes));
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
				ui.hdArtistLineEdit->setText(QString("Artist: ").append(mes));
			}
		}
		else if (tmp.messageClass == "hdactive")
		{
			this->setWindowTitle(QString("HD Active: ").append(tmp.messageArg));
			if (tmp.messageArg == "true")
			{
				QByteArray cmd;
				cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
				cmd.append(hdCodes->getOperationBytesFromString("get"));
				sendMessage(cmd);
			}
			else if (tmp.messageArg == "false")
			{
				QByteArray cmd;
				cmd.append(hdCodes->getCommandBytesFromString("rdsenable"));
				cmd.append(hdCodes->getOperationBytesFromString("set"));
				cmd.append(hdCodes->getConstantBytesFromString("one"));
				sendMessage(cmd);
			}
		}
		else if (tmp.messageClass == "rdsradiotext")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				ui.radioTextLineEdit->setText(tmp.messageArg);
			}
		}
		else if (tmp.messageClass == "hdstationname")
		{
			ui.hdStationNameLineEdit->setText(tmp.messageArg);
		}
		else if (tmp.messageClass == "hdsubchannelcount")
		{
			totalSubChannels = tmp.messageArg.replace("0","").toInt();
			ui.subChannelLineEdit->setText(QString("Sub Channels: ").append(tmp.messageArg));
		}
		else if (tmp.messageClass == "rdsprogramservice")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				ui.radioProgramLineEdit->setText(tmp.messageArg);
			}
		}
		else if (tmp.messageClass == "rdsgenre")
		{
			if (tmp.messageDirection == "reply")
			{
				//this->setWindowTitle(tmp.messageArg);
				ui.genreLineEdit->setText(tmp.messageArg);
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
				ui.lineEdit->setText("Stream Lock: 1");
				subChannelTimer->start(5000);
			}
			else if (tmp.messageArg == "false")
			{
				ui.lineEdit->setText("Stream Lock: 0");
				subChannelTimer->stop();
			}
		}
	}

}
void MainWindow::powerPushButtonPushed()
{
	subChannelTitle.clear();
	subChannelArtist.clear();
	currentSubChannel = 0;
	subChannelTimer->stop();
	ui.lineEdit->clear();
	totalSubChannels =0;
	ui.genreLineEdit->clear();
	ui.hdArtistLineEdit->clear();
	ui.hdCallSignLineEdit->clear();
	ui.hdStationNameLineEdit->clear();
	ui.hdTitleLineEdit->clear();
	ui.radioProgramLineEdit->clear();
	ui.radioTextLineEdit->clear();
	double value = ui.doubleSpinBox->value();
	QString valueStr = QString::number(value);
	valueStr.replace(".","");
	QString valueFinalStr = "";
	//valueFinalStr.setNum(valueStr.toInt(),16);
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
	//cmd.append((char)0x00);
	//cmd.append((char)0x00);
	//cmd.append((char)0x00);
	//cmd.append((char)0x00);
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
	//hdstationname
}
void MainWindow::sendMessage(QByteArray arr)
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
void MainWindow::getInfoPushButtonPushed()
{
	QByteArray ba;
	ba.append((char)0x09);
	ba.append((char)0x02);
	ba.append((char)0x01);
	ba.append((char)0x00);
	sendMessage(ba);
}
void MainWindow::subChannelPushButtonPushed()
{
	if (totalSubChannels != 0)
	{
		currentSubChannel++;
		if ((1 << currentSubChannel) >= totalSubChannels)
		{
			currentSubChannel = 0;
		}
		if (subChannelTitle.count() > currentSubChannel)
		{
			ui.hdTitleLineEdit->setText(subChannelTitle[currentSubChannel]);
		}
		if (subChannelArtist.count() > currentSubChannel)
		{
			ui.hdArtistLineEdit->setText(subChannelArtist[currentSubChannel]);
		}
		QByteArray ba;
		ba.append(hdCodes->getCommandBytesFromString("hdsubchannel"));
		ba.append(hdCodes->getOperationBytesFromString("set"));
		ba.append((1 << currentSubChannel));
		sendMessage(ba);
	}
}
void MainWindow::subChannelTimerTimeout()
{
	QByteArray cmd;
	cmd.append(hdCodes->getCommandBytesFromString("hdsubchannelcount"));
	cmd.append(hdCodes->getOperationBytesFromString("get"));
	sendMessage(cmd);
}