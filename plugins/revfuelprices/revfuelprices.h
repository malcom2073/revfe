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

#include <QApplication>
#include <QTimer>
#include <QProcess>
#include "baseinterface.h"
#include "ipcmessage.h"
#include "standardmodel.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardItemModel>
class FuelPrices : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)

public:
	FuelPrices();
	void init();
	QString getName();
	class GasStation
	{
		public:
		QString name;
		QString reg;
		QString mid;
		QString pre;
		QString location;
	};
private:
	//ComplexModel *fuelModel;
	StandardModel *fuelModel;
	QString m_name;
	QList<QProcess*> processList;
	QNetworkAccessManager *manager;
	double currentLat;
	double currentLon;
	void getStations(QString zipcode);
	void getStations(int lat, int lon);
	QList<GasStation*> gasStations;
	int reqType;
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
public slots:
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
private slots:
	void replyFinished(QNetworkReply* reply);
};
