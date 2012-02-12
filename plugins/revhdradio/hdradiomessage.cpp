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

#include "hdradiomessage.h"



HdRadioMessage::HdRadioMessage(QByteArray message,HdRadioCodes *hdCodes)
{
		messageClass = "";
		messageDirection = "";
		valid = false;
		if (message[0] == (char)0xA4)
		{
			valid = true;
			messageClass = hdCodes->getCommandStringFromBytes(message.mid(2,2));
			messageDirection = hdCodes->getOperationStringFromBytes(message.mid(4,2));
			QString retType = hdCodes->getReturnFormat(messageClass);
			if (retType == "boolean")
			{
				if (message[6] == (char)0x01)
				{
					messageArg = "true";
				}
				else
				{
					messageArg = "false";
				}
			}
			else if (retType == "int:string")
			{
				messageArg += QString::number(message.at(6)).append(0x24);
				for (int i=14;i<message.length()-1;i++)
				{
					messageArg += message[i];
					//messageArg += QString::number(message.at(6).append(0x24).append(
				}
			}
			else if (retType == "string")
			{
				for (int i=10;i<message.length()-1;i++)
				{
					//messageArg += QString::number(message.at(6)).append(0x24).append(message.at(i));
					messageArg += message[i];
				}				
			}
			else if (retType == "bitmask")
			{
				for (int i=6;i<message.length()-1;i++)
				{
					messageArg += QString::number(message[i]);
				}
			}
			//if ((message[6] == (char)0x01) && (message[7] == (char)0x00) && (message[8] == (char)0x00) && (message[9] == (char)0x00))
			//{
			//	messageArg = "up";
			//}
		}
}
