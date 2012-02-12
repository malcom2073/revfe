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

#ifndef ID3TAG_H
#define ID3TAG_H

#include <QString>
#include <QImage>

class ID3Tag
{
public:
	ID3Tag();
	QString artist;
	QString album;
	QString title;
	QString track;
	QString filename;
	QImage frontcover;
	QImage backcover;
	unsigned long lastupdate;
	bool valid;
};
#endif //ID3TAG_H
