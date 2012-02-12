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

#include "applauncher.h"
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QSettings>
void AppLauncher::init()
{
}
QString AppLauncher::getName()
{
	return "AppLauncher";
}
void AppLauncher::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}
void AppLauncher::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender);
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage("AppLauncher",IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage("AppLauncher",IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage("AppLauncher",IPCMessage("core","event","initclose",QStringList()));
		}
	}
	if (message.getClass() == "app")
	{
		if (message.getMethod() == "launch")
		{
			if (message.getArgs().count() > 0)
			{
				//container = new QX11EmbedContainer(this);
				//QString wId = QString::number(container->winId());
				//setenv("NAVIT_XID", wId.toAscii(), 1);
				//process = new QProcess(container);
				//process->start("navit");
				QString app = message.getArgs()[0];
				QProcess *proc = new QProcess(this);
				proc->start(app);
				processList.append(proc);
			}
		}
	}
	else if (message.getClass() == "window")
	{
		if (message.getMethod() == "embed")
		{
			//windowname currenthandle fullpath x y width height
			
		}

	}

}
Q_EXPORT_PLUGIN2(AppLauncherPlugin, AppLauncher)
