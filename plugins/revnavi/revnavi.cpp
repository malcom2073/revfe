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

#include "revnavi.h"

Navi::Navi()
{
	m_name = "RevNavi";
}

void Navi::init()
{
	qmlRegisterType<MapView>("MapView",0,1,"MapView");
	m_name = "RevNavi";
}
QString Navi::getName()
{
	return m_name;
}
void Navi::passPluginMessage(QString sender,QString message)
{

}
void Navi::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			//mapview = new MapView();
			//passCoreGUIItem(mapview);
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
Q_EXPORT_PLUGIN2(RevNaviPlugin, Navi)
