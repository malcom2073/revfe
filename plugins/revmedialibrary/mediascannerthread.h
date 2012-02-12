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

#ifndef MEDIASCANNERTHREAD_H
#define MEDIASCANNERTHREAD_H
#include <QThread>
#include <QList>
#include <QStringList>
#include  "playlistclass.h"
#include <fileref.h>
#include <tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include "fileref.h"
#include "id3tag.h"

class MediaScannerThread : public QThread
{
	Q_OBJECT
public:
	MediaScannerThread();
	void run();
	void setDirectory(QString dir);
	void setMediaList(QList<PlayListClass*> *playlists);
	void setSinglePass(QString filename);
	void setMultiPass(QStringList filenames);
private:
	bool singlePass;
	bool multiPass;
	QString singlePassFileName;
	QStringList multiPassFileNames;
	QList<PlayListClass*> *m_playLists;
	QString scanDir;
signals:
	void tagReceived(QString playlist, int location, ID3Tag *tag);
};
#endif //MEDIASCANNERTHREAD_H
