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

#include "DotNetManager.h"
#include <QMessageBox>
#include <QMetaType>
#include <QFile>
//#define MANAGEDWRAPPER_EXPORTS
#include "IWrapperClass.h"
#include <QMap>

void tmp(void *classptr,std::string one,std::string two)
{
	((DotNetManager*)classptr)->passMessageFromDotNet(QString(one.c_str()),QString(two.c_str()));
}
void DotNetManager::passMessageFromDotNet(QString sender,QString message)
{
	//QMessageBox::information(0,"Coolio",QString("Testing: ").append(message));
	IPCMessage ipcMessage = IPCMessage(message);
	if (ipcMessage.getTarget() == "DotNetManager")
	{
		passPluginMessage(sender,ipcMessage);
	}
	else
	{
		if (ipcMessage.getClass() == "event")
		{
			if (ipcMessage.getMethod() == "subscribe")
			{
				eventSubscriptionList.append(QPair<QString,QString>(sender,ipcMessage.getArgs()[0]));
			}
		}
		passCoreMessage(sender,ipcMessage);
	}
}
void DotNetManager::init()
{
	IWrapperClass::init();
	IWrapperClass::passCallbackPtr(&tmp,(void*)this);
	IWrapperClass::LoadPlugins(QString(QApplication::applicationDirPath()).append("/plugins").toStdString());
	QString pluginNames = QString::fromStdString(IWrapperClass::GetPluginNames());
	QString pluginFileNames = QString::fromStdString(IWrapperClass::GetPluginFileNames());
	if (!pluginNames.isEmpty())
	{
		QStringList tmpList = pluginNames.split(":");
		QStringList tmpFileList = pluginFileNames.split("|");
		if (tmpList.count() == 0)
		{
			pluginNameList << pluginNames;
			passCoreMessage("DotNetManager",IPCMessage("core","objectlist","addobject",QStringList() << pluginNames));
			//passCoreMessage("DotNetManager",IPCMessage("core","register","register",QStringList() << pluginFileNames << pluginNames));
		}
		else
		{	
			for (int i=0;i<tmpList.count();i++)
			{
				pluginNameList << tmpList[i];
				passCoreMessage("DotNetManager",IPCMessage("core","objectlist","addobject",QStringList() << tmpList[i]));
				//passCoreMessage("DotNetManager",IPCMessage("core","register","register",QStringList() << tmpFileList[i] << tmpList[i]));
			}
		}
	}
	
}
QString DotNetManager::getName()
{
	return "DotNetManager";
}
void DotNetManager::passPluginMessage(QString sender,QString message)
{

	
}
void DotNetManager::passPluginMessage(QString sender,IPCMessage message)
{
	QString strMessage = message.toString();
	passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << strMessage));
	if (message.getTarget() != "DotNetManager")
	{
		QString strMessage = message.toString();
		IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),strMessage.toStdString());
		passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << strMessage));
	}
	else
	{
		if (message.getClass() == "event")
		{
			if (message.getMethod() == "initstarted")
			{
				passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << "Attempting to init plugins..."));
				QMap<QString,QStringList> tmpPluginArgList;
				if (message.getArgs().count() > 0)
				{
					for (int i=0;i<message.getArgs().count();i++)
					{
						QString name = message.getArgs()[i];
						i++;
						QStringList tmpList;
						for (int j=1;j<=message.getArgs()[i].toInt();j++)
						{
							tmpList << message.getArgs()[j+i];
						}
						//i += messagegetArgs()[i].toInt();
						i += message.getArgs()[i].toInt();
						tmpPluginArgList[name] = tmpList;
					}
				}
				
				for (int i=0;i<pluginNameList.count();i++)
				{
					passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << "Initstarted: " + pluginNameList[i]));
					if (tmpPluginArgList.contains(pluginNameList[i]))
					{
						IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),IPCMessage(pluginNameList[i],"event","initstarted",tmpPluginArgList[pluginNameList[i]]).toString().toStdString());
					}
					else
					{
						IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),IPCMessage(pluginNameList[i],"event","initstarted",QStringList()).toString().toStdString());
					}
					passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << "Initstarted success: " + pluginNameList[i]));
				}
				emit passCoreMessage("DotNetManager",IPCMessage("core","event","initstarted",QStringList()));
			}
			else if (message.getMethod() == "initcomplete")
			{
				for (int i=0;i<pluginNameList.count();i++)
				{
					IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),IPCMessage(pluginNameList[i],"event","initcomplete",QStringList()).toString().toStdString());
				}
				emit passCoreMessage("DotNetManager",IPCMessage("core","event","initcomplete",QStringList()));
			}
			else if (message.getMethod() == "initclose")
			{
				for (int i=0;i<pluginNameList.count();i++)
				{
					passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << "Plugin Closing" << pluginNameList[i]));
					IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),IPCMessage(pluginNameList[i],"event","initclose",QStringList()).toString().toStdString());
					passCoreMessage("DotNetManager",IPCMessage("core","debug","debug",QStringList() << "Plugin Closed" << pluginNameList[i]));
				}
				emit passCoreMessage("DotNetManager",IPCMessage("core","event","initclose",QStringList()));
			}
			else if (message.getMethod() == "throw")
			{
				for (int i=0;i<eventSubscriptionList.count();i++)
				{
					if (eventSubscriptionList[i].second == message.getArgs()[0])
					{
						IWrapperClass::passPluginMessage(classPtr,sender.toStdString(),IPCMessage(eventSubscriptionList[i].first,"event","throw",message.getArgs()).toString().toStdString());
					}
				}
			}
		}
		else if (message.getClass() == "objectlist")
		{
			if (message.getMethod() == "addobject")
			{

			}
		}
		else if (message.getClass() == "admin")
		{
			if (message.getMethod() == "unloadplugin")
			{
				for (int i=0;i<pluginNameList.count();i++)
				{
					if (pluginNameList[i] == message.getArgs()[0])
					{
						//IWrapperClass::unloadPlugin
					}
				}
			}
		}
	}
}

Q_EXPORT_PLUGIN2(DotNetManagerPlugin, DotNetManager)
