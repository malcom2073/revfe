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

#include "mediafile.h"

#include <QMessageBox>
MediaFile::MediaFile()
{

}
MediaFile::MediaFile(QString filename)
{
	m_fullPath = filename;
	m_fileName = filename.mid(filename.lastIndexOf('/')+1);
}
void MediaFile::setArtist(QString artist)
{
	m_artist = artist;
}
void MediaFile::setAlbum(QString album)
{
	m_album = album;
}
void MediaFile::setTitle(QString title)
{
	m_title = title;
}
void MediaFile::setPath(QString path)
{
	m_fullPath = path;
}
QString MediaFile::artist()
{
	return m_artist;
}
QString MediaFile::album()
{
	return m_album;
}
QString MediaFile::title()
{
	return m_title;
}
QString MediaFile::fileName()
{
	return m_fileName;
}
QString MediaFile::fullPath()
{
	return m_fullPath;
}
QString MediaFile::displayName()
{
	if ((!m_title.isEmpty()) && (!m_artist.isEmpty()))
	{
		return "NULL";
	}
	else if (!m_fileName.isEmpty())
	{
		return m_fileName;
	}
	else
	{
		return m_fullPath;
	}
}
