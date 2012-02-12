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

#include "debugwindow.h"
#include <QDateTime>


DebugWindow::DebugWindow()
{
	//this->show();
	this->setGeometry(1000,0,400,400);
	output_Display = new QTextBrowser(this);
	output_Display->setGeometry(10,10,380,380);
	output_Display->show(); 
	debugEnabled = false;
}
void DebugWindow::debug_output(QString text)
{
	if (debugEnabled)
	{
		debugFile.write(QString(text + "\n").toAscii());
		debugFile.flush();
		output_Display->insertPlainText(text + "\n");
	}	
//output_Display->setText(output_Display->getPlainText() + text + "\n");
}
void DebugWindow::enableDebug()
{
	debugFile.setFileName(QString("debugOutput.").append(QString::number(QDateTime::currentDateTime().toTime_t())).append(".log"));
	debugFile.open(QIODevice::ReadWrite | QIODevice::Text);
	debugEnabled = true;
}
void DebugWindow::disableDebug()
{
	debugEnabled = false;
}
