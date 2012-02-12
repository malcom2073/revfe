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


#ifndef PLAYLISTCLASS_H
#define PLAYLISTCLASS_H

#include <QString>
#include <QList>
#include "mediafileclass.h"
#include "id3tag.h"
class PlayListClass
{
public:
	PlayListClass();
	void setTitle(QString name);
	//void setPlayListName(QString name);
	void addFile(QString name);
	void insertFile(QString name,int num);
	void removeFile(int num);
	void setId3(int num,ID3Tag *tag);
	ID3Tag *id3(int num);
	QString title() { return m_playListName; }
	int size() { return m_mediaList.size(); }
	void moveSong(int num,int newnum);
	MediaFileClass* getFileNum(int num);
	void addFile(MediaFileClass* file) { m_mediaList.append(file); }
private:
	QString m_playListName;
	QList<MediaFileClass*> m_mediaList;
};
#endif //PLAYLISTCLASS_H
