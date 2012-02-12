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


#include "playlistclass.h"


PlayListClass::PlayListClass()
{
}
void PlayListClass::setTitle(QString name)
{
	m_playListName = name;
}
void PlayListClass::addFile(QString name)
{
	m_mediaList.append(new MediaFileClass(name));
}
void PlayListClass::insertFile(QString name,int num)
{
	if (num == -1)
	{
		addFile(name);
	}
	else
	{
		m_mediaList.insert(num,new MediaFileClass(name));
	}
}
void PlayListClass::setId3(int num,ID3Tag *tag)
{
	if (tag->filename == m_mediaList[num]->fullPath())
	{
		m_mediaList[num]->setId3Tag(tag);
	}
}
ID3Tag *PlayListClass::id3(int num)
{
	return m_mediaList[num]->getId3Tag();
}
void PlayListClass::removeFile(int num)
{
	m_mediaList.removeAt(num);
}
MediaFileClass* PlayListClass::getFileNum(int num)
{
	return m_mediaList[num];
}
void PlayListClass::moveSong(int num,int newnum)
{
	if ((newnum < m_mediaList.count()) && (num < m_mediaList.count()))
	{
		//MediaFileClass *tmp = m_mediaList[num];
		m_mediaList.swap(num,newnum);
		//m_mediaList.removeAt(num);
		//m_mediaList.insert(num,tmp);
	}
}
