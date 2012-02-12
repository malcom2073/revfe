
#include "aamapi.h"
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include "json.h"




AAMApi::AAMApi(QObject *parent) : QObject(parent)
{
	qRegisterMetaType<QList<App> >("QList<App>");
	man = new QNetworkAccessManager();
	m_authed = false;
	connect(man,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
}
void AAMApi::setUserPass(QString username,QString pass)
{
	m_username = username;
	m_pass = pass;
}
void AAMApi::getCategories()
{
	if (m_authed)
	{
		m_current = "categorylist";
		QNetworkReply *rep = man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.private.php?signature=" + m_signature + "&method=getCategories")));
	}
	else
	{
		m_current = "categorylist";
		auth();
	}

}

void AAMApi::setPrivateKey(QString key)
{
	m_key = key;
}
void AAMApi::getApp(QString appid)
{
	if (m_authed)
	{
		m_current = "getapp";
		m_val = appid;
		QNetworkReply *rep = man->get(QNetworkRequest(QUrl(QString("https://autoappmart.com/api/api.private.php?signature=" + m_signature + "&method=getFileDownload&username=") + m_username + "&password=" + m_pass + "&app_id=" + appid)));
		connect(rep,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));
	}
	else
	{
		m_current = "getapp";
		m_val = appid;
		auth();
	}
}
void AAMApi::getAppsByCategory(QString cat)
{
	if (m_authed)
	{
		m_current = "category";
		QNetworkReply *rep = man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.private.php?signature=" + m_signature + "&method=getAppsByCategory&categories=" + cat)));
	}
	else
	{
		m_current = "category";
		m_val = cat;
		auth();
	}
}
void AAMApi::downloadProgress(qint64 received ,qint64 total)
{
	qDebug() << received << total;
}

void AAMApi::auth()
{
	man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/auth.private.php?username=" + m_username + "&password=" + m_pass + "&app_secret=" + m_key)));
}

void AAMApi::getNumberOfApps(QString user)
{
	m_current = "numofapps";
	man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.php?method=getNumberOfApps&username=" + user)));
}

void AAMApi::go()
{
	man = new QNetworkAccessManager();
	m_authed = false;
	connect(man,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
	//man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.php?method=getNumberOfApps&username=optikalefx")));
	//man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.php?method=GetFileDownload&categories=32")));
	man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/auth.private.php?username=malcom2073&password=malco&app_secret=751a4c9baf1f93267719f1e35146aafd")));
	//751a4c9baf1f93267719f1e35146aafd
	//$nvp = "username=$username&password=$pass&app_secret=$app_secret";
	//$auth = json_decode(remoteRequest("https://autoappmart.com/api/auth.private.php",$nvp));

}

void AAMApi::finished(QNetworkReply *reply)
{
	QString type =reply->header(QNetworkRequest::ContentTypeHeader).toString();
	//qDebug() << type;
	if (type != "text/html")
	{
		//This is likely the end of a file download.
		App tmpapp;
		bool found = false;
		for (int i=0;i<applist.size();i++)
		{
			if (applist[i].app_name == m_val)
			{
				found = true;
				tmpapp = applist[i];
			}
		}
		if (!found)
		{
			qDebug() << "File download complete for file that is not in the app list!!!";
			qDebug() << m_val;
			return;
		}
		QFile test(tmpapp.filename);
		test.open(QIODevice::ReadWrite);
		test.write(reply->readAll());
		test.close();

		fileDownloadComplete(tmpapp,QDir(QDir::currentPath()).absoluteFilePath(test.fileName()));
		return;
	}
	QString replystr = reply->readAll();
	qDebug() << replystr;

	bool ok;
	QVariantMap map = Json::parse(replystr,ok).toMap();
	if (!ok)
	{
		qDebug() << "Fatal error during Json parsing";
		return;
	}
	if (map.value("error").toBool())
	{
		qDebug() << "Error in json reply";
		qDebug() << replystr;
	}

	if (!m_authed)
	{
		if (!map.value("success").toBool())
		{
			qDebug() << "Success in json reply returned false";
			return;
		}
		//qDebug() << "Success!" << map.value("signature").toString();
		m_signature = map.value("signature").toString();
		if (m_signature == "")
		{
			qDebug() << "No signature!";
			return;
		}
		m_authed = true;
		if (m_current == "category")
		{
			getAppsByCategory(m_val);
		}
		else if (m_current == "getapp")
		{
			getApp(m_val);
		}
		else if (m_current == "categorylist")
		{
			getCategories();
		}
		//man->get(QNetworkRequest(QUrl("https://autoappmart.com/api/api.private.php?signature=" + sig + "&method=getAppsByCategory&categories=22")));
		//$nvp = ;
	}
	else
	{
		//qDebug() << m_current;
		if (m_current == "category")
		{
			if (map.contains("apps"))
			{
				QVariantList apps = map.value("apps").toList();
				QList<App> appslist;
				for (int j=0;j<apps.size();j++)
				{
					App a;
					QVariantMap appsmap = apps[j].toMap();
					QList<QString> keys = appsmap.keys();
					for (int i=0;i<keys.size();i++)
					{
						if (keys[i] == "app_name")
						{
							a.app_name = appsmap[keys[i]].toString();
						}
						if (keys[i] == "date_added")
						{
							a.date_added = appsmap[keys[i]].toString();
						}
						if (keys[i] == "date_modified")
						{
							a.date_modified = appsmap[keys[i]].toString();
						}
						if (keys[i] == "description")
						{
							a.description = appsmap[keys[i]].toString();
						}
						if (keys[i] == "dev_id")
						{
							a.dev_id = appsmap[keys[i]].toString();
						}
						if (keys[i] == "dev_name")
						{
							a.dev_name = appsmap[keys[i]].toString();
						}
						if (keys[i] == "image")
						{
							a.image = appsmap[keys[i]].toString();
						}
						if (keys[i] == "price")
						{
							a.price = appsmap[keys[i]].toString();
						}
						if (keys[i] == "products_id")
						{
							a.product_id = appsmap[keys[i]].toString();
						}
						if (keys[i] == "requirements")
						{
							a.requirements = appsmap[keys[i]].toString();
						}
						if (keys[i] == "support_url")
						{
							a.support_url = appsmap[keys[i]].toString();
						}
						if (keys[i] == "thread")
						{
							a.thread = appsmap[keys[i]].toString();
						}
						if (keys[i] == "version")
						{
							a.version = appsmap[keys[i]].toString();
						}
						if (keys[i] == "whats_new")
						{
							a.whats_new = appsmap[keys[i]].toString();
						}
						//qDebug() << keys[i] << appsmap[keys[i]].toString();
					}
					applist.append(a);
				}
				//here
				emit appList(applist);
			}
		}
		else if (m_current == "getapp")
		{
			//qDebug() << "Got App:" << replystr;
			QString name = map.value("name").toString();
			QString filename = map.value("filename").toString();
			QString url = map.value("url").toString();
			//qDebug() << name << filename << url;
			m_val = name;
			for (int i=0;i<applist.size();i++)
			{
				if (applist[i].app_name == name)
				{
					applist[i].filename = filename;
					applist[i].url = url;
					break;
				}
			}
			QNetworkReply *rep = man->get(QNetworkRequest(QUrl(QString(url))));
			connect(rep,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));
		}
		else if (m_current == "categorylist")
		{

		}
		else if (m_current == "numofapps")
		{

		}
	}
}
