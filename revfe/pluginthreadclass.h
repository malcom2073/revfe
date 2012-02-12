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

#ifndef PLUGINTHREADCLASS_H
#define PLUGINTHREADCLASS_H
#include <QThread>
#include <QPluginLoader>
#include "ipcmessage.h"
class PluginThreadClass : public QThread
{
	Q_OBJECT
public:
	PluginThreadClass(QObject *parent = 0);
	void passPluginFileName(QString filename);
	QString pluginName;
	QString filename;
	int waitForReady();
	int waitForInitStarted();
	int waitForInitComplete();
	int waitForInitClose();
	bool initstarted;
	bool initcomplete;
	bool initclose;
	void unloadPlugin();
private:
	void run();
	bool ready;
	void passPluginMessageSlot(QString sender, IPCMessage message, bool blocking);
	QPluginLoader loader;
public slots:
	void passPluginMessageSlot(QString sender, IPCMessage message);
	void passPluginMessageSlotBlocking(QString sender, IPCMessage message);
signals:
	void passCoreMessage(QString sender, IPCMessage message);
	void passCoreMessageBlocking(QString,IPCMessage);
	void passCoreModel(QString name,QObject *model);
	void passCoreGUIItem(QObject *item);
	void passPluginMessage(QString sender, IPCMessage message);
	void passPluginMessageBlocking(QString,IPCMessage);
	void pluginLoadFail(QString pluginFileName);
};
#endif //PLUGINTHREADCLASS_H
