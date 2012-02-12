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
//#include "sqlite3.h"
#include <QSqlDatabase>
#include "playlistclass.h"
#include "id3tag.h"
#include "tag.h"
#include "fileref.h"
class EntryClass
{
public:
	QString Target_Table;
	int Target_Entry;
	QString title;
	QString artist;
	QString album;
	QString filename;
	QString fullPath;
	int tracknum;
};
class MediaDB : public QObject
{
	Q_OBJECT
public:
	MediaDB(QObject *parent=0);
	bool isOpen() { return open; }
	void openDB();
	void closeDB();
	void setPlayListPointer(QList<PlayListClass*> &pointer);
	void updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
	void updateEntries(QString Target_Table,QList<EntryClass> classlist);
	int addEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
	int addEntries(QString Target_Table,QList<EntryClass> classlist);
	int swapEntries(QString Target_Table,int Target_Entry_One, int Target_Entry_Two);
	int createTable(QString Target_Table);
	bool init(QString dir,QList<PlayListClass*> *playlists);
	QList<QString> init();
	QList<QString> add(QString dir);
private:
	int fileCount(QString dir);
	QList<QString> m_stringlist;
	bool open;
	QFile *debugFile;
	void enterDirectory(QString musicdir,QString dir);
	//sqlite3 *dataBase;
	//QSqlDatabase *dataBase;
	//QList<PlayListClass*> *playListClassList;
	QList<MediaFileClass*> m_mediaList;
private slots:
	void private_updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
signals:
	void updateEntrySignal(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum);
	void mediaDBError(QString error);
	void tagsReceived(QList<QString> filename, const QList<ID3Tag> tag);
	void tagReceived(QString filename, const ID3Tag tag);
	void mediaList(QList<MediaFileClass*> media);
	void fileLoadingProgress(int num, int total);
};
#endif //MEDIADB_H
