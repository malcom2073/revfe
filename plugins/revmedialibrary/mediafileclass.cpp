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


#include "mediafileclass.h"


MediaFileClass::MediaFileClass()
{
	m_id3Tag = 0;
}
MediaFileClass::~MediaFileClass()
{
	if (m_id3Tag)
	{
		delete m_id3Tag;
	}
}

MediaFileClass::MediaFileClass(QString filename)
{
	m_id3Tag = 0;
	m_fileName = filename;
}
QString MediaFileClass::fullPath()
{
	return m_fileName;
}
QString MediaFileClass::displayName()
{
	if ((m_id3Tag->artist != "") && (m_id3Tag->title != ""))
	{
		return QString(m_id3Tag->artist).append(" - ").append(m_id3Tag->title);
	}
	else
	{
		return m_fileName;
	}
}
void MediaFileClass::setId3Tag(ID3Tag *tag)
{
	if (m_id3Tag)
	{
		delete m_id3Tag;
	}
	m_id3Tag = tag;
}
