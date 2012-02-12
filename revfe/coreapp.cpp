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

#include "coreapp.h"
#include <QMessageBox>
#include <QtGlobal>
#include <QAbstractEventDispatcher>
#include <QXmlStreamReader>
#include <QDebug>
#include <QSessionManager>
#include <QClipboard>
#include <QFile>
#include <QMetaObject>
#include <QGraphicsObject>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <PowrProf.h>
#endif


CoreApp::CoreApp()
{
	/*window = new QDeclarativeView();
	window->show();
	window->rootContext()->setContextProperty("window",this);
	window->rootContext()->setContextProperty("propertyMap",&propertyMap);
	window->setGeometry(-1,-1,800,600);*/

	//window->setSource(QUrl::fromLocalFile("QML/main.qml"));


	QCoreApplication::setApplicationName("CarPal");

	//Absolutly STUPID workaround, required when KDE's Qt is in use.

	eng = new QScriptEngine(this);
	connect(QApplication::instance(),SIGNAL(commitDataRequest(QSessionManager &)),this,SLOT(dataRequest(QSessionManager &))); 
	initsThrown = false;
	qRegisterMetaType<IPCMessage>("IPCMessage");
	//QApplication::setApplicationName("MediaEngine");
	debugWindow = new DebugWindow();	
	//debugWindow->show();
	//debugWindow->debug_output("Debug Display");


	QDir settingsDir;
	if (QFile::exists(QApplication::applicationDirPath().append("/plugins")))
	{
		settingsDir = QDir(QApplication::applicationDirPath().append("/plugins"));
	}
	else if (QFile::exists("/usr/share/revfe/plugins"))
	{
		settingsDir = QDir("/usr/share/revfe/plugins");
	}
	else if (QFile::exists("~/.revfe/plugins"))
	{
		settingsDir = QDir("!/.revfe/plugins");
	}
	else
	{
		qDebug() << "Core.cpp: Error: You have no settings folder. RevFE cannot continue";
		exit(-1);
	}
	qDebug() << "Settings directory set to:" << settingsDir;

	foreach (QString fileName, settingsDir.entryList(QDir::Files))
	{
		//qDebug() << "Settings file:" << fileName;
		if (fileName.endsWith(".xml"))
		{
			QFile myFile(settingsDir.absoluteFilePath(fileName));
			myFile.open(QIODevice::ReadOnly);
			QByteArray myArray = myFile.readAll();
			myFile.close();
			QXmlStreamReader xml(myArray);
			QString pluginname = "";
			while (!xml.atEnd())
			{
				if (xml.name() == "settings" && xml.isStartElement())
				{
					pluginname = xml.attributes().value("plugin").toString();
					//qDebug() << "Core.cpp: Settings file found for:" << pluginname;
					settings.m_keyList[pluginname] = QList<QString>();
					settings.m_pluginList.append(pluginname);
				}
				else if (xml.name() == "multisetting" && xml.isStartElement())
				{
					if (!settings.m_multiSettingsMap.contains(pluginname))
					{
						settings.m_multiSettingsMap[pluginname] = QMap<QString,QList<SettingValue> >();
					}
					QString key = xml.attributes().value("key").toString();
					QString value = xml.attributes().value("value").toString();
					QString type = xml.attributes().value("type").toString();
					int id = xml.attributes().value("id").toString().toInt();
					if (value.contains("~"))
					{
						value = value.replace("~",getenv("HOME"));
					}
					//qDebug() << "Multi-Value:" << key << value << type;
					SettingValue val;
					val.key = key;
					val.value = value;
					val.type = type;
					val.id = id;
					settings.m_multiSettingsMap[pluginname][key].append(val);
					settings.m_keyList[pluginname].append(key);
				}
				if (xml.name() == "setting" && xml.isStartElement())
				{
					if (!settings.m_settingsMap.contains(pluginname))
					{
						settings.m_settingsMap[pluginname] = QMap<QString,SettingValue>();
					}
					QString key = xml.attributes().value("key").toString();
					QString value = xml.attributes().value("value").toString();
					QString type = xml.attributes().value("type").toString();
					if (value.contains("~"))
					{
						value = value.replace("~",getenv("HOME"));
					}
					//qDebug() << "Value:" << key << value << type;
					SettingValue val;
					val.key = key;
					val.value = value;
					val.type = type;
					val.id = 0;
					settings.m_settingsMap[pluginname][key] = val;
					settings.m_keyList[pluginname].append(key);
				}
				xml.readNext();
			}
		}
	}





	QXmlStreamReader xml;
	QString settingFileStr;
	if (QFile::exists("settings.xml"))
	{
		settingFileStr = "settings.xml";
	}
	else if (QFile::exists("~/.revfe/settings.xml"))
	{
		settingFileStr = "~/.revfe/settings.xml";
	}
	else if (QFile::exists("/usr/share/revfe/settings.xml"))
	{
		settingFileStr = "/usr/share/revfe/settings.xml";
	}
	else
	{
		qDebug() << "Core.cpp: Error! No settings.xml file found. This file should have installed with the package in /usr/share/revfe";
	}
	QFile myFile(settingFileStr);
	myFile.open(QIODevice::ReadOnly);
	QByteArray myArray = myFile.readAll();
	myFile.close();
	xml.addData(myArray);
	while (!xml.atEnd())
	{
		xml.readNext();
		if (xml.isStartElement())
		{
			if ((xml.name() == "settings") && (xml.isStartElement()))
			{
				QString debugString = xml.attributes().value("debug").toString();
				if (debugString == "true")
				{
					settings.debug = true;
					//debugWindow->enableDebug();
				}
				else
				{
					settings.debug = false;
				}
			}
			if ((xml.name() == "plugin") && (xml.isStartElement()))
			{
				QString pluginName = xml.attributes().value("name").toString();
				//qDebug() << "Plugin settings: " << pluginName;
				if (!settings.m_settingsMap.contains(pluginName) && !settings.m_multiSettingsMap.contains(pluginName))
				{
					qDebug() << "Core.cpp: Settings found for plugin:" << pluginName << "but no plugin definition file was found";
					qDebug() << "Core.cpp: This is not yet supported, and constitutes a fatal error. Now exiting";
					exit(-1);
				}
				QStringList pluginattributes;
				while ((xml.name() != "plugin") || (!xml.isEndElement()))
				{
					xml.readNext();
					if ((xml.name() == "setting") && (xml.isStartElement()))
					{
						QXmlStreamAttributes a = xml.attributes();
						for (int i=0;i<a.size();i++)
						{
							if (a[i].name() == "id")
							{
								continue;
							}
							if (settings.m_settingsMap[pluginName].contains(a[i].name().toString()))
							{
								QString text = QString(a[i].value().toString());
								//qDebug() << "Single" << text;
								if (text.contains("~"))
								{
									text = text.replace("~",getenv("HOME"));
									//qDebug() << "Replaced ~ with home directory: " << text << " :: " << getenv("HOME");
								}
								settings.m_settingsMap[pluginName][a[i].name().toString()].value = text;
							}
							else
							{
								qDebug() << "Core.cpp: Setting found for plugin:" << pluginName << "That does not exist";
								qDebug() << "Core.cpp: Name:" << a[i].name();
								qDebug() << "Core.cpp: This is not yet supported, and constitutes a fatal error. Now exiting";
								exit(-1);
							}
						}
					}
					if ((xml.name() == "multisetting") && xml.isStartElement())
					{

						QXmlStreamAttributes a = xml.attributes();
						QString name = "";
						QString val = "";
						int id = -1;
						for (int i=0;i<a.size();i++)
						{
							for (int j=0;j<a.size();j++)
							{
								if (a[j].name() == "id")
								{
									id = a[j].value().toString().toInt();
									break;
								}
							}
							if (a[i].name() == "id")
							{
								//id = a[i].value().toString().toInt();
								continue;
							}
							else
							{
								name = a[i].name().toString();
								val = a[i].value().toString();
							}
							if (settings.m_multiSettingsMap[pluginName].contains(name))
							{
								bool found = false;
								for (int j=0;j<settings.m_multiSettingsMap[pluginName][name].size();j++)
								{
									if (settings.m_multiSettingsMap[pluginName][name][j].id == id)
									{
										found = true;
										QString text = QString(val);
										//qDebug() << "Multi" << text;
										if (text.contains("~"))
										{
											text = text.replace("~",getenv("HOME"));
											//qDebug() << "Replaced ~ with home directory: " << text << " :: " << getenv("HOME");
										}
										settings.m_multiSettingsMap[pluginName][name][j].value = text;
									}
								}
								if (!found)
								{
									//This is a multi-setting, so it does not have to have a pre-defined setting
									//Use the type from the previous setting
									QString text = QString(val);
									//qDebug() << "New Multi" << text;
									if (text.contains("~"))
									{
										text = text.replace("~",getenv("HOME"));
										//qDebug() << "Replaced ~ with home directory: " << text << " :: " << getenv("HOME");
									}
									SettingValue set;
									set.key = name;
									set.value = val;
									set.type = settings.m_multiSettingsMap[pluginName][name][0].type;
									set.id = settings.m_multiSettingsMap[pluginName][name][settings.m_multiSettingsMap[pluginName][name].size()-1].id + 1;
									settings.m_multiSettingsMap[pluginName][name].append(set);
								}
							}
						}

					}
				}
				//qDebug() << "Total single settings for " << pluginName << settings.m_settingsMap[pluginName].count();
				//qDebug() << "Total multi settings for " << pluginName << settings.m_multiSettingsMap[pluginName].count();
			}
		}
	}

	//QDir pluginsDir = QDir(QApplication::applicationDirPath());
	QDir pluginsDir;
	if (QFile::exists(QApplication::applicationDirPath().append("/plugins")))
	{
		pluginsDir = QDir(QApplication::applicationDirPath().append("/plugins"));
	}
	else if (QFile::exists("/usr/share/revfe/plugins"))
	{
		pluginsDir = QDir("/usr/share/revfe/plugins");
	}
	else if (QFile::exists("~/.revfe/plugins"))
	{
		pluginsDir = QDir("~/.revfe/plugins");
	}
	else
	{
		qDebug() << "Core.cpp: Warning! You have no plugins folder! This should have been installed at /usr/share/revfe/plugins by the installer!!";
		qDebug() << "Core.cpp: Since no plugins means no features, now exiting";
		exit(-1);
	}
	//pluginsDir.cd("plugins");
	foreach (QString fileName, pluginsDir.entryList(QDir::Files))
	{
		#ifdef Q_OS_WIN32
		if (fileName.endsWith(".dll"))
		#endif
		#ifdef Q_OS_LINUX
		if (fileName.endsWith(".so"))
		#endif
		#ifdef Q_OS_MAC
		if (fileName.endsWith(".dylib"))
		#endif
		{
			QString fullFilePath = pluginsDir.absoluteFilePath(fileName);
			//debugWindow->debug_output("Loading plugin: " + fileName);
			qDebug() << "Core.cpp: Loading plugin class: " << fileName;
			PluginClass *tmpClass = new PluginClass(fullFilePath);
			connect(tmpClass,SIGNAL(passCoreMessage(QString,IPCMessage)),this,SLOT(passedCoreMessage(QString,IPCMessage)));
			connect(tmpClass,SIGNAL(passCoreMessageBlocking(QString,IPCMessage)),this,SLOT(passedCoreMessageBlocking(QString,IPCMessage)),Qt::DirectConnection);
			connect(tmpClass,SIGNAL(pluginLoadFail(QString)),this,SLOT(pluginLoadFail(QString)));
			connect(tmpClass,SIGNAL(passCoreModel(QString,QObject*)),this,SLOT(passedCoreModel(QString,QObject*)));
			pluginList.append(tmpClass);
/*
			//Need to start windowmanager here
			#ifdef Q_OS_WIN32
			if ((fileName.endsWith("WindowManagerPlugin.dll")) || fileName.endsWith("WindowManagerQMLPlugin.dll") || (fileName.endsWith("DotNetManagerPlugin.dll")))
			#endif
			#ifdef Q_OS_LINUX
			if (fileName.endsWith("libWindowManagerQMLPlugin.so") || fileName.endsWith("libWindowManagerPlugin.so"))
			#endif
			#ifdef Q_OS_MAC
			if ((fileName.endsWith("libWindowManagerPlugin.dylib")) || (fileName.endsWith("libMediaEnginePlugin.dylib")))
			#endif
			{
				tmpClass->startUnThreaded();
			}
*/
			tmpClass->startThreaded();
		}
	}
	/*
	for (int i=0;i<pluginList.count();i++)
	{
		QString fileName = pluginList[i]->getFileName();
		//debugWindow->debug_output("Starting plugin: " + fileName);
		//qDebug() << "Starting plugin class : " << fileName;
		#ifdef Q_OS_WIN32
		if ((fileName.endsWith("WindowManagerPlugin.dll")) || fileName.endsWith("WindowManagerQMLPlugin.dll") || (fileName.endsWith("DotNetManagerPlugin.dll")))
		#endif
		#ifdef Q_OS_LINUX
		if (fileName.endsWith("libWindowManagerQMLPlugin.so") || fileName.endsWith("libWindowManagerPlugin.so"))
		#endif
		#ifdef Q_OS_MAC
		if ((fileName.endsWith("libWindowManagerPlugin.dylib")) || (fileName.endsWith("libMediaEnginePlugin.dylib")))
		#endif
		{
			//pluginList[i]->startUnThreaded();
		}
		else
		{
			pluginList[i]->startThreaded();
		}
	}*/
	QFile myskinFile("QML/Skin.xml");
	myskinFile.open(QIODevice::ReadOnly);
	QByteArray myskinArray = myskinFile.readAll();
	myskinFile.close();
	QXmlStreamReader skinxml;
	skinxml.addData(myskinArray);

	bool relativeLayout = false;
	QStringList totalApps;
	while (!skinxml.atEnd())
	{
		skinxml.readNext();
		if ((skinxml.name() == "geometry") && (skinxml.isStartElement()))
		{
		}
		if ((skinxml.name() == "pagelist") && (skinxml.isStartElement()))
		{
			//m_windowList
			QDeclarativeView *tmpWindow = 0;
			QString url;
			if (skinxml.attributes().value("type").toString() == "real")
			{
				tmpWindow = new QDeclarativeView();
				m_windowList.append(tmpWindow);
				tmpWindow->rootContext()->setContextProperty("window",this);
				tmpWindow->rootContext()->setContextProperty("propertyMap",&propertyMap);
				int x = skinxml.attributes().value("x").toString().toInt();
				int y = skinxml.attributes().value("y").toString().toInt();
				int w = skinxml.attributes().value("width").toString().toInt();
				int h = skinxml.attributes().value("height").toString().toInt();
				url = skinxml.attributes().value("main").toString();
				tmpWindow->setGeometry(x,y,w,h);
				qDebug() << "Source:" << tmpWindow->source();
			}
			QString pagename = skinxml.attributes().value("name").toString();
			QString pagefilename = skinxml.attributes().value("filename").toString();
			QStringList pagelist;
			QStringList filelist;
			skinxml.readNext();
			while (skinxml.name() != "pagelist")
			{
				if (skinxml.name() == "page" && skinxml.isStartElement())
				{
					QString name = skinxml.attributes().value("name").toString();
					QString file = skinxml.attributes().value("file").toString();
					pagelist << name;
					filelist << file;
					totalApps << name;
					//list << name;
				}
				skinxml.readNext();
			}
			//mainWindow->setList(pagename,pagelist);
			//mainWindow->setList(pagefilename,filelist);
			propertyMap.setProperty(pagename.toAscii(),QVariant(pagelist));
			propertyMap.setProperty(pagefilename.toAscii(),QVariant(filelist));
			if (tmpWindow)
			{
				tmpWindow->setSource("QML/" + url);
				tmpWindow->show();
			}
		}
	}
	propertyMap.setProperty("totalapps",QVariant(totalApps));
	//if (!QMetaObject::invokeMethod(m_windowList[0]->rootObject(),"addSettingsPage",Q_ARG(QVariant,QVariant(name))))
	for (int i=0;i<m_windowList.size();i++)
	{
		QMetaObject::invokeMethod(m_windowList[i]->rootObject(),"loadComplete");
	}
}
void CoreApp::passedCoreGUIItem(QObject *item)
{

}

