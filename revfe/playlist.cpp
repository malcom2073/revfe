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

#include "playlist.h"


Playlist::Playlist()
{

}

void Playlist::addFile(MediaFile* file)
{
	fileList.append(file);
}
void Playlist::addFile(QString filepath)
{
	MediaFile *tmpFile = new MediaFile(filepath);
	fileList.append(tmpFile);
}
void Playlist::setTitle(QString title)
{
	m_title = title;
}
QString Playlist::title()
{
	return m_title;
}
int Playlist::size()
{
	return fileList.size();
}
MediaFile *Playlist::getFileNum(int number)
{
	return fileList.at(number);
}
