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

#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include "SerialClass.h"
#include "ui_MainWindowUi.h"
#include "HdRadioMessage.h"
#include "HdRadioCodes.h"
#include "HdRadioCodes.h"

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
private:
	SerialClass *serial;
	Ui::Form ui;
	QTimer *serialTimer;
	QByteArray recvArray;
	bool inMessage;
	bool isFirst;
	int messageCount;
	int currentCount;
	void parseMessage(QByteArray arr);
	void sendMessage(QByteArray arr);
	HdRadioCodes *hdCodes;
	int totalSubChannels;
	int currentSubChannel;
	QTimer *subChannelTimer;
	QList<QString> subChannelArtist;
	QList<QString> subChannelTitle;
private slots:
	void connectPushButtonPushed();
	void disconnectPushButtonPushed();
	void serialReadTimer();
	void powerPushButtonPushed();
	void getInfoPushButtonPushed();
	void subChannelPushButtonPushed();
	void subChannelTimerTimeout();
};
