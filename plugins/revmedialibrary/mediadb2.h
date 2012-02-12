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

#ifndef MEDIADB_H
#define MEDIADB_H

#include <QObject>
#include <QFile>
#include <QDir>
#include "sqlite3.h"
#include "PlayListClass.h"
#include "ID3Tag.h"
#include "tag.h"
#include "fileref.h"
class MediaDB : public QObject
{
	Q_OBJECT
public:
	MediaDB();
	void setPlayListPointer(QList<PlayListClass*> &pointer);
	int updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
	int addEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
private:
	sqlite3 *dataBase;
	QList<PlayListClass*> *playListClassList;
};
#endif //MEDIADB_H
