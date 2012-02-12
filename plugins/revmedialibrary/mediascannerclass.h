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

#ifndef MEDIASCANNERCLASS_H
#define MEDIASCANNERCLASS_H
#include <QObject>
#include <QStringList>
#include "mediascannerthread.h"
#include "mediadb.h"
#include "id3tag.h"
class MediaScannerClass : public QObject
{
	Q_OBJECT
public:
	MediaScannerClass();
	void setDataBase(MediaDB *database);
	void startScannerThread();
	void setPlaylists(QList<PlayListClass*> *playlists);
	void singleScan(QString filename);
	void multiScan(QStringList filename);
	bool isActive();
private:
	QList<PlayListClass*> *m_playLists;
	MediaScannerThread *scanThread;
	MediaDB *m_dataBase;
signals:
	void threadTagReceived(QString playlist, int location, ID3Tag *tag);
};
#endif //MEDIASCANNERCLASS_H
