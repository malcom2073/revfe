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
#include <QDir>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QSettings>
#include "coreapp.h"


int main(int argc, char **argv)
{
	QApplication a(argc,argv);
	if (argc > 1)
	{
		QString arg = QString(argv[1]);
		if (arg == "clear")
		{
			QSettings set("IFS","RevFE");
			//set.beginGroup("WindowManager");
			set.clear();
			//QString name = set.value("currentwindow","").toString();

		}
	}
	CoreApp t;
	return a.exec();
}
