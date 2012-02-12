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

#include "mediascannerclass.h"
#include <QMetaType>
MediaScannerClass::MediaScannerClass()
{
	qRegisterMetaType<ID3Tag*>("ID3Tag*");
	scanThread = new MediaScannerThread();
	connect(scanThread,SIGNAL(tagReceived(QString,int,ID3Tag*)),this,SIGNAL(threadTagReceived(QString,int,ID3Tag*)));
}

void MediaScannerClass::setDataBase(MediaDB *database)
{
	m_dataBase = database;
}
void MediaScannerClass::setPlaylists(QList<PlayListClass*> *playlists)
{
	m_playLists = playlists;
	scanThread->setMediaList(m_playLists);
}
void MediaScannerClass::singleScan(QString filename)
{
	
}
bool MediaScannerClass::isActive()
{
	return scanThread->isRunning();
}
void MediaScannerClass::multiScan(QStringList filenames)
{
	scanThread->setMultiPass(filenames);
	scanThread->start();
}
void MediaScannerClass::startScannerThread()
{
	scanThread->start();
}