void CoreApp::passedCoreModel(QString name, QObject *model)
{
	for (int i=0;i<m_windowList.size();i++)
	{
		//new ModelTest((QAbstractItemModel*)model,this);
		m_windowList[i]->rootContext()->setContextProperty(name,model);
		qDebug() << "Added model:" << name;
	}
}

void CoreApp::passMessage(QString message)
{
	qDebug() << "Window Thread:" << QThread::currentThread();
	qDebug() << "Window message:" << message;
	//emit passCoreMessageSignal(message);
	IPCMessage msg(message);
	passedCoreMessage("Window",msg);

}
/*void CoreApp::setQMLProperty(QString name,QString value)
{
	//qDebug() << "Set QML Property:" << name << value;
	propertyMap.setProperty(name.toAscii(),QVariant(value));
}*/

void CoreApp::saveSettings()
{
	if (QFile::exists("settings.xml"))
	{
		QFile::remove("settings.xml");
	}
	QXmlStreamWriter xml;
	QFile save("settings.xml");
	save.open(QIODevice::ReadWrite);
	xml.setDevice(&save);

	xml.writeStartElement("settings");
	xml.writeAttribute("debug",(settings.debug ? "true" : "false"));
	for (int i=0;i<settings.m_pluginList.size();i++)
	{
		xml.writeCharacters("\n\t");
		xml.writeStartElement("plugin");
		xml.writeAttribute("name",settings.m_pluginList[i]);
		for (int j=0;j<settings.m_keyList[settings.m_pluginList[i]].size();j++)
		{
			if (settings.m_settingsMap[settings.m_pluginList[i]].contains(settings.m_keyList[settings.m_pluginList[i]][j]))
			{
				xml.writeCharacters("\n\t\t");
				xml.writeStartElement("setting");
				QString key = settings.m_settingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]].key;
				QString value = settings.m_settingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]].value;
				//qDebug() << "Single:" << key << value << "0";
				xml.writeAttribute(key,value);
				xml.writeAttribute("id","0");
				xml.writeEndElement();
			}
			else if (settings.m_multiSettingsMap[settings.m_pluginList[i]].contains(settings.m_keyList[settings.m_pluginList[i]][j]))
			{
				qDebug() << settings.m_pluginList[i] << settings.m_keyList[settings.m_pluginList[i]] << settings.m_multiSettingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]].size();
				for (int k=0;k<settings.m_multiSettingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]].size();k++)
				{
					xml.writeCharacters("\n\t\t");
					xml.writeStartElement("multisetting");
					QString key = settings.m_multiSettingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]][k].key;
					QString value = settings.m_multiSettingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]][k].value;
					QString id = QString::number(settings.m_multiSettingsMap[settings.m_pluginList[i]][settings.m_keyList[settings.m_pluginList[i]][j]][k].id);
					//qDebug() << "Multi:" << settings.m_pluginList[i] << settings.m_keyList[settings.m_pluginList[i]][j] << key << value << id;
					xml.writeAttribute(key,value);
					xml.writeAttribute("id",id);
					xml.writeEndElement();
				}
			}

		}
		xml.writeCharacters("\n\t");
		xml.writeEndElement();
	}
	xml.writeCharacters("\n");
	xml.writeEndElement();
	save.close();
	qDebug() << "Core.cpp: Settings saved";
}

