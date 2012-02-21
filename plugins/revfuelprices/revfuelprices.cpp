 /***************************************************************************
*   Copyright (C) 2010 by Michael Carpenter (malcom2073)                  *
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

#include "revfuelprices.h"
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QClipboard>
#include <QXmlStreamReader>



/*
Fuel Prices Plugin
This plugin has the ability to retreive a list of fuel stations nearby (by either zip code or by
gps coordinates) allowing you to select the stations with the cheapest fuel. The primary functions
for this plugin are:

- FuelPrices:fuel get-zip !ZIPCODE!
	Gets list of fuel prices by zip code
- FuelPrices:fuel get-gps
	Gets list of fuel prices by gps. You must have an active GPS fix or this function will fail

 Skin notes:
Labels:
- %FUEL-ZIPCODE%
	Current zip code
- %FUEL-ADDRESS%
	Address currently selected from the list
Tables:
- FuelPrices:PriceList
	5 column variable row list to display fuel prices.
	columns are: Name, Address, Regular, Midgrade, and Premium.
*/




FuelPrices::FuelPrices()
{
	m_name = "RevFuelPrices";
}


//Init function. Gets called when the plugin is first created.
//Do not pass any core messages, this is purely for private in-class 
//initializations
void FuelPrices::init()
{
	currentLat = 0;
	currentLon = 0;
	reqType = 0;
	manager = new QNetworkAccessManager(this);

	connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));
}

//Has to return the class name
QString FuelPrices::getName()
{
	return m_name;
}

//Unused. Will be removed in future versions
void FuelPrices::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}

