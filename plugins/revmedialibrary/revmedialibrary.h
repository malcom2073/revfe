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

#include <QApplication>
#include <QString>
#include <QTimer>
#include "baseinterface.h"
#include "standardmodel.h"
#include "ipcmessage.h"
#include "playlistclass.h"
#include "mediadb.h"
#include <QList>
#include <QTreeView>
#include "mediascannerclass.h"
#include "mediadbthread.h"
#include "videodb.h"
class MediaLibrary : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)
public:
	MediaLibrary();
	void init();
	void show();
	QString getName();
private:
	QString m_name;
	bool dirfound;
	//QList<Playlist*> Playlists;
	QList<PlayListClass*> *Playlists;
	QString NowPlayingBaseString;
	QString m_baseDir;
	PlayListClass* NowPlayingList;
	MediaDB *dataBase;
	QTimer *databaseTimer;
	MediaScannerClass *mediaScanner;
	StandardModel *m_playlistModel;
	StandardModel *m_playlistEditModel;
	StandardModel *m_playlistListModel;
	MediaScannerClass *singleScanner;
	MediaDBThread *mediaDBThread;
	QList<EntryClass> databaseEntryList;
	int databaseEntryTickCount;
	QList<QString> m_currentEditPlaylist;
	QList<PlayListClass*> m_playlists;
	QList<MediaFileClass*> m_mediaList;
	//QList<QString> m_musicDirList;
	QMap<int,QString> m_musicDirMap;
	//QList<PlayListClass*> m_mediaFolderList;
	void createPlaylists(QString from);
	void generateFolderModel(QString dir);
public slots:
	void mediaInformationUpdate(QString playlistname,int tracknum, ID3Tag *tag);
	void mediaInformationUpdate(QString filename,const ID3Tag tag);
	void multipleMediaInformationUpdate(QList<QString> filename,const QList<ID3Tag> tag);
	void mediaList(QList<MediaFileClass*> media);
	void mediaDBThreadEnded(QList<PlayListClass*> playlists);
	void mediaDBInitialScanComplete();
	void mediaScanProgressUpdate(QString);
	void databaseTimerTick();
	void mediaDBError(QString);
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
};
