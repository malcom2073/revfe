#ifndef AAMAPI_H
#define AAMAPI_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


class App
{
public:
	QString app_name;
	QString date_added;
	QString date_modified;
	QString description;
	QString dev_id;
	QString dev_name;
	QString image;
	QString price;
	QString product_id;
	QString requirements;
	QString support_url;
	QString thread;
	QString version;
	QString whats_new;
	QString url;
	QString filename;
};



class AAMApi : public QObject
{
	Q_OBJECT
public:
	AAMApi(QObject *parent=0);
	void setUserPass(QString username,QString pass);
	void setPrivateKey(QString key);
	void getAppsByCategory(QString cat);
	void getApp(QString appid);
	void getNumberOfApps(QString user);
	void getCategories();
	void auth();
	void go();
private:
	QList<App> applist;
	QString m_val;
	QString m_current;
	QString m_username;
	QString m_pass;
	QString m_key;
	QString m_signature;
	bool m_authed;
	QNetworkAccessManager *man;
private slots:
	void finished(QNetworkReply *reply);
	void downloadProgress(qint64,qint64);
signals:
	void appList(QList<App> apps);
	void fileDownloadComplete(App app,QString filename);
};

#endif // AAMAPI_H
