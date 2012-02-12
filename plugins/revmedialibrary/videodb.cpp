#include "videodb.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
VideoDB::VideoDB(QObject *parent) : QObject(parent)
{

}

QList<QString> VideoDB::addDirectory(QString dir)
{
	Q_UNUSED(dir)
	m_fileList.clear();
	QSqlDatabase dataBase;
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	dataBase.setDatabaseName("video.db");
	if (!dataBase.open())
	{
		qDebug() << "Error opening database";
		return QList<QString>();
	}
	if (!dataBase.transaction())
	{
		qDebug() << "Error opening db for transaction";
	}

	QSqlQuery result(dataBase);

	if (!result.prepare("select * from 'Table_Index';"))
	{
		qDebug() << "VideoDB.cpp: Error with select Table_Index. Creating new Table_Index" << result.lastError().text();;
		QSqlQuery createTable(dataBase);
		if (!createTable.prepare("create table 'Table_Index' (tiKey INTEGER PRIMARY KEY, title TEXT, length TEXT);"))
		{
			qDebug() << "VideoDB.cpp: Error preparing create table 'Table_Index':" << createTable.lastError().text();
			return QList<QString>();
		}
		if (!createTable.exec())
		{
			qDebug() << "VideoDB.cpp: Error execing create table 'Table_Index'" << createTable.lastError().text();
			return QList<QString>();
		}
	}

	if (!dataBase.commit())
	{
		qDebug() << "Error MediaDB.cpp line 131";
		return QList<QString>();

	}
	dataBase.close();
	return QList<QString>();
}
