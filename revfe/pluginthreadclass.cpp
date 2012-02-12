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

#include "pluginthreadclass.h"
#include <baseinterface.h>
#include <QMetaType>
#include <QApplication>
#include <QDebug>

PluginThreadClass::PluginThreadClass(QObject *parent) : QThread(parent)
{
	ready = false;
	initstarted = false;
	initcomplete = false;
	initclose = false;
}
void PluginThreadClass::passPluginFileName(QString fileName)
{
	filename = fileName;
}
void PluginThreadClass::run()
{
	qRegisterMetaType<IPCMessage>("IPCMessage");
	//QPluginLoader loader(filename);
	loader.setFileName(filename);
	QString errorString = loader.errorString();
	QObject *plugin = loader.instance();
	bool running = false;
	
	if (plugin)
	{
		//printf("Plugin Found...\n");
		BaseInterface *iBase = qobject_cast<BaseInterface*>(plugin);
		if (iBase)
		{
			qDebug() << "Plugin loaded: " << filename.mid(filename.lastIndexOf("/"));
			connect(iBase,SIGNAL(passCoreMessage(QString,IPCMessage)),this,SIGNAL(passCoreMessage(QString,IPCMessage)));
			connect(iBase,SIGNAL(passCoreModel(QString,QObject*)),this,SIGNAL(passCoreModel(QString,QObject*)));
			connect(iBase,SIGNAL(passCoreGUIItem(QObject*)),this,SIGNAL(passCoreGUIItem(QObject*)));
			connect(iBase,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),this,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),Qt::BlockingQueuedConnection);
			connect(this,SIGNAL(passPluginMessage(QString,IPCMessage)),iBase,SLOT(passPluginMessage(QString,IPCMessage)));//,Qt::DirectConnection);
			connect(this,SIGNAL(passPluginMessageBlocking(QString,IPCMessage)),iBase,SLOT(passPluginMessage(QString,IPCMessage)),Qt::BlockingQueuedConnection);
			pluginName = iBase->getName();
			passCoreMessage("PluginThreadClass",IPCMessage("core","register","register",QStringList() << filename << pluginName));
			iBase->init();
			qDebug() << "Plugin init complete: " << filename.mid(filename.lastIndexOf("/"));
			running = true;
		}
		else
		{
			qDebug() << QString("Plugin failed to load: ").append(filename);
			passCoreMessage("PluginThreadClass",IPCMessage("core","debug","debug",QStringList() << "Plugin load fail:" << pluginName));
			int stop = 1;
			emit pluginLoadFail(filename);
			ready = true;
		}
	}
	else
	{
		qDebug() << QString("Plugin failed to instantiate: ").append(filename.mid(filename.lastIndexOf("/"))).append(" : ").append(errorString);
		passCoreMessage("PluginThreadClass",IPCMessage("core","debug","debug",QStringList() << "Plugin load fail:" << pluginName));
		emit pluginLoadFail(filename);
		ready = true;
		//QMessageBox::information(0,"Test",QString("Error loading plugin: ").append(fileName).append(loader.errorString()));
	}
	if (running)
	{
 		ready = true;
		exec();
	}
	else
	{
		ready = true;
		exec();
	}
}
void PluginThreadClass::passPluginMessageSlotBlocking(QString sender, IPCMessage message)
{
	passPluginMessageSlot(sender,message,true);
}

void PluginThreadClass::passPluginMessageSlot(QString sender, IPCMessage message, bool blocking)
{
	if (message.getTarget() != pluginName)
	{
		return;
	}
	if (blocking)
	{
		passPluginMessageBlocking(sender,message);
	}
	else
	{
		passPluginMessage(sender,message);
	}
	if (message.getMethod() == "initstarted")
	{
		initstarted = true;
	}
	else if (message.getMethod() == "initcomplete")
	{
		initcomplete = true;
	}
	else if (message.getMethod() == "initclose")
	{
		initclose = true;
	}

}
void PluginThreadClass::passPluginMessageSlot(QString sender, IPCMessage message)
{
	passPluginMessageSlot(sender,message,false);
}
int PluginThreadClass::waitForReady()
{
	int count = 0;
	while (!ready)
	{
		//count++;
		//QApplication::processEvents();
		QThread::currentThread()->msleep(5);
		//if (count > 1000000) count = 0;
	}
	return 0;
}
int PluginThreadClass::waitForInitStarted()
{
	int count = 0;
	while (!initstarted)
	{
		count++;
		if (count > 1000000) count = 0;
	}
	return 0;
}
int PluginThreadClass::waitForInitComplete()
{
	int count = 0;
	while (!initcomplete)
	{
		count++;
		
		if (count > 1000000) count = 0;
	}
	return 0;
}
int PluginThreadClass::waitForInitClose()
{
	while (!initclose)
	{

	}
	return 0;
}
void PluginThreadClass::unloadPlugin()
{
	loader.unload();
}
