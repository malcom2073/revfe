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

#ifndef PLUGINCLASS_H
#define PLUGINCLASS_H

#include <QObject>
#include <QString>
#include "pluginthreadclass.h"
#include "baseinterface.h"
#include "ipcmessage.h"
#include <QPluginLoader>
class PluginClass : public QObject
{
	Q_OBJECT
public:
	PluginClass();
	PluginClass(QObject *p,QString n,QString f);
	PluginClass(QString f);
	void startThreaded();
	void startUnThreaded();
	void passPluginMessage(QString,IPCMessage);
	void passPluginMessageBlocking(QString,IPCMessage);
	void passPluginMessage(QString,IPCMessage,bool);
	QString getName() { return pluginName; }
	QString getFileName() { return pluginFileName; }
	bool IsThreaded() { return isThreaded; }
	void reload();
	void setName(QString name) { pluginName = name; }
	bool IsInitStarted() { return isInitStarted; }
	bool IsInitCompleted() { return isInitCompleted; }
	bool IsRegistered() { return registered; }
	void unloadPlugin();
	bool isLoaded() { return isPluginLoaded; }
private:
	bool isPluginLoaded;
	bool registered;
	bool isThreaded;
	bool isInitStarted;
	bool isInitCompleted;
	PluginThreadClass *threadedPluginObject;
	BaseInterface *pluginObject;
	QString pluginName;
	QString pluginFileName;
	QPluginLoader loader;
signals:
	void passPluginMessageSignal(QString,IPCMessage);
	void passCoreModel(QString,QObject*);
	void passCoreGUIItem(QObject *item);
	void passPluginMessageSignalBlocking(QString,IPCMessage);
	void passCoreMessageBlocking(QString,IPCMessage);
	void passCoreMessage(QString,IPCMessage);
	void pluginLoadFail(QString pluginFileName);
private slots:
	//void passPluginMessage(QString,IPCMessage,bool);
	//void passCoreMessage(QString,IPCMessage);
};
#endif //PLUGINCLASS_H
