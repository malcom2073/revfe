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

#include "revautoappmart.h"
#include <QDebug>
#include <QSslError>
#include <QProcess>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
AutoAppMart::AutoAppMart()
{
	m_name = "RevAutoAppMart";
}

void AutoAppMart::init()
{
	qRegisterMetaType<QModelIndex>("QModelIndex");
	api = new AAMApi(this);
	connect(api,SIGNAL(appList(QList<App>)),this,SLOT(appList(QList<App>)));
	connect(api,SIGNAL(fileDownloadComplete(App,QString)),this,SLOT(fileDownloadComplete(App,QString)));

	//appListModel = new ComplexModel();

	QHash<int,QByteArray> roles;

	roles[0] = ".";
	roles[1] = "app_name";
	roles[2] = "date_added";
	roles[3] = "date_modified";
	roles[4] = "description";
	roles[5] = "dev_id";
	roles[6] = "dev_name";
	roles[7] = "image";
	roles[8] = "price";
	roles[9] = "product_id";
	roles[10] = "requirements";
	roles[11] = "support_url";
	roles[12] = "thread";
	roles[13] = "version";
	roles[14] = "whats_new";
	appListModel = new StandardModel(roles);
	/*appListModel->setRoles(list);
	QList<QList<QString> > data;
	appListModel->setData(data);
*/

}
QString AutoAppMart::getName()
{
	return m_name;
}
void AutoAppMart::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);

}
void AutoAppMart::passPluginMessage(QString sender,IPCMessage message)
{
	Q_UNUSED(sender)
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			QString user = "";
			QString pass = "";
			for (int i=0;i<message.getArgs().count();i++)
			{
				QString key = message.getArgs()[i];
				QString value = message.getArgs()[i+1];
				QString type = message.getArgs()[i+2];
				QString id = message.getArgs()[i+3];
				i++;
				i++;
				i++;
				//qDebug() << "Auto App Mart settings:" << key << value << type << id;

				if (key.split("=")[1] == "username")
				{
					user = value.split("=")[1];
				}
				else if (key.split("=")[1] == "password")
				{
					pass = value.split("=")[1];
				}
			}
			api->setUserPass(user,pass);
			api->setPrivateKey("751a4c9baf1f93267719f1e35146aafd");
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreModel("autoappmartapps",appListModel);
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "autoappmart_status" << "Idle"));
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
		else if (message.getMethod() == "throw")
		{
		}
	}
	else if (message.getClass() == "api")
	{
		if (message.getMethod() == "get-category")
		{
			if (message.getArgs().size() > 0)
			{
				qDebug() << "Request to get categories";
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "autoappmart_status" << "Requesting category: " + message.getArgs()[0]));
				QString cat = message.getArgs()[0];
				api->getAppsByCategory(cat);
			}
		}
		else if (message.getMethod() == "get-category-list")
		{
			api->getCategories();
		}
		else if (message.getMethod() == "download")
		{
			if (message.getArgs().size() > 0)
			{
				QString id = message.getArgs()[0];
				qDebug() << "Request to download app id:" << id;
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "autoappmart_status" << "Downloading..."));
				api->getApp(id);
			}
		}
	}
}
void AutoAppMart::appList(QList<App> apps)
{
	appListModel->clear();
	//QList<QList<QString> > modellist;
	for (int i=0;i<apps.size();i++)
	{
		QStandardItem *item = new QStandardItem();
		item->setData(apps[i].app_name,1);
		item->setData(apps[i].date_added,2);
		item->setData(apps[i].date_modified,3);
		item->setData(apps[i].description,4);
		item->setData(apps[i].dev_id,5);
		item->setData(apps[i].dev_name,6);
		item->setData(apps[i].image,7);
		item->setData(apps[i].price,8);
		item->setData(apps[i].product_id,9);
		item->setData(apps[i].requirements,10);
		item->setData(apps[i].support_url,11);
		item->setData(apps[i].thread,12);
		item->setData(apps[i].version,13);
		item->setData(apps[i].whats_new,14);
		appListModel->appendRow(item);
	}

	//appListModel->setData(modellist);
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "autoappmart_status" << QString("Categories Done. ") + QString::number(apps.size()) + " apps retrieved"));
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",list));
}

void AutoAppMart::fileDownloadComplete(App app, QString filename)
{
	Q_UNUSED(app)
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "autoappmart_status" << "Download complete"));
	emit passCoreMessage(m_name,IPCMessage("RevPluginManager","plugin","install",QStringList() << filename));
}

Q_EXPORT_PLUGIN2(RevAutoAppMartPlugin, AutoAppMart)