void CoreApp::sendInitStarted()
{
	qDebug() << "Core.cpp: Sending initstarted to all plugins";
	QTimer *timer = new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(initStartedTimeTick()));
	timer->start(1000);
	m_initStartedTime = QDateTime::currentDateTime();
	for (int i=0;i<pluginList.count();i++)
	{
		bool initsthere = false;
		QStringList inits;
		if (settings.m_settingsMap.contains(pluginList[i]->getName()))
		{
			initsthere = true;
			for (int j=0;j<settings.m_keyList[pluginList[i]->getName()].size();j++)
			{
				if (settings.m_settingsMap[pluginList[i]->getName()].contains(settings.m_keyList[pluginList[i]->getName()][j]))
				{
					QString key = "key=" + settings.m_settingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]].key;
					QString value = "value=" + settings.m_settingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]].value;
					QString type = "type=" + settings.m_settingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]].type;
					int id = settings.m_settingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]].id;
					inits.append(key);
					inits.append(value);
					inits.append(type);
					inits.append("id=" + QString::number(id));
					//qDebug() << "Single setting added for" << pluginList[i]->getName() << key << value << type;
				}
			}

		}
		if (settings.m_multiSettingsMap.contains(pluginList[i]->getName()))
		{
			initsthere = true;
			for (int j=0;j<settings.m_keyList[pluginList[i]->getName()].size();j++)
			{
				if (settings.m_multiSettingsMap[pluginList[i]->getName()].contains(settings.m_keyList[pluginList[i]->getName()][j]))
				{
					for (int k=0;k<settings.m_multiSettingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]].size();k++)
					{
						QString key = "key=" + settings.m_multiSettingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]][k].key;
						QString value = "value=" + settings.m_multiSettingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]][k].value;
						QString type = "type=" + settings.m_multiSettingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]][k].type;
						int id = settings.m_multiSettingsMap[pluginList[i]->getName()][settings.m_keyList[pluginList[i]->getName()][j]][k].id;
						inits.append(key);
						inits.append(value);
						inits.append(type);
						inits.append("id=" + QString::number(id));
						//qDebug() << "Multi setting added for" << pluginList[i]->getName() << key << value << type;
					}
				}
			}

		}
		if (initsthere)
		{
			//debugWindow->debug_output("Init sent with settings: " + pluginList[i]->getName());
			qDebug() << "Core.cpp: Sending initstarted with settings to: " << pluginList[i]->getName();
			pluginList[i]->passPluginMessage("core",IPCMessage(pluginList[i]->getName(),"event","initstarted",inits));
		}
		else
		{
			//debugWindow->debug_output("Init sent: " + pluginList[i]->getName());
			qDebug() << "Core.cpp: Sending initstarted to: " << pluginList[i]->getName();
			pluginList[i]->passPluginMessage("core",IPCMessage(pluginList[i]->getName(),"event","initstarted",QStringList()));
		}
	}
}
void CoreApp::initStartedTimeTick()
{
	if (m_initStartedTime.addSecs(10) < QDateTime::currentDateTime())
	{
		//qDebug() << "10 seconds passed, killing unfinished plugins";
		//It's been 10 seconds, let's kill a plugin.
		bool all = true;
		for (int i=0;i<pluginList.size();i++)
		{
			if (!m_initStartedMap.contains(pluginList[i]->getName()))
			{
				all = false;
				qDebug() << "Core.cpp:" << pluginList[i]->getName() << "has not returned from initstarted in 10 seconds. Killing it off";
				delete pluginList[i];
				pluginList.removeAt(i);
			}
			else if (!m_initStartedMap[pluginList[i]->getName()])
			{
				all = false;
				qDebug() << "Core.cpp:" << pluginList[i]->getName() << "has not returned from initstarted in 10 seconds. Killing it off";
				delete pluginList[i];
				pluginList.removeAt(i);
			}
		}
		QTimer *timer = qobject_cast<QTimer*>(sender());
		timer->stop();
		timer->deleteLater();
		if (!all)
		{
			sendInitComplete();
		}
	}
}

