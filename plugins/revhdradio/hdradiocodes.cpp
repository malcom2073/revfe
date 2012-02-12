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

#include "hdradiocodes.h"



HdRadioCodes::HdRadioCodes()
{
	QByteArray ba;
	ba.append((char)0x01);
	ba.append((char)0x00);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("power",ba));
	ba.clear();
	ba.append((char)0x02);
	ba.append((char)0x00);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("mute",ba));
	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x01);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("signalstrength",ba));
	ba.clear();
	ba.append((char)0x02);
	ba.append((char)0x01);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("tune",ba));
	ba.clear();
	ba.append((char)0x03);
	ba.append((char)0x01);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("seek",ba));


	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdactive",ba));
	ba.clear();
	ba.append((char)0x02);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdstreamlock",ba));
	ba.clear();
	ba.append((char)0x03);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdsignalstrength",ba));
	ba.clear();
	ba.append((char)0x04);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdsubchannel",ba));
	ba.clear();
	ba.append((char)0x05);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdsubchannelcount",ba));
	ba.clear();
	ba.append((char)0x06);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdenablehdtuner",ba));
	ba.clear();
	ba.append((char)0x07);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdtitle",ba));
	ba.clear();
	ba.append((char)0x08);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdartist",ba));
	ba.clear();
	ba.append((char)0x92);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdcallsign",ba));
	ba.clear();
	ba.append((char)0x10);
	ba.append((char)0x02);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdstationname",ba));
	//hdRadioCommandCodes.append(QPair<QString,QByteArray>("hduniqueid",QByteArray(0x11,0x02)));
	//hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdapiversion",QByteArray(0x12,0x02)));
	//hdRadioCommandCodes.append(QPair<QString,QByteArray>("hdhwversion",QByteArray(0x13,0x02)));

	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x03);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("rdsenable",ba));
	ba.clear();
	ba.append((char)0x07);
	ba.append((char)0x03);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("rdsgenre",ba));
	ba.clear();
	ba.append((char)0x08);
	ba.append((char)0x03);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("rdsprogramservice",ba));
	ba.clear();
	ba.append((char)0x09);
	ba.append((char)0x03);
	hdRadioCommandCodes.append(QPair<QString,QByteArray>("rdsradiotext",ba));



	ba.clear();
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioOperationCodes.append(QPair<QString,QByteArray>("set",ba));
	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x00);
	hdRadioOperationCodes.append(QPair<QString,QByteArray>("get",ba));
	ba.clear();
	ba.append((char)0x02);
	ba.append((char)0x00);
	hdRadioOperationCodes.append(QPair<QString,QByteArray>("reply",ba));

	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("up",ba));
	ba.clear();
	ba.append((char)0xFF);
	ba.append((char)0xFF);
	ba.append((char)0xFF);
	ba.append((char)0xFF);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("down",ba));
	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("fm",ba));
	ba.clear();
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("am",ba));
	ba.clear();
	ba.append((char)0x01);
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("one",ba));
	ba.clear();
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	ba.append((char)0x00);
	hdRadioConstantCodes.append(QPair<QString,QByteArray>("zero",ba));
	
	hdRadioDataFormat.append(QPair<QString,QString>("power","boolean"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdactive","boolean"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdstreamlock","boolean"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdsignalstrength","int"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdsubchannel","bitmask"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdsubchannelcount","bitmask"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdenablehdtuner","boolean"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdtitle","int:string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdartist","int:string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdcallsign","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdstationname","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hduniqueid","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdapiversion","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("hdhwversion","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("rdsradiotext","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("rdsprogramservice","string"));
	hdRadioDataFormat.append(QPair<QString,QString>("rdsgenre","string"));

}
QString HdRadioCodes::getReturnFormat(QString arg)
{
	for (int i=0;i<hdRadioDataFormat.count();i++)
	{
		if (hdRadioDataFormat[i].first == arg)
		{
			return hdRadioDataFormat[i].second;
		}
	}
	return "";
}
QString HdRadioCodes::getCommandStringFromBytes(QByteArray ba)
{
	for (int i=0;i<hdRadioCommandCodes.count();i++)
	{
		char one = ba[0];
		char two = ba[1];
		QString first = hdRadioCommandCodes[i].first;
		char oneone = hdRadioCommandCodes[i].second[0];
		char twotwo = hdRadioCommandCodes[i].second[1];
 		if ((one == 0x09) && (two == 0x03))
		{
			int stop = 1;
		}
		if (ba == hdRadioCommandCodes[i].second)
		{
			return hdRadioCommandCodes[i].first;
		}
	}
	return "";
}
QByteArray HdRadioCodes::getOperationBytesFromString(QString op)
{
	for (int i=0;i<hdRadioOperationCodes.count();i++)
	{
		if (op == hdRadioOperationCodes[i].first)
		{
			return hdRadioOperationCodes[i].second;
		}
	}
	return QByteArray();
}
QByteArray HdRadioCodes::getCommandBytesFromString(QString op)
{
	for (int i=0;i<hdRadioCommandCodes.count();i++)
	{
		if (op == hdRadioCommandCodes[i].first)
		{
			return hdRadioCommandCodes[i].second;
		}
	}
	return QByteArray();
}
QString HdRadioCodes::getOperationStringFromBytes(QByteArray ba)
{
	for (int i=0;i<hdRadioOperationCodes.count();i++)
	{
		if (ba == hdRadioOperationCodes[i].second)
		{
			return hdRadioOperationCodes[i].first;
		}
	}	
	return "";
}
QString HdRadioCodes::getConstantStringFromBytes(QByteArray ba)
{
	for (int i=0;i<hdRadioConstantCodes.count();i++)
	{
		if (ba == hdRadioConstantCodes[i].second)
		{
			return hdRadioConstantCodes[i].first;
		}
	}	
	return "";
}
QByteArray HdRadioCodes::getConstantBytesFromString(QString op)
{
	for (int i=0;i<hdRadioConstantCodes.count();i++)
	{
		if (op == hdRadioConstantCodes[i].first)
		{
			return hdRadioConstantCodes[i].second;
		}
	}
	return QByteArray();
}
