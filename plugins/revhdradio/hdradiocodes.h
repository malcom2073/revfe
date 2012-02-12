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

#ifndef HDRADIOCODES_H
#define HDRADIOCODES_H
#include <QString>
#include <QList>
#include <QByteArray>
#include <QPair>
class HdRadioCodes
{
public:
	HdRadioCodes();
	QString getCommandStringFromBytes(QByteArray ba);
	QString getOperationStringFromBytes(QByteArray ba);
	QString getConstantStringFromBytes(QByteArray ba);
	QByteArray getOperationBytesFromString(QString op);
	QByteArray getCommandBytesFromString(QString op);
	QByteArray getConstantBytesFromString(QString op);
	QString getReturnFormat(QString arg);
	QList<QPair<QString,QByteArray> > hdRadioCommandCodes;
	QList<QPair<QString,QByteArray> > hdRadioOperationCodes;
	QList<QPair<QString,QByteArray> > hdRadioConstantCodes;
	QList<QPair<QString,QByteArray> > hdRadioArgs;
	QList<QPair<QString,QString> > hdRadioDataFormat;
};

#endif
