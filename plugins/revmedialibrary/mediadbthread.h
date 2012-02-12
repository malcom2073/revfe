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

#ifndef MEDIADBTHREAD_H
#define MEDIADBTHREAD_H
#include <QThread>
#include <fileref.h>
#include <tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <QMutex>
#include "fileref.h"
#include "id3tag.h"
#include "mediafile.h"
#include "mediadb.h"
#include "videodb.h"

class MediaDBThread : public QThread
{
	Q_OBJECT
public:
	MediaDBThread();
	virtual ~MediaDBThread();
	void run();
	MediaDB *getDatabase() { return mediaDatabase; }
	void setPlayListPointer(QList<PlayListClass*> &pointer);
	void setDir(QString dir) { musicDir = dir; }
	void initialScan();
	void scan(QList<QString> files);
	void addDirectory(QString dir);
private:
	QList<QString> m_scanList;
	QList<QString> m_internalScanList;
	QList<QString> m_dirList;
	QList<QString> m_internalDirList;
	QMutex m_scanListMutex;
	QMutex m_dirListMutex;
	QString musicDir;
	MediaDB *mediaDatabase;
	QList<PlayListClass*> Playlists;
signals:
	void threadFinished(QList<PlayListClass*> playlists);
	void tagReceived(QString playlist, int location, ID3Tag *tag);
	void tagsReceived(QList<QString> filename,const QList<ID3Tag> tag);
	void tagReceived(QString filename,const ID3Tag tag);
	void mediaList(QList<MediaFileClass*> media);
	void scanProgressUpdate(QString progress);
	void initialScanComplete();
	void initialFilesAdded();
};
#endif //MEDIADBTHREAD_H
