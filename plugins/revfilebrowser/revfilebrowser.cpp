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

#include "revfilebrowser.h"
#include <QMessageBox>
#include <QDebug>
#include <QThread>




FileBrowser::FileBrowser()
{
	m_name = "RevFileBrowser";
}

void FileBrowser::init()
{
	currentDirectory=0;
	filterString="";
	/*filelist = new ComplexModel();
	QList<QList<QString> > data;
	QStringList roles;
	roles << "text";
	filelist->setRoles(roles);
	filelist->setData(data);*/
	QHash<int,QByteArray> roles;
	roles[0] = ".";
	roles[1] = "text";
	filelist = new StandardModel(roles);

}
QString FileBrowser::getName()
{
	return m_name;
}
void FileBrowser::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}
QStringList FileBrowser::getDirs(QString base)
{
	return QDir(base).entryList(QDir::NoDotAndDotDot | QDir::Dirs);
}

void FileBrowser::passPluginMessage(QString sender,IPCMessage message)
{
	QString IPC = message.toString();
	//qDebug() << IPC;
	if (message.getClass() == "window")
	{
	}
	else if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			//while(true);
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			//while(true);

			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
	if (message.getClass() == "dir")
	{
		if (message.getMethod() == "list")
		{
			if (message.getArgs().size() > 0)
			{
				StandardModel *tmplist=0;
				QList<QList<QString> > data;
				if (!m_modelMap.contains(message.getArgs()[1]))
				{
					//currentDir = message.getArgs()[0];

					QHash<int,QByteArray> roles;
					roles[0] = ".";
					roles[1] = "text";
					tmplist = new StandardModel(roles);
					m_modelList.append(message.getArgs()[1]);
					m_modelMap[message.getArgs()[1]] = tmplist;
				}
				else
				{
					tmplist = m_modelMap[message.getArgs()[1]];
					tmplist->clear();
				}
				QStringList dirs = getDirs(message.getArgs()[0]);
				for (int i=0;i<dirs.size();i++)
				{
					QStandardItem *item = new QStandardItem();
					item->setData(dirs[i],1);
					//QList<QString> list;
					//list.append(dirs[i]);
					//data.append(list);
					tmplist->appendRow(item);
				}
				//tmplist->setData(data);
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertylistmodelset" << message.getArgs()[1] << dirs));
				passCoreModel(message.getArgs()[1],tmplist);
			}
		}
	}
	if (message.getClass() == "file")
	{
		if (message.getMethod() == "list")
		{
			if (message.getArgs().count() > 0)
			{
				StandardModel *tmplist=0;
				QList<QList<QString> > data;
				if (!m_modelMap.contains(message.getArgs()[1]))
				{
					QHash<int,QByteArray> roles;
					roles[0] = ".";
					roles[1] = "text";
					tmplist = new StandardModel(roles);

					m_modelList.append(message.getArgs()[1]);
					m_modelMap[message.getArgs()[1]] = tmplist;
				}
				else
				{
					tmplist = m_modelMap[message.getArgs()[1]];
					tmplist->clear();
				}
				QStringList retValArgs = QDir(message.getArgs()[0]).entryList(QDir::NoDotAndDotDot | QDir::Files);
				for (int i=0;i<retValArgs.size();i++)
				{
					//QList<QString> list;
					//list.append(retValArgs[i]);
					//data.append(list);
					QStandardItem *item = new QStandardItem();
					item->setData(retValArgs[i],1);
					tmplist->appendRow(item);
				}
				//tmplist->setData(data);
				//qDebug () << retValArgs;
				/*if (filterString.isEmpty())
				{
					retValArgs << info.absoluteFilePath().mid(tmpDirString.length());
				}
				else
				{
					if (info.absoluteFilePath().contains(filterString))
					{
						retValArgs << info.absoluteFilePath().mid(tmpDirString.length());
					}
				}*/
				if (message.getArgs().count() > 2)
				{
					emit passCoreMessage(m_name,IPCMessage(message.getArgs()[1],"filelist","list",retValArgs));
				}
				else
				{
					//emit passCoreMessage(m_name,IPCMessage(sender,"filelist","list",retValArgs));
					//qDebug() << "Setting prop:" << message.getArgs()[1];
					//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertylistmodelset" << message.getArgs()[1] << retValArgs));
					passCoreModel(message.getArgs()[1],tmplist);
				}
				return;
			}
		}
		else if (message.getMethod() == "select")
		{
			if (message.getArgs()[0].endsWith("-DIR"))
			{
				passPluginMessage(sender,IPCMessage(m_name,"file","cd",message.getArgs()));
				return;
			}
			else
			{
				//passCoreMessage(m_name,IPCMessage(sender,"filelist","select",QStringList() << message.getArgs()[0]));
				passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:filelist" << "select" << message.getArgs()[0]));
				////event:listchange listname insert num filename artist album
				passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:listchange" <<  "selectedfilelist" << "insert" << "-1" << message.getArgs()[0] << message.getArgs()[0] << message.getArgs()[0]));
				fileList.append(currentDirectory->absolutePath().append("/").append(message.getArgs()[0]));
			}
		}
		else if (message.getMethod() == "setfilter")
		{
			if (message.getArgs().count() > 0)
			{
				filterString = message.getArgs()[0];
			}
		}
		else if (message.getMethod() == "remove")
		{
			if (message.getArgs().count() > 0)
			{
				int index = message.getArgs()[0].toInt();
				if (index != -1)
				{
					fileList.removeAt(index);
					passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:listchange" << "selectedfilelist" << "remove" << QString::number(index)));
				}
			}
		}
		else if (message.getMethod() == "cd")
		{
			if (message.getArgs().count() > 0)
			{
				if (currentDirectory)
				{
					emit passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << message.getArgs()[0].mid(0,message.getArgs()[0].length()-4)));
					currentDirectory->cd(message.getArgs()[0].mid(0,message.getArgs()[0].length()-4));
					passPluginMessage(sender,IPCMessage(m_name,"file","list",QStringList() << message.getArgs()[0].mid(0,message.getArgs()[0].length()-4) << message.getArgs()[1]));
					
					return;
				}
			}
		}
		else if (message.getMethod() == "sendlist")
		{
			if (message.getArgs().count() > 0)
			{
				QStringList args;
				for (int i=0;i<fileList.count();i++)
				{
					//emit passCoreMessage(m_name,IPCMessage(message.getArgs()[0],message.getArgs()[1],message.getArgs()[2],QStringList() << fileList.at(i)));
					args << fileList.at(i);
				}
				emit passCoreMessage(m_name,IPCMessage(message.getArgs()[0],message.getArgs()[1],message.getArgs()[2],args));
				fileList.clear();
				return;
				//addsong
			}
		}
		else if (message.getMethod() == "clear")
		{
			fileList.clear();
		}
	}
}
Q_EXPORT_PLUGIN2(FileBrowserPlugin, FileBrowser)
