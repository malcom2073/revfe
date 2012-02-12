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

#ifndef BASEINTERFACE_H
#define BASEINTERFACE_H
#include <QtPlugin>
#include "ipcmessage.h"
class BaseInterface : public QObject
{
Q_OBJECT
public:
	virtual void init() = 0;
	virtual QString getName() = 0;
	//virtual void passPluginMessage(QString pluginName,QString message) = 0;
public slots:
	virtual void passPluginMessage(QString sender,QString message) = 0;
	virtual void passPluginMessage(QString sender,IPCMessage message) = 0;
	//virtual void passPluginMessageBlocking(QString sender,IPCMessage message) = 0;
signals:
	void passCoreModel(QString name, QObject *model);
	void passCoreGUIItem(QObject *item);
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);

};
Q_DECLARE_INTERFACE(BaseInterface,"BaseInterface/1.0")
#endif //BASEINTERFACE_H