//Function for receiving messages from the core and other plugins
//Arguments:
//sender - Name of class that sent the message
//message - IPCmessage of the actual message sent
void FuelPrices::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender);
	//Most if not all functions are a part of the fuel class
	if (message.getClass() == "fuel")
	{
		//Retrieves fuel based on zip code. First argument should be a zip code, eg: QStringList << "90210";
		if (message.getMethod() == "get-zip")
		{
			//while (true);
			//qDebug() << "About to get fuel prices by zip code";
			getStations(message.getArgs()[0]);
			//qDebug() << "Getting fuel prices by zip code";
		}
		//set's the zip code in-form, and retreives the list of fuel prices.
		else if (message.getMethod() == "setzip")
		{
			getStations(message.getArgs()[0]);
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%FUEL-ZIPCODE%" << message.getArgs()[0]));
			//qDebug() << "Zip:" << message.getArgs()[0];
		}
		//Retreives fuel prices based on gps coordinates. This first retreives a zip code based on the current location, and then fuel prices based on that.
		else if (message.getMethod() == "get-gps")
		{
			qDebug() << "Get Gps:";
			if (currentLat != 0 && currentLon != 0)
			{
				getStations(currentLat,currentLon);
			}
		}
		//This is the function called when someone clicks on a fuel point address in the listbox. This sets the address to the clipboard for easy navigation.
		else if (message.getMethod() == "address")
		{
			//qDebug() << "Setting address";
			int num = message.getArgs()[0].toInt()-1;
			if (num > 0 && num < gasStations.count())
			{
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%FUEL-ADDRESS%" << gasStations[num]->location));
				emit passCoreMessage(m_name,IPCMessage("core","clipboard","copy",QStringList() << gasStations[num]->location));
				//qDebug() << "Address:" << gasStations[num]->location;
				//QApplication::clipboard()->setText(gasStations[num]->location);
			}
		}
		
	}
	else if (message.getClass() == "event")
	{
		if (message.getMethod() == "throw")
		{
			//We signed up for gps coodinate updates to keep our position up to date!
			if (message.getArgs()[0] == "event:gps_posupdate")
			{
				currentLat = message.getArgs()[1].toDouble();
				currentLon = message.getArgs()[2].toDouble();
				qDebug() << "Location changed!:" << currentLat << currentLon;
			}
		}
		if (message.getMethod() == "initstarted")
		{
			QStringList list;
			list << "event:propertycomplexlistset";
			list << "FuelPricesList";
			list << "5";
			list << "Station_Name";
			list << "Station_Address";
			list << "Regular";
			list << "Midgrade";
			list << "Premium";
			list << "0";
			//qDebug() << "Emitting Fuel Prices";

			//fuelModel = new ComplexModel();
			//QList<QList<QString> > data;
			QHash<int,QByteArray> roles;
			//QStringList roles;
			roles[0] = "text";
			roles[1] = "Station_Name";
			roles[2] = "Station_Address";
			roles[3] = "Regular";
			roles[4] = "Midgrade";
			roles[5] = "Premium";
			//fuelModel->setRoles(roles);
			//fuelModel->setData(data);
			//*/
			//QHash<int,QByteArray> roles;
			//roles.insert(0,"Station_Name");
			fuelModel = new StandardModel(roles);
			//fuelModel->setRoleNames(roles);
			//fuelModel->setHorizontalHeaderLabels(roles);

			emit passCoreModel("FuelPricesList",fuelModel);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));

		}
		//Sign up for any events here
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","subscribe",QStringList() << "event:gps_posupdate"));
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
}
//Function to retreive a list of fuel stations based on zip code.
void FuelPrices::getStations(QString zipcode)
{
	reqType = 1;
	manager->get(QNetworkRequest(QUrl("http://www.automotive.com/gas-prices/results.html?zip=" + zipcode)));
}
//Function to retreive a list of fuel statiosn based on gps coodinates.
void FuelPrices::getStations(int lat, int lon)
{
	reqType = 2;
	manager->get(QNetworkRequest(QUrl(QString("http://ws.geonames.org/findNearbyPostalCodes?lat=").append(QString::number(lat)).append("&lng=").append(QString::number(lon)))));
}
void FuelPrices::replyFinished(QNetworkReply* reply)
{
	qDebug() << "Network reply for fuelprices";
	if (reqType == 1)
	{
		QString rep = reply->readAll();
		//qDebug() << rep;
		if (reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString() != "")
		{
			manager->get(QNetworkRequest(QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString())));
		}
		else
		{
			int initial = rep.indexOf("<div id=\"TABLE_GRID_"); //57\" class=\"pad_l pad_r\"  >");
			//int start = rep.indexOf("<b>",initial);
			//int end = rep.indexOf("</b>",initial);
			//int initialdate = rep.indexOf("<td id=\"ctl00_PlaceHolderWideTopColumn_ctl00_ctl00_tdCopy\" class=\"sz11 capitalize w560\">",initial) + 102;
			int enddate = rep.indexOf("<tr>",initial);
			int starter = enddate;
			bool good = true;
			gasStations.clear();
			int count = 0;
			while (good)
			{
				count++;
				//good = false;
				int startfuel = rep.indexOf("<td class=\"brdr1_r brdr1_t\">",starter);
				if (startfuel == -1)
				{
					good = false;
					continue;
				}
				GasStation *station = new GasStation();

				int endfuel = rep.indexOf("</td>\r\n",startfuel);
				QString entry = rep.mid(startfuel,endfuel-startfuel);
				entry = entry.replace("\r","");
				entry = entry.replace("\n","");
				//qDebug() << "Fuel:" << entry;
				int startname = entry.indexOf("<span class='b clr2'>")+22;
				int endname = entry.indexOf("</span>",startname)-1;
				station->name = entry.mid(startname,endname - startname);
				int startaddress = entry.indexOf("<br>",endname) + 5;
				int endaddress = entry.indexOf("<br>",startaddress);
				station->location = entry.mid(startaddress,endaddress-startaddress);
				int startcost = entry.indexOf("<td class=\"ctr pad5 b brdr6_nb clr2\">",endaddress) + 37;
				int endcost = entry.indexOf("</td>",startcost);
				station->reg = entry.mid(startcost,endcost-startcost);
				int startmidcost = entry.indexOf("<td class=\"ctr pad5 b brdr6_nb clr2\">",endcost)+37;
				int endmidcost = entry.indexOf("</td>",startmidcost);
				station->mid = entry.mid(startmidcost,endmidcost-startmidcost);
				int startpremcost = entry.indexOf("<td class=\"ctr pad5 b brdr6_nb clr2\">",endmidcost)+37;
				int endpremcost = entry.indexOf("</td>",startpremcost);
				station->pre = entry.mid(startpremcost,endpremcost-startpremcost);
				starter = endfuel;
				gasStations.append(station);
				if (count > 5000)
				{
					qDebug() << "timeout error on fuel prices.";
					good = false;
				}
			}
			if (gasStations.count() == 0)
			{
				qDebug() << "RevFuelPrices: No Station data...";
				//fuelModel->clearContents();
				fuelModel->clear();
				QStringList list;
				list << "event:listchange";
				list << "FuelPrices:PriceList";
				list << "newlist";
				list << "2";
				list << "5";
				QStringList tmpNameList = QStringList() << "Name" << "Address" << "Regular" << "Midgrade" << "Premium";
				for (int i=0;i<tmpNameList.count();i++)
				{
					list << tmpNameList[i];
					list << "No Data";
				}
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));
			}
			else
			{
				qDebug() << "RevFuelPrices: " << gasStations.count() << "stations found";
				/*QStringList list;
				list << "event:propertycomplexlistset";
				list << "FuelPricesList";
				list << "5";
				list << "Station_Name";
				list << "Station_Address";
				list << "Regular";
				list << "Midgrade";
				list << "Premium";
				list << QString::number(gasStations.count());
				QStringList tmpNameList = QStringList() << "Name" << "Address" << "Regular" << "Midgrade" << "Premium";
				*/
				fuelModel->clear();
				//fuelModel->setRowCount(gasStations.count());
				for (int j=0;j<gasStations.count();j++)
				{
					QStandardItem *item = new QStandardItem();
					//qDebug() << "Station" << gasStations[j]->name << gasStations[j]->location << gasStations[j]->reg << gasStations[j]->mid << gasStations[j]->pre;
					QStringList list;
					item->setData(gasStations[j]->name,1);
					item->setData(gasStations[j]->location,2);
					item->setData(gasStations[j]->reg,3);
					item->setData(gasStations[j]->mid,4);
					item->setData(gasStations[j]->pre,5);
					/*list << gasStations[j]->name;
					list << gasStations[j]->location;
					list << gasStations[j]->reg;
					list << gasStations[j]->mid;
					list << gasStations[j]->pre;*/
					//qDebug() << "ADding row:" << list;
					//fuelModel->addRow(list);
					//fuelModel->setItem(j,0,new QStandardItem(gasStations[j]->name));
					fuelModel->appendRow(item);
				}
				//qDebug() << "Emitting Fuel Prices";
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));
			}
		reqType = 0;
		}
	}
	else if (reqType == 2)
	{
		QString repStr = reply->readAll();
		QXmlStreamReader xml(repStr);
		while (!xml.atEnd())
		{
			if (xml.name() == "code" && xml.isStartElement())
			{
				xml.readNext();
				while (xml.name() != "code")
				{
					if (xml.name() == "postalcode" && xml.isStartElement())
					{
						xml.readNext();
						qDebug() << "Name:" << xml.text().toString();
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%FUEL-ZIPCODE%" << xml.text().toString()));
					}
					xml.readNext();
				}
			}
			xml.readNext();
		}
		reqType = 0;
	}
//	reply->deleteLater();
}
Q_EXPORT_PLUGIN2(FuelPricesPlugin, FuelPrices)
