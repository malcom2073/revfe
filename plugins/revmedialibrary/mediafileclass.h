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


#ifndef MEDIAFILECLASS_H
#define MEDIAFILECLASS_H

#include <QString>
#include "id3tag.h"

class MediaFileClass
{
public:
	MediaFileClass();
	MediaFileClass(QString filename);
	void setId3Tag(ID3Tag *tag);
	ID3Tag *getId3Tag() { return m_id3Tag; }
	QString fullPath();
	void setFullPath(QString path) { m_fileName = path; }
	QString displayName();
	~MediaFileClass();
private:
	QString m_fileName;
	ID3Tag *m_id3Tag;
};


#endif //MEDIAFILECLASS_H