void CoreApp::initCompleteTimerTick()
{
	//qDebug() << "Waiting on plugins for initcomplete" << m_initStartedTime << QDateTime::currentDateTime();
	if (m_initCompleteTime.addSecs(10) < QDateTime::currentDateTime())
	{
		//qDebug() << "10 seconds passed, killing unfinished plugins";
		//It's been 10 seconds, let's kill a plugin.
		for (int i=0;i<pluginList.size();i++)
		{
			if (!m_initCompleteMap.contains(pluginList[i]->getName()))
			{
				qDebug() << "Core.cpp:" << pluginList[i]->getName() << "has not returned from initcomplete in 10 seconds. Killing it off";
				delete pluginList[i];
				pluginList.removeAt(i);
			}
			else if (!m_initCompleteMap[pluginList[i]->getName()])
			{
				qDebug() << "Core.cpp:" << pluginList[i]->getName() << "has not returned from initcomplete in 10 seconds. Killing it off";
				delete pluginList[i];
				pluginList.removeAt(i);
			}
		}
		QTimer *timer = qobject_cast<QTimer*>(sender());
		timer->stop();
		timer->deleteLater();
	}
}

void CoreApp::sendInitComplete()
{
	m_initCompleteTime = QDateTime::currentDateTime();
	QTimer *timer = new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(initCompleteTimerTick()));
	timer->start(1000);
	for (int i=0;i<pluginList.count();i++)
	{
		//pluginList[i]->passPluginMessageBlocking("core",IPCMessage(pluginList[i]->getName(),"event","initcomplete",QStringList()));
		qDebug() << "Core.cpp: Sending initcomplete to: " << pluginList[i]->getName();
		pluginList[i]->passPluginMessage("core",IPCMessage(pluginList[i]->getName(),"event","initcomplete",QStringList()));
	}
}

