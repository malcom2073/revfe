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

#include "ipcmessage.h"



IPCMessage::IPCMessage()
{

}

IPCMessage::IPCMessage(QString message)
{
//parse the message here
	//Need to replace this with a regex to prevent errors
	if ((message.indexOf(':') != -1) && (message.indexOf(' ') != -1) && (message.split(' ').count() >= 2) && (message.indexOf(';') == -1))
	{
		_valid = true;
		_target = message.split(":")[0];
		int first = message.indexOf(" ");
		int second = message.indexOf(" ",first+1);
		_class = message.mid(message.indexOf(":")+1,first - (message.indexOf(":") + 1));
		//QString pluginCommand = message.mid(first,second-first);
		//QString pluginArgs = pluginCommand.mid(pluginCommand.indexOf(" ")+1);
		_method = message.mid(first+1,second - (first + 1));
		//for (int i=0;i<
		for (int i=0;i<message.length();i++)
		{
			if (message[i] == '!')
			{
				int index = message.indexOf('!',i+1);
				if (index != -1)
				{
					_args.append(message.mid(i+1,index-(i+1)));
					i = index;
				}
				else
				{
					//unknown error
				}
			}
		}
		for (int i=0;i<_args.count();i++)
		{
			//QString tmpArg = _args[i];
		}
		//QString pluginArgs = message.mid(message.indexOf("!")+1, message.lastIndexOf("!") - (message.indexOf("!")+1));
		//_args.append(pluginArgs);
		_method = _method.mid(0,_method.indexOf(" "));
	}
	else 
	{
		_valid = false;	
	}

}
bool IPCMessage::isValid()
{
	return _valid;
}
IPCMessage::IPCMessage(QString Target, QString Class, QString Method, QStringList Args)
{
_target = Target;
_class = Class;
_method = Method;
_args = Args;
}
//IPCMessage::IPCMessage(std::string Target,std::string Class,std::string Method, QStringList Args)
//{
//	IPCMessage(QString(Target),QString(Class),QString(Method),Args);
//}
//IPCMessage(const char* Target,const char* Class,const char* Method, QStringList Args)
//{
//	IPCMessage(QString(Target),QString(Class),QString(Method),Args);
//}
void IPCMessage::setArgs(QStringList Args)
{
_args = Args;
}
QString IPCMessage::toString()
{
	QString retVar = "";
	retVar.append(_target).append(":").append(_class).append(" ").append(_method);
	for (int i=0;i<_args.count();i++)
	{
		retVar.append(" !").append(_args[i]).append("!");
	}
	return retVar;
}
