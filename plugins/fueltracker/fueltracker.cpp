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

#include "fueltracker.h"
#include <QMessageBox>
#include <QDateTime>

void FuelTracker::init()
{
}
QString FuelTracker::getName()
{
	return "FuelTracker";
}
void FuelTracker::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}
void FuelTracker::rebuildView()
{
			QStringList timeColumn;
			QStringList milesColumn;
			QStringList fuelColumn;
			QStringList fuelPriceColumn;
			timeColumn << "Time";
			milesColumn << "Miles";
			fuelColumn << "Fuel";
			fuelPriceColumn << "Price";
			double avgMpg=0;
			for (int i=0;i<fuelEntryList.count();i++)
			{
				if (i != 0)
				{
					avgMpg += ((fuelEntryList[i].m_miles - fuelEntryList[i-1].m_miles) / (fuelEntryList[i].m_gallons));
				}
				timeColumn << QDateTime::fromTime_t(fuelEntryList[i].m_timet).toString("MM/dd/yyyy hh:mm:ss");//QString::number(fuelEntryList[i].m_timet);
				milesColumn << QString::number(fuelEntryList[i].m_miles);
				fuelColumn << QString::number(fuelEntryList[i].m_gallons);
				fuelPriceColumn  << QString::number(fuelEntryList[i].m_price);
			}
			if (fuelEntryList.count() > 0)
			{
				avgMpg /= fuelEntryList.count();
			}
			else
			{
				avgMpg = 0;
			}
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelAvgMpgText%" << QString::number(avgMpg)));
			QStringList list;
			list << "event:listchange";
			list << "fuelHistoryList";
			list << "newlist";
			list << QString::number(fuelEntryList.count() + 1);
			list << "4";
			list << timeColumn;
			list << milesColumn;
			list << fuelColumn;
			list << fuelPriceColumn;
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",list));
	
}
void FuelTracker::save()
{
	if (QFile::exists("fueltracker.dat"))
	{
		QFile::remove("fueltracker.dat");
	}
	QFile fuelTrackerFile("fueltracker.dat");
	fuelTrackerFile.open(QIODevice::ReadWrite);
	for (int i=0;i<fuelEntryList.count();i++)
	{
		fuelTrackerFile.write(QString::number(fuelEntryList[i].m_timet,'f',0).append((char)0x24).append(QString::number(fuelEntryList[i].m_miles)).append((char)0x24).append(QString::number(fuelEntryList[i].m_gallons).append((char)0x24).append(QString::number(fuelEntryList[i].m_price)).append("\r\n")).toStdString().c_str());
	}
	fuelTrackerFile.close();
}
void FuelTracker::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender);
	if (message.getClass() == "mileage")
	{
		if (message.getMethod() == "setfuel")
		{
			currentFuel = message.getArgs()[0];
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelGallonsText%" << currentFuel));
		}
		else if (message.getMethod() == "setmiles")
		{
			currentMiles = message.getArgs()[0];
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelMilesText%" << currentMiles));
		}
		else if (message.getMethod() == "setprice")
		{
			currentPrice = message.getArgs()[0];
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelPriceText%" << currentPrice));
		}
		else if (message.getMethod() == "setdate")
		{
			currentTime = message.getArgs()[0];
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelDateText%" << QDateTime::fromTime_t(currentTime.toLong()).toString("MM/dd/yyyy hh:mm:ss")));
		}
		else if (message.getMethod() == "remove")
		{
			//int index = message.getArgs()[0].toInt();
			fuelEntryList.removeAt(message.getArgs()[0].toInt() - 1);
			//emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:listchange" << "fuelHistoryList" << "remove" << message.getArgs()[0]));
			rebuildView();
			save();
		}
		else if (message.getMethod() == "save")
		{
			//uint timet = QDateTime::currentDateTime().toTime_t();
			uint timet = currentTime.toLong();
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelGallonsText%" << ""));
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelMilesText%" << ""));
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelPriceText%" << ""));
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%fuelDateText%" << ""));
			//fuelMilesList.append(QPair<QString,QString>(currentMiles,currentFuel));
			fuelEntryList.append(FuelEntry(currentMiles.toDouble(),currentFuel.toDouble(),timet,currentPrice.toDouble()));
			currentMiles = "";
			currentFuel = "";
			currentPrice = "";
			save();
			rebuildView();
		}
		else if (message.getMethod() == "clear")
		{
			QFile::remove("fueltracker.dat");
			fuelEntryList.clear();
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","throw",QStringList() << "event:listchange" << "fuelHistoryList" << "clear"));
		}
	}
	else if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			QFile fuelTrackerFile("fueltracker.dat");
			fuelTrackerFile.open(QIODevice::ReadOnly);
			QString fuelTrackerString(fuelTrackerFile.readAll());
			fuelTrackerFile.close();
			QStringList fuelTrackerStringList = fuelTrackerString.split("\r\n");
			if (fuelTrackerStringList.count() > 1)
			{
				for (int i=0;i<fuelTrackerStringList.count() - 1;i++)
				{
					QStringList lineSplit = fuelTrackerStringList[i].split((char)0x24);
					if (lineSplit.count() > 1)
					{
						fuelEntryList.append(FuelEntry(lineSplit[1].toDouble(),lineSplit[2].toDouble(),lineSplit[0].toLong(),lineSplit[3].toDouble()));
					}
					//fuelMilesList.append(QPair<QString,QString>(fuelTrackerStringList[i],fuelTrackerStringList[i+1]));
					//i++;
				}
				rebuildView();
			}
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage("FuelTracker",IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
Q_EXPORT_PLUGIN2(FuelTrackerPlugin, FuelTracker)
