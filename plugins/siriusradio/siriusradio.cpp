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

#include "SiriusRadio.h"
#include <QDebug>
void SiriusRadio::init()
{
	radio = new Sirius(this);
	connect(radio,SIGNAL(currentStationChanged(int)),this,SLOT(siriusStationChanged(int)));
	connect(radio,SIGNAL(channelSongChanged(int,QString)),this,SLOT(siriusSongChanged(int,QString)));
	connect(radio,SIGNAL(channelNameChanged(int,QString)),this,SLOT(siriusNameChanged(int,QString)));
	connect(radio,SIGNAL(channelIconChanged(int,QIcon)),this,SLOT(siriusIconChanged(int,QIcon)));
	connect(radio,SIGNAL(channelGenreChanged(int,QString)),this,SLOT(siriusGenreChanged(int,QString)));
	connect(radio,SIGNAL(channelArtistChanged(int,QString)),this,SLOT(siriusArtistChanged(int,QString)));
	m_port = "";
}
QString SiriusRadio::getName()
{
	return "SiriusRadio";
}
void SiriusRadio::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender)
	Q_UNUSED(message)
}
void SiriusRadio::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender)
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			//Should init with settings.
			if (message.getArgs().size() == 0)
			{
				qDebug() << "SiriusRadio plugin initted without settings. Disabling interface";
				return;
			}
			for (int i=0;i<message.getArgs().count();i++)
			{
				QStringList split = message.getArgs()[0].split('=');
				if (split[0] == "comport")
				{
					m_port = split[1];
				}
			}
			passCoreMessage("SiriusRadio",IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			passCoreMessage("SiriusRadio",IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			passCoreMessage("SiriusRadio",IPCMessage("core","event","initclose",QStringList()));
		}

		else if (message.getMethod() == "throw")
		{
		}
	}
	else if (message.getClass() == "sirius")
	{
		if (message.getMethod() == "connect")
		{
			if (m_port != "")
			{
				radio->openSerial(m_port);
				radio->initSCC1();
			}
		}
		else if (message.getMethod() == "setstation")
		{
			if (message.getArgs().size() > 0)
			{
				radio->setCurrentStation(message.getArgs()[0].toInt());
			}
		}
		else if (message.getMethod() == "power")
		{
			if (message.getArgs().size() > 0)
			{
				if (message.getArgs()[0] == "on")
				{
					radio->setPower(true);
				}
				else if (message.getArgs()[0] == "off")
				{
					radio->setPower(false);
				}
			}
		}
	}
}
void SiriusRadio::siriusStationChanged(int channel)
{
	if (m_channelNameMap.contains(channel))
	{
		emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_CHANNEL_NAME" << m_channelNameMap[channel]));
	}
	else
	{
		emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_CHANNEL_NAME" << QString::number(channel)));
	}
	m_currentChannel = channel;
}

void SiriusRadio::siriusSongChanged(int channel,QString newSong)
{
	Q_UNUSED(channel);
	emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_SONG" << newSong));
}

void SiriusRadio::siriusNameChanged(int channel,QString newName)
{
	m_channelNameMap[channel] = newName;
	if (channel == m_currentChannel)
	{
		emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_CHANNEL_NAME" << m_channelNameMap[channel]));
	}
}

void SiriusRadio::siriusIconChanged(int channel,QIcon newIcon)
{
	Q_UNUSED(channel);
	Q_UNUSED(newIcon);
}

void SiriusRadio::siriusGenreChanged(int channel,QString newGenre)
{
	emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_GENRE" << newGenre));
}

void SiriusRadio::siriusArtistChanged(int channel,QString newArtist)
{
	emit passCoreMessage("SiriusRadio",IPCMessage("core","event","throw",QStringList() << "propertyset" << "1" << "SIRIUS_CURRENT_ARTIST" << newArtist));
}

Q_EXPORT_PLUGIN2(SiriusRadioPlugin, SiriusRadio)
