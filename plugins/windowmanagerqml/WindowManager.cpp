/***************************************************************************
*   Copyright (C) 2010 by Michael Carpenter (malcom2073)                  *
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

#include "WindowManager.h"
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QRect>
#include <QSettings>
void WindowManager::init()
{
	qDebug() << "WindowManager::init()";
	qRegisterMetaType<IPCMessage>("IPCMessage");
	m_opengl = false;
	mainWindow = new Window(m_opengl);
	m_windowList.append(mainWindow);
	mainWindow->setGeometry(100,100,800,600);
	//connect(mainWindow,SIGNAL(playlistListSelected(QString,int)),this,SLOT(windowPlaylistListClicked(QString,int)));
	//connect(mainWindow,SIGNAL(currentPlaylistSelected(QString,int)),this,SLOT(windowCurrentPlaylistClicked(QString,int)));
	connect(mainWindow,SIGNAL(exitSignal()),this,SLOT(windowExitSignal()));
	connect(mainWindow,SIGNAL(passCoreMessageSignal(QString)),this,SLOT(windowPassCoreMessage(QString)));
	connect(mainWindow,SIGNAL(passCoreMessageSignal(QString,IPCMessage)),this,SIGNAL(passCoreMessage(QString,IPCMessage)));
}
QString WindowManager::getName()
{
	return "WindowManager";
}
void WindowManager::passPluginMessage(QString pluginName,QString message)
{

}
void WindowManager::passPluginMessage(QString sender,IPCMessage message)
{
	QString mclass = message.getClass();
	QString mmethod = message.getMethod();
	if (message.getTarget() == "core")
	{
		//passCoreMessage(sender,message);
	}
	else if (message.getTarget() == "WindowManager")
	{
		if (message.getClass() == "event")
		{
			if (message.getMethod() == "subscribe")
			{
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",message.getArgs()));
			}
			else if (message.getMethod() == "throw")
			{
				if (message.getArgs()[0] == "event:propertyset")
				{
					//message.getARgs()[1] - number of name/value pairs
					//message.getArgs()[2] - property name
					//message.getArgs()[3] - property value
					//qDebug() << message.toString();
					int count = message.getArgs()[1].toInt();
					for (int i=0;i<count;i++)
					{
						for (int j=0;j<m_windowList.size();j++)
						{
							m_windowList[j]->setQMLProperty(message.getArgs()[2+(i * 2)],message.getArgs()[3+(i * 2)]);
						}
					}
				}
				else if (message.getArgs()[0] == "event:propertylistset")
				{
					//qDebug() << "List:" << message.toString();
					QStringList list;
					for (int i=2;i<message.getArgs().size();i++)
					{
						list << message.getArgs()[i];
					}
					for (int j=0;j<m_windowList.size();j++)
					{
						m_windowList[j]->setList(message.getArgs()[1],list);
					}
				}
				else if (message.getArgs()[0] == "event:propertylistmodelset")
				{
					QStringList list;
					for (int i=2;i<message.getArgs().size();i++)
					{
						list << message.getArgs()[i];
					}
					for (int j=0;j<m_windowList.size();j++)
					{
						m_windowList[j]->setListModel(message.getArgs()[1],list);
					}
				}
				else if (message.getArgs()[0] == "event:propertylistmodeladd")
				{
					QStringList list;
					for (int i=2;i<message.getArgs().size();i++)
					{
						list << message.getArgs()[i];
					}
					for (int j=0;j<m_windowList.size();j++)
					{
						m_windowList[j]->addToListModel(message.getArgs()[1],list);
					}
				}
				else if (message.getArgs()[0] == "event:propertycomplexlistset")
				{
					//qDebug() << "Property Complex List Set";
					//Message format:
					//Title namecount, name1...nameN,rowcount,row1col1,row1colN,row2col1,row2colN... etc
					QString title = message.getArgs()[1];
					int namecount = message.getArgs()[2].toInt();
					//qDebug() << "Name count:" << namecount;
					QStringList names;
					QList<QList<QString> > data;
					for (int i=0;i<namecount;i++)
					{
						names.append(message.getArgs()[3+i]);
					}
					int rowcount = message.getArgs()[3+namecount].toInt();
					//qDebug() << "Row Count:" << rowcount;
					for (int j=0;j<rowcount;j++)
					{
						data.append(QList<QString>());
						for (int i=0;i<namecount;i++)
						{
							data[j].append(message.getArgs()[(3+namecount+1) + (j * namecount) + i]);
						}
					}
					mainWindow->setComplexList(title,names,data);
				}

				if (message.getArgs()[0] == "event:audiopositionupdate")
				{
					//mainWindow->setTotalTimeText(message.getArgs()[1]);
					//mainWindow->setTimeElapsedText(message.getArgs()[2]);
					//mainWindow->setTimeRemainingText(message.getArgs()[3]);
				}
				else if (message.getArgs()[0] == "event:newmedialist")
				{
					passCoreMessage("WindowManager",IPCMessage("MediaLibrary","playlist","getlist",QStringList()));
				}
				else if (message.getArgs()[0] == "event:playlistloaded")
				{
					//passCoreMessage("WindowManager",IPCMessage("MediaLibrary","playlist","getsongfilenames",QStringList()));
				}
				else if (message.getArgs()[0] == "event:mediachange")
				{
					//mainWindow->mediaChange(message.getArgs()[1],message.getArgs()[2].toInt());
				}
			}
			else if (message.getMethod() == "initstarted")
			{

				qDebug() << "WindowManager.cpp initcomplete" << message.getArgs();
				for (int i=0;i<message.getArgs().size();i++)
				{
					QString key = message.getArgs()[i];
					QString value = message.getArgs()[i+1];
					QString type = message.getArgs()[i+2];
					QString id = message.getArgs()[i+3];
					if (key == "opengl")
					{
						if (value == "true")
						{
							m_opengl = true;
						}
						else if (value == "false")
						{
							m_opengl = false;
						}
					}
					i+=3;
				}
				qDebug() << __FILE__ << __LINE__ << "Setting OPENGL Mode to" << m_opengl;
				mainWindow->setOpengl(m_opengl);

				///passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:playlistloaded"));
				//passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:newmedialist"));
				//passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:mediachange"));
				//passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:audiopositionupdate"));
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:propertyset"));
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:propertylistset"));
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:propertycomplexlistset"));
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:propertylistmodel"));
				passCoreMessage("WindowManager",IPCMessage("core","event","subscribe",QStringList() << "event:propertylistmodelset"));

				passCoreMessage("WindowManager",IPCMessage("core","event","initstarted",QStringList()));

			}
			else if (message.getMethod() == "initcomplete")
			{
				QFile myFile("QML/Skin.xml");
				myFile.open(QIODevice::ReadOnly);
				QByteArray myArray = myFile.readAll();
				myFile.close();
				QXmlStreamReader xml;
				xml.addData(myArray);

				bool relativeLayout = false;
				while (!xml.atEnd())
				{
					xml.readNext();
					if ((xml.name() == "geometry") && (xml.isStartElement()))
					{
					}
					if ((xml.name() == "pagelist") && (xml.isStartElement()))
					{
						QString pagename = xml.attributes().value("name").toString();
						QString pagefilename = xml.attributes().value("filename").toString();
						QStringList pagelist;
						QStringList filelist;
						xml.readNext();
						while (xml.name() != "pagelist")
						{
							if (xml.name() == "page" && xml.isStartElement())
							{
								QString name = xml.attributes().value("name").toString();
								QString file = xml.attributes().value("file").toString();
								pagelist << name;
								filelist << file;
								//list << name;
							}
							xml.readNext();
						}
						mainWindow->setList(pagename,pagelist);
						mainWindow->setList(pagefilename,filelist);
					}
				}
				mainWindow->load();
				mainWindow->show();
				passCoreMessage("WindowManager",IPCMessage("core","event","initcomplete",QStringList()));
			}
			else if (message.getMethod() == "initclose")
			{
				passCoreMessage("WindowManager",IPCMessage("core","event","initclose",QStringList()));
			}

		}
		else if (message.getClass() == "window")
		{
			if (message.getMethod() == "showlast")
			{
			}
			else if (message.getMethod() == "getwinidlist")
			{
			}
			else if (message.getMethod() == "showconfirmation")
			{
			}
			else if (message.getMethod() == "rrcmd")
			{
			}
			else if (message.getMethod() == "embed")
			{
			}
			else if (message.getMethod() == "register")
			{
				//utilityAppModel
			}
			else if (message.getMethod() == "settings")
			{
				qDebug() << "GOT SETTING HERE!" << message.getArgs().size();
				for (int i=0;i<message.getArgs().size();i++)
				{
					QString name = message.getArgs()[i];
					mainWindow->createSettingsPage(name);
					i++;
					int count = message.getArgs()[i].toInt();
					i++;
					for (int j=0;j<count;j++)
					{
						QString key = message.getArgs()[i++];
						QString value = message.getArgs()[i++];
						QString type = message.getArgs()[i++];
						int id = message.getArgs()[i++].toInt();
						mainWindow->addSetting(name,key,value,type,id);
					}
					i--;
				}
			}
		}
		else if (message.getClass() == "playlist")
		{
			if (message.getMethod() == "list")
			{
				//QMessageBox::information(0,"Test","Playlist called!" + QString::number(message.getArgs().size()));
				//mainWindow->setPlaylistList(message.getArgs());
				//for (int i=0;i<message.getArgs().size();i++)
				//{
					///mainWindow->m_playList.append(message.getArgs()[i]);
//					qDebug() << "PLAYLIST:" << message.getArgs()[i];
				//}
			}
			else if (message.getMethod() == "songlist")
			{
				QList<QString> songList;
				QString playname = message.getArgs()[0];
				QString basename = message.getArgs()[1];
				for (int i=2;i<message.getArgs().size();i++)
				{
					QString file = message.getArgs()[i];
					i++;
					QString album = message.getArgs()[i];
					i++;
					QString artist = message.getArgs()[i];
					i++;
					QString title = message.getArgs()[i];
					i++;
					QString track = message.getArgs()[i];
					songList.append(artist + " - " + title);
				}
				//mainWindow->setCurretPlaylist(playname,songList);
			}
		}
		//return;
	}
	else
	{
	}
}
void WindowManager::windowPassCoreMessage(QString message)
{
	IPCMessage msg(message);
	emit passCoreMessage("Window",msg);
}

void WindowManager::windowExitSignal()
{
	passCoreMessage("WindowManager",IPCMessage("core","admin","close",QStringList()));
}

void WindowManager::windowCurrentPlaylistClicked(QString item, int index)
{
	passCoreMessage("WindowManager",IPCMessage("MediaEngine","media","play",QStringList() << QString::number(index)));
}

void WindowManager::windowPlaylistListClicked(QString item,int index)
{
	passCoreMessage("WindowManager",IPCMessage("MediaLibrary","playlist","set",QStringList() << item));
}

Q_EXPORT_PLUGIN2(WindowManagerQMLPlugin, WindowManager)