void CoreApp::pluginLoadFail(QString pluginfilename)
{
	//debugWindow->debug_output("Plugin FAILED to load: " + pluginfilename);
	qDebug() << "Core.cpp: Plugin FAILED to load:" << pluginfilename;
	for (int i=0;i<pluginList.count();i++)
	{
		if (pluginList[i]->getFileName() == pluginfilename)
		{
			//debugWindow->debug_output("Removed plugin: " + pluginfilename);
			pluginList.removeAt(i);
			//return;
		}
	}
	bool all = true;
	for (int i=0;i<pluginList.count();i++)
	{
		if (!pluginList[i]->IsRegistered())
		{
			//debugWindow->debug_output("Plugin not registered!: " + pluginList[i]->getName());
			all = false;
		}
	}
	if ((all))
	{
		//debugWindow->debug_output("All plugins registered with a failure, sending inits");
		qDebug() << "Core.cpp: All plugins registered with a failure. Sending inits to the rest";
		//initsThrown =  true;
		sendInitStarted();
		//qDebug() << "All inits completed";
	}
}
void CoreApp::passedCoreMessage(QString sender,IPCMessage message)
{
	passedCoreMessage(sender,message,false);
}
void CoreApp::passedCoreMessageBlocking(QString sender,IPCMessage message)
{
	passedCoreMessage(sender,message,true);
}
void CoreApp::passedCoreMessage(QString sender,IPCMessage message,bool blocking)
{
	/*
	if (!(message.getClass() == "event") || (message.getMethod() == "subscribe"))
	{
		debugWindow->debug_output(QString("IPC Received"));
		debugWindow->debug_output("Sender: " + sender);
		debugWindow->debug_output("Target: " +message.getTarget());
		debugWindow->debug_output("Class: " + message.getClass());
		debugWindow->debug_output("Method: " + message.getMethod());
		for (int i=0;i<message.getArgs().count();i++)
		{
			debugWindow->debug_output(QString("Arg").append(QString::number(i)).append(": ").append(message.getArgs()[i]));
		}
	}
	else
	{
		if (message.getMethod() == "throw")
		{
			//qDebug() << "Message Throw:" << message.toString();
			if (message.getArgs().count() > 0)
			{
				if ((message.getArgs()[0] != "event:currentplaylistupdate") && (message.getArgs()[0] != "event:audiopositionupdate"))
				{
					bool good = true;
					if (message.getArgs().count() > 1)
					{
						if (((message.getArgs()[1] == "%TT-CURRENTTIME%") || (message.getArgs()[1] == "%TT-TOTALTIME%") || (message.getArgs()[1] == "%TT-CURRENTTIMEPERCENT%")) || (message.getArgs()[0].contains("obdinfo")))
						{
							good = false;
						}
					}
					if (good)
					{
						debugWindow->debug_output(QString("IPC Received"));
						debugWindow->debug_output("Sender: " + sender);
						debugWindow->debug_output("Target: " +message.getTarget());
						debugWindow->debug_output("Class: " + message.getClass());
						debugWindow->debug_output("Method: " + message.getMethod());
						for (int i=0;i<message.getArgs().count();i++)
						{
							debugWindow->debug_output(QString("Arg").append(QString::number(i)).append(": ").append(message.getArgs()[i]));
						}
					}
				}
			}
		}
	}
	*/
	//Parse messages here
	if (message.getTarget() == "core")
	{
		//Message is meant for core
		if (message.getClass() == "debug")
		{
			if (message.getArgs().count() > 0)
			{
				QString args = "";
				for (int i=0;i<message.getArgs().count();i++)
				{
					args.append(message.getArgs().at(i) + "::");
				}
				debugWindow->debug_output(args);
				debugWindow->debug_output("END");
			}
		}
		else if (message.getClass() == "event")
		{
			if (message.getMethod() == "subscribe")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == sender)
					{
						eventHandlerList.append(QPair<QString,QObject*>(message.getArgs()[0],pluginList[i]));
					}
				}
				for (int i=0;i<extraHandlerList.count();i++)
				{
					if (extraHandlerList[i].first == sender)
					{
						eventHandlerList.append(QPair<QString,QObject*>(message.getArgs()[0],extraHandlerList[i].second));
					}
				}
			}
			else if (message.getMethod() == "initstarted")
			{
				qDebug() << "Core.cpp: Init started finished:" << sender;
				m_initStartedMap[sender] = true;
				bool all = true;
				for (int i=0;i<pluginList.size();i++)
				{
					if (m_initStartedMap.contains(pluginList[i]->getName()))
					{
						if (!m_initStartedMap[pluginList[i]->getName()])
						{
							all = false;
						}
					}
					else
					{
						all = false;
					}
				}
				if (all)
				{
					sendInitComplete();
				}
			}
			else if (message.getMethod() == "initcomplete")
			{
				qDebug() << "Core.cpp: Init complete finished:" << sender;
				m_initCompleteMap[sender] = true;
				bool all = true;
				for (int i=0;i<pluginList.size();i++)
				{
					if (m_initCompleteMap.contains(pluginList[i]->getName()))
					{
						if (!m_initCompleteMap[pluginList[i]->getName()])
						{
							all = false;
						}
					}
					else
					{
						all = false;
					}
					//Need to send settings to QML
					if ((pluginList[i]->getName() == "WindowManager") && sender == "WindowManager")
					{

						QStringList msg;
						//Need to send all the settings to the WindowManager, so it can set up proper settings pages
						for (int j=0;j<settings.m_pluginList.size();j++)
						{
							QString name = settings.m_pluginList[j];
							msg << name;
							msg << "0";
							int loc = msg.length()-1;
							int count = 0;
							for (int k=0;k<settings.m_keyList[name].size();k++)
							{
								if (settings.m_settingsMap[name].contains(settings.m_keyList[name][k]))
								{
									count++;
									msg << settings.m_settingsMap[name][settings.m_keyList[name][k]].key;
									msg << settings.m_settingsMap[name][settings.m_keyList[name][k]].value;
									msg << settings.m_settingsMap[name][settings.m_keyList[name][k]].type;
									msg << "0";
									//qDebug() << "!!!!" << msg;
								}
								else if (settings.m_multiSettingsMap[name].contains(settings.m_keyList[name][k]))
								{
									for (int l=0;l<settings.m_multiSettingsMap[name][settings.m_keyList[name][k]].size();l++)
									{
										count++;
										msg << settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].key;
										msg << settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].value;
										msg << settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].type;
										msg << QString::number(settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].id);
										qDebug() << settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].value;
									}
									//qDebug() << "!!!" << msg;
								}
							}
							msg[loc] = QString::number(count);
						}
						pluginList[i]->passPluginMessage("core",IPCMessage(pluginList[i]->getName(),"window","settings",msg));
					}
				}
				if (all)
				{
					for (int j=0;j<settings.m_pluginList.size();j++)
					{
						qDebug() << "Setting:" << j << "of" << settings.m_pluginList.size();
						QString name = settings.m_pluginList[j];
						if (!QMetaObject::invokeMethod(m_windowList[0]->rootObject(),"addSettingsPage",Q_ARG(QVariant,QVariant(name))))
						{

						}
						for (int k=0;k<settings.m_keyList[name].size();k++)
						{
							if (settings.m_settingsMap[name].contains(settings.m_keyList[name][k]))
							{
								QString setting = settings.m_settingsMap[name][settings.m_keyList[name][k]].key;
								QString value = settings.m_settingsMap[name][settings.m_keyList[name][k]].value;
								QString type = settings.m_settingsMap[name][settings.m_keyList[name][k]].type;
								QString id = "0";
								if (!QMetaObject::invokeMethod(m_windowList[0]->rootObject(),"addSettingsValue",Q_ARG(QVariant,QVariant(name)),Q_ARG(QVariant,QVariant(setting)),Q_ARG(QVariant,QVariant(value)),Q_ARG(QVariant,QVariant(type)),Q_ARG(QVariant,QVariant(id))))
								{

								}
							}
							else if (settings.m_multiSettingsMap[name].contains(settings.m_keyList[name][k]))
							{
								for (int l=0;l<settings.m_multiSettingsMap[name][settings.m_keyList[name][k]].size();l++)
								{
									QString setting = settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].key;
									QString value = settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].value;
									QString type = settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].type;
									QString id = QString::number(settings.m_multiSettingsMap[name][settings.m_keyList[name][k]][l].id);
									if (!QMetaObject::invokeMethod(m_windowList[0]->rootObject(),"addSettingsValue",Q_ARG(QVariant,QVariant(name)),Q_ARG(QVariant,QVariant(setting)),Q_ARG(QVariant,QVariant(value)),Q_ARG(QVariant,QVariant(type)),Q_ARG(QVariant,QVariant(id))))
									{

									}

								}
							}

						}
					}

					for (int i=0;i<pluginList.size();i++)
					{
					}
					qDebug() << "All inits sent and complete.";
				}
			}
			else if (message.getMethod() == "initclose")
			{

			}
			else if (message.getMethod() == "throw")
			{
				if (message.getArgs()[0] == "event:propertyset")
				{
					int count = message.getArgs()[1].toInt();
					for (int i=0;i<count;i++)
					{
						//qDebug() << "Propertyset" << message.getArgs()[2+(i * 2)] << message.getArgs()[3+(i * 2)];
						propertyMap.setProperty(message.getArgs()[2+(i * 2)].toAscii(),QVariant(message.getArgs()[3+(i * 2)]));
						//i++;
								//m_windowList[j]->setQMLProperty(,);
					}

				}
				for (int i=0;i<eventHandlerList.count();i++)
				{
					if (eventHandlerList[i].first == message.getArgs()[0])
					{
						PluginClass *tmpClass = qobject_cast<PluginClass*>(eventHandlerList[i].second);
						if (tmpClass)
						{
							if (blocking)
							{
								tmpClass->passPluginMessageBlocking("core",IPCMessage(tmpClass->getName(),"event","throw",message.getArgs()));
							}
							else
							{
								tmpClass->passPluginMessage("core",IPCMessage(tmpClass->getName(),"event","throw",message.getArgs()));
							}
						}
					}
				}
			}
			return;
		}
		else if (message.getClass() == "register")
		{
			//debugWindow->debug_output("Plugin registered: " + message.getArgs()[1]);
			bool all = true;
			for (int i=0;i<pluginList.count();i++)
			{
				//if (message.getArgs()[0] == pluginList[i]->getFileName());
				//The next three lines are a hack, for some odd reason the above line does not work. Very odd behavior.
				QString cmp1 = message.getArgs()[0];
				QString cmp2 = pluginList[i]->getFileName();
				if (cmp1 == cmp2)
				{
					pluginList[i]->setName(message.getArgs()[1]);
					pluginList[i]->passPluginMessage("core",IPCMessage(message.getArgs()[1],"event","registered",QStringList()));
				}
			}
			for (int i=0;i<pluginList.count();i++)
			{
				if ((!pluginList[i]->IsRegistered()) && (message.getArgs()[1] != pluginList[i]->getName()))
				{
					QString d = pluginList[i]->getFileName();
					//debugWindow->debug_output("Plugin not registered!: " + pluginList[i]->getFileName());
					all = false;
				}
				else if (pluginList[i]->IsRegistered())
				{
					//debugWindow->debug_output("Plugin registered!: " + pluginList[i]->getName());
				}
				else
				{
					QString d = pluginList[i]->getFileName();
					//debugWindow->debug_output("Plugin not registered!: " + pluginList[i]->getFileName());
					all = false;
				}
			}
			if ((all))
			{
				initsThrown = true;
				//debugWindow->debug_output("All plugins registered, sending inits");
				sendInitStarted();
				qDebug() << "Core.cpp: All inits completed";
			}
		}
		else if (message.getClass() == "global")
		{
			//These are global commands, for RR functionality initially
			//if (message.getMethod() == "command")
			//{
				//QMessageBox::information(0,"Global",message.getMethod());
			if (message.getMethod() == "MAIN")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						pluginList[i]->passPluginMessage("core",IPCMessage("menu","window","show",QStringList()),false);
					}
				}
			}
			else if (message.getMethod().startsWith("LOAD"))
			{
				QStringList split = message.getMethod().split(";");
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage(split[1].mid(0,split[1].length()-5).toLower(),"window","show",QStringList()));
				//pluginList[i]->passPluginMessage("core",IPCMessage(split[1].mid(0,split[1].length()-5).toLower(),"window","show",QStringList()),false);
				if (split.count() > 2)
				{
					//findPluginByName("EmbeddingEngine")->passPluginMessage("core",IPCMessage());
					findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("WindowManager","window","embed",split));
					////windowname currenthandle fullpath x y width height
				}
			}
			else if (message.getMethod() == "AUDIO")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						pluginList[i]->passPluginMessage("core",IPCMessage("audio_player","window","show",QStringList()),false);
					}
				}	
			}
			else if (message.getMethod() == "RADIO")
			{
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("radio_player","window","show",QStringList()));
			}
			else if (message.getMethod() == "VIDEO")
			{
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("video_browser","window","show",QStringList()));
			}
			else if (message.getMethod() == "EXIT")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						//pluginList[i]->passPluginMessage("core",IPCMessage("audio_player","window","show",QStringList()),false);
						pluginList[i]->passPluginMessage("core",IPCMessage("WindowManager","window","showlast",QStringList()));
					}
				}
			}
			else if (message.getMethod().startsWith("CLOSEWINDOW"))
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						//pluginList[i]->passPluginMessage("core",IPCMessage("audio_player","window","show",QStringList()),false);
						pluginList[i]->passPluginMessage("core",IPCMessage("WindowManager","window","showlast",QStringList()));
					}
				}
			}
			else if (message.getMethod().startsWith("CLOSE"))
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						//pluginList[i]->passPluginMessage("core",IPCMessage("audio_player","window","show",QStringList()),false);
						pluginList[i]->passPluginMessage("core",IPCMessage("WindowManager","window","showlast",QStringList()));
					}
				}
			}
			else if (message.getMethod() == "LIST")
			{
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("audio_browser","window","show",QStringList()));
				/*for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == "WindowManager")
					{
						//pluginList[i]->passPluginMessage("core",IPCMessage("audio_player","window","show",QStringList()),false);
						pluginList[i]->passPluginMessage("core",IPCMessage("audio_browser","window","show",QStringList()));
					}
				}
				*/
			}
			else if (message.getMethod() == "EXITMENU")
			{
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("exit_menu","window","show",QStringList()));
			}
			else if (message.getMethod() == "CANCEL")
			{
				findPluginByName("WindowManager")->passPluginMessage("core",IPCMessage("WindowManager","window","showlast",QStringList()));
			}
			else if (message.getMethod() == "QUIT")
			{
				passedCoreMessage("core",IPCMessage("core","admin","close",QStringList()));
			}
			else if (message.getMethod() == "PLAY")
			{
				passedCoreMessage("core",IPCMessage("MediaEngine","media","play",QStringList()));
			}
			else if (message.getMethod() == "PREV")
			{
				passedCoreMessage("core",IPCMessage("MediaEngine","media","prev",QStringList()));
			}
			else if (message.getMethod() == "STOP")
			{
				passedCoreMessage("core",IPCMessage("MediaEngine","media","stop",QStringList()));
			}
			else if (message.getMethod() == "NEXT")
			{
				passedCoreMessage("core",IPCMessage("MediaEngine","media","next",QStringList()));
			}
			else if (message.getMethod() == "PGUP")
			{
				passedCoreMessage("core",IPCMessage("WindowManager","window","rrcmd",QStringList() << "scrollpgup"));
			}
			else if (message.getMethod() == "PGDOWN")
			{
				passedCoreMessage("core",IPCMessage("WindowManager","window","rrcmd",QStringList() << "scrollpgdown"));
			}
			else if (message.getMethod() == "UP")
			{
				passedCoreMessage("core",IPCMessage("WindowManager","window","rrcmd",QStringList() << "scrollup"));
			}
			else if (message.getMethod() == "DOWN")
			{
				passedCoreMessage("core",IPCMessage("WindowManager","window","rrcmd",QStringList() << "scrolldown"));
			}
			return;
			//}
		}
		else if (message.getClass() == "objectlist")
		{
			if (message.getMethod() == "addobject")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					QString cmp1 = pluginList[i]->getName();
					QString cmp2 = sender;
					QString cmp3 = message.getTarget();
					if (cmp1 == sender)
					{
						if (pluginAttributes.contains(message.getArgs()[0]))
						{
							debugWindow->debug_output("Added attributes to: " + pluginList[i]->getName());
							pluginAttributes[pluginList[i]->getName()] = QStringList() << pluginAttributes.value(pluginList[i]->getName()) << message.getArgs()[0] << QString::number(pluginAttributes.value(message.getArgs()[0]).count()) << pluginAttributes.value(message.getArgs()[0]);
						}
						debugWindow->debug_output("Didn't add attributes to: " + pluginList[i]->getName() + " for: " + message.getArgs()[0]);
						extraHandlerList.append(QPair<QString,QObject*>(message.getArgs()[0],pluginList[i]));
					}
				}
			}
		}
		else if (message.getClass() == "admin")
		{
			if (message.getMethod() == "close")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					qDebug() << "Passing init:close to:" << pluginList[i]->getName();
					pluginList[i]->passPluginMessageBlocking("core",IPCMessage(pluginList[i]->getName(),"event","initclose",QStringList()));
				}
				QApplication::exit(1);
			}
			else if (message.getMethod() == "standby")
			{
				#ifdef Q_OS_WIN32
				SetSuspendState(false,true,false);
				#endif
			}
			else if (message.getMethod() == "hibernate")
			{
				#ifdef Q_OS_WIN32
				SetSuspendState(true,true,false);
				#endif
			}
			else if (message.getMethod() == "unloadplugin")
			{
				bool found = false;
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == message.getArgs()[0])
					{
						pluginList[i]->unloadPlugin();
						found = true;
					}
				}
				if (!found)
				{
					for (int i=0;i<pluginList.count();i++)
					{
						if (pluginList[i]->getName() == "DotNetManager")
						{
							//pluginList[i]->unloadPlugin();
							pluginList[i]->passPluginMessage("core",IPCMessage("DotNetManager","admin","unloadplugin",message.getArgs()));
							//found = true;
						}
					}					
				}

			}
			else if (message.getMethod() == "reloadplugin")
			{
				for (int i=0;i<pluginList.count();i++)
				{
					if (pluginList[i]->getName() == message.getArgs()[0])
					{
						pluginList[i]->reload();
						//pluginList[i]->startUnThreaded();
					}
				}
			}
			else if (message.getMethod() == "pluginlist")
			{
				QStringList list;
				for (int i=0;i<pluginList.count();i++)
				{
					list << pluginList[i]->getName();;
					list << pluginList[i]->getFileName();
					list << ((pluginList[i]->isLoaded()) ? "true" : "false");
				}
				passedCoreMessage("core",IPCMessage(message.getArgs()[0],message.getArgs()[1],message.getArgs()[2],list),blocking);
			}
		}
		else if (message.getClass() == "clipboard")
		{
			if (message.getMethod() == "copy")
			{
				QApplication::clipboard()->setText(message.getArgs()[0]);
			}
			else if (message.getMethod() == "paste")
			{
				passedCoreMessage("core",IPCMessage(message.getArgs()[0],message.getArgs()[1],message.getArgs()[2],QStringList() << QApplication::clipboard()->text()));
			}
		}
		else if (message.getClass() == "settings")
		{
			if (message.getMethod() == "save")
			{

			}
		}
	}
	else
	{
		//For some reason I disabled this method of blocking plugin calls, not sure why.
		//I'll figure it out eventually, but for now it seems for the most part plugins
		//are unable to send blocking calls to each other. Probably for the best.
		/*
		if (blocking)
		{
		for (int i=0;i<pluginList.count();i++)
		{
			QString cmp1 = pluginList[i]->getName();
			QString cmp2 = sender;
			QString cmp3 = message.getTarget();
			if (pluginList[i]->getName() == message.getTarget())
			{
				pluginList[i]->passPluginMessageBlocking(sender,message);
				return;
			}
			if (pluginList[i]->getName() == message.getTarget())
			{
				pluginList[i]->passPluginMessageBlocking(sender,message);
				return;
			}
		}
		for (int i=0;i<extraHandlerList.count();i++)
		{
			if (extraHandlerList[i].first == message.getTarget())
			{
				qobject_cast<PluginClass*>(extraHandlerList[i].second)->passPluginMessageBlocking(sender,message);
			}
		}
		}
		else
		{
		*/
		for (int i=0;i<pluginList.count();i++)
		{
			QString cmp1 = pluginList[i]->getName();
			QString cmp2 = sender;
			QString cmp3 = message.getTarget();
			if (pluginList[i]->getName() == message.getTarget())
			{
				pluginList[i]->passPluginMessage(sender,message);
				return;
			}
		}
		for (int i=0;i<extraHandlerList.count();i++)
		{
			QString cmp1 = extraHandlerList[i].first;
			QString cmp2 = message.getTarget();

			if (extraHandlerList[i].first == message.getTarget())
			{
				PluginClass* tmpPlugin = qobject_cast<PluginClass*>(extraHandlerList[i].second);
				QString cmp3 = tmpPlugin->getName();
				tmpPlugin->passPluginMessage(sender,message);

				debugWindow->debug_output("Found extra handler for: " + message.getTarget() + " At: " + tmpPlugin->getName());
				return;
			}
		}
		debugWindow->debug_output("Error finding handler for: " + message.getTarget() + " from: " + sender);
	}
}
void CoreApp::saveSetting(QString name,QString key,QString value,QString type,int id)
{
	//QString name = message.getArgs()[0];
	//QString key = message.getArgs()[1];
	//QString value = message.getArgs()[2];
	//QString type = message.getArgs()[3];
	//int id = message.getArgs()[4].toInt();
	if (!settings.m_settingsMap.contains(name) && !settings.m_multiSettingsMap.contains(name))
	{
		qDebug() << "Core.cpp: Invalid plugin specified when trying to save settings:" << name;
		return;
	}
	if (settings.m_settingsMap[name].contains(key))
	{
		if (settings.m_settingsMap[name][key].type != type)
		{
			qDebug() << "Core.cpp: Invalid type specified for saved value. Expected" << settings.m_settingsMap[name][key].type << "got" << type;
			return;
		}
		settings.m_settingsMap[name][key].value = value;
	}
	else if (settings.m_multiSettingsMap[name].contains(key))
	{
		bool found = false;
		for (int i=0;i<settings.m_multiSettingsMap[name][key].size();i++)
		{
			if (settings.m_multiSettingsMap[name][key][i].id == id)
			{
				found = true;
				if (settings.m_multiSettingsMap[name][key][i].type == type)
				{
					settings.m_multiSettingsMap[name][key][i].value = value;
				}
				else
				{
					qDebug() << "Core.cpp: Invalid type specified for saved value. Expected:" << settings.m_multiSettingsMap[name][key][i].type << "got" <<type;
					return;
				}
			}
		}
		if (!found)
		{
			qDebug() << "Core.cpp: No setting found for ID:" << QString::number(id) << "Adding new setting";
			SettingValue set;
			set.id = id;
			set.key = key;
			set.value = value;
			set.type = type;
			qDebug() << "Core.cpp: New setting:" << name << key << value << type << id;
			qDebug() << "Core.cpp: Old size:" << settings.m_multiSettingsMap[name][key].size();
			settings.m_multiSettingsMap[name][key].append(set);
			qDebug() << "Core.cpp: New Size:" << settings.m_multiSettingsMap[name][key].size();
		}
	}
	else
	{
		qDebug() << "Core.cpp: Invalid key specified when trying to save settings for plugin" << name << "Key" << key;
		return;
	}
	saveSettings();
	for (int i=0;i<pluginList.count();i++)
	{
		if (pluginList[i]->getName() == name)
		{
			//pluginList[i]->unloadPlugin();
			pluginList[i]->passPluginMessage("core",IPCMessage(name,"setting","change",QStringList() << key << value << type << QString::number(id)));
			//found = true;
		}
	}
}
void CoreApp::exitClicked()
{
	//Use the message function here, it's easier.
	passedCoreMessage("core",IPCMessage("core","admin","close",QStringList()));
}

PluginClass* CoreApp::findPluginByName(QString name)
{
	for (int i=0;i<pluginList.count();i++)
	{
		if (pluginList[i]->getName() == name)
		{
			return pluginList[i];
		}
	}
	return 0;
}
CoreApp::~CoreApp()
{
	qDebug() << "Core.cpp: Shutting down...";
	for (int i=0;i<pluginList.count();i++)
	{
		qDebug() << "Core.cpp: Sending close command to plugin: " << pluginList[i]->getName();
		pluginList[i]->passPluginMessageBlocking("core",IPCMessage(pluginList[i]->getName(),"event","initclose",QStringList()));
	}
}
void CoreApp::dataRequest(QSessionManager &manager)
{
	Q_UNUSED(manager)
	//qDebug() << "CANCELED SHUTDOWN!!!\n";
	qDebug() << "Shutting down...";
	//manager.cancel();
	for (int i=0;i<pluginList.count();i++)
	{
		pluginList[i]->passPluginMessageBlocking("core",IPCMessage(pluginList[i]->getName(),"event","initclose",QStringList()));
	}
}
