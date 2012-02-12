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

#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H
#include <QString>
#include <QStringList>

class IPCMessage
{
public:
	IPCMessage();
	IPCMessage(QString message);
	IPCMessage(QString Target, QString Class, QString Method, QStringList Args);
	//IPCMessage(std::string Target,std::string Class,std::string Method, QStringList Args);
	//IPCMessage(const char* Target,const char* Class,const char* Method, QStringList Args);
	QString getTarget() { return _target; }
	QString getClass() { return _class; }
	QString getMethod() { return _method; }
	QStringList getArgs() { return _args; }
	void setArgs(QStringList Args);
	bool isValid();
	QString toString();
private:
	QString _target;
	QString _class;
	QString _method;
	QStringList _args;
	bool _valid;
};
#endif //IPCMESSAGE_H