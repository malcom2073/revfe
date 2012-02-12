/***************************************************************************
*   Copyright (C) 2012 by Michael Carpenter (malcom2073)                  *
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

#include "revfusionbrain.h"
#include <QDebug>
FusionBrain::FusionBrain()
{
	m_name="RevFusionBrain";
}

void FusionBrain::init()
{
	m_fbFound = false;
	//fb = new FB();
	fbReadTimer = new QTimer(this);
	connect(fbReadTimer,SIGNAL(timeout()),this,SLOT(fbReadTimerTick()));

}
QString FusionBrain::getName()
{
	return m_name;
}
void FusionBrain::fbReadTimerTick()
{
	int status = fb->getAllOutputStatus();
	QList<bool> outputStatus;
	for (int i=0;i<16;i++)
	{
		if ((status >> i) & 1)
		{
			outputStatus.append(true);
		}
		else
		{
			outputStatus.append(false);
		}
	}
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:fusion_inputs" << ))
}

void FusionBrain::passPluginMessage(QString sender,QString message)
{

}
void FusionBrain::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			//Find the fusion brain, if it exists.
			if (/*!fb->findFB()*/0)
			{
				qDebug() << "RevFusionBrain: Unable to locate FusionBrainV4";
			}
			else
			{
				m_fbFound = true;
				//fb->connect();
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
	else if (message.getClass() == "output")
	{
		if (message.getMethod() == "set")
		{
			fb->setSingleOutput(message.getArgs()[0].toInt(),(message.getArgs()[1] == "on" ? true : false));
		}

	}
}
Q_EXPORT_PLUGIN2(RevFusionBrainPlugin, FusionBrain)
