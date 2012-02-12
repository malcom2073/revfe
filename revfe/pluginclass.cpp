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

#include "pluginclass.h"
#include <QMetaType>
#include <QDebug>
PluginClass::PluginClass()
{
	isThreaded = false;
	isInitStarted = false;
	isInitCompleted = false;
	registered = false;
}
PluginClass::PluginClass(QObject *p,QString n,QString f)
{
	isThreaded = false;
	isInitStarted = false;
	isInitCompleted = false;
	registered = false;
	pluginObject = qobject_cast<BaseInterface*>(p);
	pluginName = n;
	pluginFileName = f;
}
PluginClass::PluginClass(QString f)
{
	isThreaded = false;
	isInitStarted = false;
	isInitCompleted = false;
	registered = false;
	pluginFileName = f;
}
void PluginClass::startThreaded()
{
	qRegisterMetaType<IPCMessage>("IPCMessage");
	isThreaded = true;
	threadedPluginObject = new PluginThreadClass();
	connect(threadedPluginObject,SIGNAL(passCoreMessage(QString,IPCMessage)),this,SIGNAL(passCoreMessage(QString,IPCMessage)));
	connect(threadedPluginObject,SIGNAL(passCoreModel(QString,QObject*)),this,SIGNAL(passCoreModel(QString,QObject*)));
	connect(threadedPluginObject,SIGNAL(passCoreGUIItem(QObject*)),this,SIGNAL(passCoreGUIItem(QObject*)));
	connect(threadedPluginObject,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),this,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),Qt::DirectConnection);
	connect(this,SIGNAL(passPluginMessageSignal(QString,IPCMessage)),threadedPluginObject,SLOT(passPluginMessageSlot(QString,IPCMessage)));
	connect(this,SIGNAL(passPluginMessageSignalBlocking(QString,IPCMessage)),threadedPluginObject,SLOT(passPluginMessageSlotBlocking(QString,IPCMessage)),Qt::DirectConnection);
	connect(threadedPluginObject,SIGNAL(pluginLoadFail(QString)),this,SIGNAL(pluginLoadFail(QString)));
	threadedPluginObject->passPluginFileName(pluginFileName);
	threadedPluginObject->start();
	passCoreMessage("PluginClass",IPCMessage("core","debug","debug",QStringList() << "Waiting for plugin: " << pluginFileName));
	threadedPluginObject->waitForReady();
	passCoreMessage("PluginClass",IPCMessage("core","debug","debug",QStringList() << "Done for plugin: " << pluginFileName));
	isPluginLoaded = true;
	pluginName = threadedPluginObject->pluginName;
	int stop = 1;
}
void PluginClass::startUnThreaded()
{
	qRegisterMetaType<IPCMessage>("IPCMessage");
	//QPluginLoader loader(pluginFileName);
	loader.setFileName(pluginFileName);
	QObject *plugin = loader.instance();
	if (plugin)
	{
		qDebug() << "Plugin loaded unthreaded: " << pluginFileName.mid(pluginFileName.lastIndexOf("/"));
		isPluginLoaded = true;
		pluginObject = qobject_cast<BaseInterface*>(plugin);
		if (!pluginObject)
		{
			qDebug() << "Error casting QObject *plugin to BaseInterface*. Something is seriously wrong here";
		}
		connect(pluginObject,SIGNAL(passCoreMessage(QString,IPCMessage)),this,SIGNAL(passCoreMessage(QString,IPCMessage)));
		connect(pluginObject,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),this,SIGNAL(passCoreMessage(QString,IPCMessage)),Qt::DirectConnection);
		connect(pluginObject,SIGNAL(passCoreModel(QString,QObject*)),this,SIGNAL(passCoreModel(QString,QObject*)));
		connect(pluginObject,SIGNAL(passCoreGUIItem(QObject*)),this,SIGNAL(passCoreGUIItem(QObject*)));
		connect(this,SIGNAL(passPluginMessageSignal(QString,IPCMessage)),pluginObject,SLOT(passPluginMessage(QString,IPCMessage)));
		connect(this,SIGNAL(passPluginMessageSignalBlocking(QString,IPCMessage)),pluginObject,SLOT(passPluginMessage(QString,IPCMessage)),Qt::DirectConnection);
		pluginName = pluginObject->getName();
		passCoreMessage("PluginClass",IPCMessage("core","register","register",QStringList() << pluginFileName << pluginName));
		pluginObject->init();
		int stop =1;
		//iBase->passPluginMessage("core",IPCMessage(iBase->getName(),"event","initstarted",QStringList()));
	}
	else
	{
		qDebug() << "Plugin failed to load unthreaded: " << pluginFileName.mid(pluginFileName.lastIndexOf("/"));
		emit pluginLoadFail(pluginFileName);
	}

}
void PluginClass::passPluginMessageBlocking(QString sender,IPCMessage message)
{
	passPluginMessage(sender,message,true);
}
void PluginClass::passPluginMessage(QString sender,IPCMessage message,bool blocking)
{
	if (blocking)
	{
		if (isThreaded)
		{
			emit passPluginMessageSignalBlocking(sender,message);
			if (message.getMethod() == "initstarted")
			{
				//threadedPluginObject->waitForInitStarted();
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "AAinitstarted from: " + pluginName));
				isInitStarted = true;
			}
			else if (message.getMethod() == "initcomplete")
			{
				//threadedPluginObject->waitForInitComplete();
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "AAinitcomplete from: " + pluginName));
				isInitCompleted = true;
			}
			else if (message.getMethod() == "initclose")
			{
				threadedPluginObject->waitForInitClose();
				//isInitClose = true;
			}
			else if (message.getMethod() == "registered")
			{
				registered = true;
			}
		}
		else
		{
			pluginObject->passPluginMessage(sender,message);
			if (message.getMethod() == "initstarted")
			{
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initstarted from: " + pluginName));
				isInitStarted = true;
			}
			else if (message.getMethod() == "initcomplete")
			{
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initcomplete from: " + pluginName));
				isInitCompleted = true;
			}
			else if (message.getMethod() == "initclose")
			{

			}
			else if (message.getMethod() == "registered")
			{
				registered = true;
			}
		}
	}
	else
	{
		if (isThreaded)
		{
			emit passPluginMessageSignal(sender,message);
			if (message.getMethod() == "initstarted")
			{
				//threadedPluginObject->waitForInitStarted();
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initstarted from: " + pluginName));
				isInitStarted = true;
			}
			else if (message.getMethod() == "initcomplete")
			{
				//threadedPluginObject->waitForInitComplete();
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initcomplete from: " + pluginName));
				isInitCompleted = true;
			}
			else if (message.getMethod() == "initclose")
			{
				threadedPluginObject->waitForInitClose();
				//isInitClose = true;
			}
			else if (message.getMethod() == "registered")
			{
				registered = true;
			}
		}
		else
		{
			pluginObject->passPluginMessage(sender,message);
			if (message.getMethod() == "initstarted")
			{
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initstarted from: " + pluginName));
				isInitStarted = true;
			}
			else if (message.getMethod() == "initcomplete")
			{
				passCoreMessage(pluginName,IPCMessage("core","debug","debug",QStringList() << "initcomplete from: " + pluginName));
				isInitCompleted = true;
			}
			else if (message.getMethod() == "initclose")
			{

			}
			else if (message.getMethod() == "registered")
			{
				registered = true;
			}
		}
	}
}
void PluginClass::passPluginMessage(QString sender,IPCMessage message)
{
	passPluginMessage(sender,message,false);
}
void PluginClass::unloadPlugin()
{
	if (isThreaded)
	{
		threadedPluginObject->unloadPlugin();
		isPluginLoaded = false;
	}
	else
	{
		loader.unload();
		isPluginLoaded = false;
	}
}
void PluginClass::reload()
{
	if (isThreaded)
	{
		startThreaded();
	}
	else
	{
		startUnThreaded();
	}
}
