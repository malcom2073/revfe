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

#include "mediadb.h"
#include <QDateTime>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>
#include <QThread>
#include <QDebug>
MediaDB::MediaDB(QObject *parent) : QObject(parent)
{
	open = false;
	connect(this,SIGNAL(updateEntrySignal(QString,int,QString,QString,QString,QString,QString,int)),this,SLOT(private_updateEntry(QString,int,QString,QString,QString,QString,QString,int)),Qt::DirectConnection);
}
QList<QString> MediaDB::add(QString dir)
{
	qDebug() << "MediaDB::add" << dir;
	m_mediaList.clear();
	m_stringlist.clear();
	QSqlDatabase dataBase;
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	dataBase.setDatabaseName("test.db");
	if (!dataBase.open())
	{
		qDebug() << "Error opening database";
		return QList<QString>();
	}
	if (!dataBase.transaction())
	{
		qDebug() << "Error opening db for transaction";
	}
	enterDirectory(dir,dir);
	qDebug() << "Media_List Scan Complete" << dir << m_mediaList.size() << "files found";
	if (!dataBase.commit())
	{
		debugFile->write(QString("Error commiting DB\r\n").toStdString().c_str());
		debugFile->flush();
		qDebug() << "Error MediaDB.cpp line 57";
		return QList<QString>();

	}
	dataBase.close();
	emit mediaList(m_mediaList);
	//emit mediaList(m_stringlist);
	return m_stringlist;

}

QList<QString> MediaDB::init()
{
	QSqlDatabase dataBase;
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	dataBase.setDatabaseName("test.db");
	if (!dataBase.open())
	{
		qDebug() << "Error opening database";
		return QList<QString>();
	}
	if (!dataBase.transaction())
	{
		qDebug() << "Error opening db for transaction";
	}
	//QString musicDir = dir;
	//QDir tmpDir(musicDir);
	//tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	//QDir tmpFiles(musicDir);
	//tmpFiles.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	//QStringList infoList = tmpDir.entryList();
	//QSqlRecord tableIndexRecord = dataBase.record("Table_Index");
	QSqlQuery result(dataBase);
	bool table = true;
	qDebug() << "Testing for valid Media_List in database";
	if (!result.prepare("select * from 'Media_List';"))
	{
		qDebug() << "Invalid prepare on Media_List select:" << result.lastError().databaseText();
		//return;
	}
	if (!result.exec())
	{
		qDebug() << "No valid Media_List found! Need to add directories!" << result.lastError().databaseText();
		table = false;
	}
	else
	{
		qDebug() << "Valid Media_Index found! Not rescanning";
	}
	if (!table)
	{
		//No table created.
		table = false;
		//Nothing loaded, worthy of an initial rescan
		QSqlQuery tableCreator(dataBase);
		if (!tableCreator.prepare(QString("create table '").append("Media_List").append("' (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER);")))
		{
			qDebug() << "Error preparing Media_List table create";
			return QList<QString>();
		}
		//tableCreater.prepare("create table 'Media_List' (tiKey INTEGER PRIMARY KEY, title TEXT);");
		if (!(tableCreator.exec()))
		{
			qDebug() << "Error execing create table Media_List" << tableCreator.lastError().text();
			return QList<QString>();
		}
		if (!tableCreator.prepare(QString("create table 'Playlist_List' (tikey INTEGER PRIMARY KEY, title TEXT, songs TEXT);")))
		{
			qDebug() << "Error preparing create table Playlist_List";
			return QList<QString>();
		}
		if (!tableCreator.exec())
		{
			qDebug() << "Error Execing create table Playlist_List";
			return QList<QString>();
		}
		//qDebug() << "FINDING TOTAL NUMBER OF FILES";
		//int num = fileCount(tmpFiles.absolutePath());
		//qDebug() << "TOTAL NUMBER OF FILES:" << num;
		qDebug() << "Table created";
		if (!dataBase.commit())
		{
			debugFile->write(QString("Error commiting DB\r\n").toStdString().c_str());
			debugFile->flush();
			qDebug() << "Error MediaDB.cpp line 131";
			return QList<QString>();

		}
		dataBase.close();

		return QList<QString>();
		//enterDirectory(tmpFiles.absolutePath(),tmpFiles.absolutePath());
		qDebug() << "Media_List Scan Complete";
		if (!dataBase.commit())
		{
			debugFile->write(QString("Error commiting DB\r\n").toStdString().c_str());
			debugFile->flush();
			qDebug() << "Error MediaDB.cpp line 131";
			return QList<QString>();

		}
		dataBase.close();
		emit mediaList(m_mediaList);
		//emit mediaList(m_stringlist);
		return m_stringlist; //The first time you scan in files, you need to let MediaDBThread scan through them
	}
	else
	{
		if (!result.exec())
		{
			//error here
			return QList<QString>();
			qDebug() << "Error MediaDB.cpp line 122";
		}
	}
	m_stringlist.clear();
	result.clear();
	if (!result.prepare("select * from 'Media_List';"))
	{
		debugFile->write(QString("Error preparing select from table\r\n").toStdString().c_str());
		debugFile->flush();
		return QList<QString>();
		//QMessageBox::information(0,"Error","Error opening DB");
	}
	if (!result.exec())
	{
		qDebug() << "Error executing select from Media_List";
		return QList<QString>();
	}
	QList<QString> needupdateslist;
	//QList<QString> namelist;
	//QList<ID3Tag> taglist;
	while (result.next())
	{
		ID3Tag *tag = new ID3Tag();
		int titleFieldNo = result.record().indexOf("title");
		int artistFieldNo = result.record().indexOf("artist");
		int albumFieldNo = result.record().indexOf("album");
		//int filenameFieldNo = result.record().indexOf("filename");
		int fullpathFieldNo = result.record().indexOf("fullPath");
		int lastupdate = result.record().indexOf("lastupdate");
		//ID3Tag *id3 = new ID3Tag();
		tag->valid = true;
		QString fullpath = result.value(fullpathFieldNo).toString();
		tag->filename = fullpath;
		//tmpPlayListClass->addFile(fullpath);
		tag->title = result.value(titleFieldNo).toString();
		tag->artist = result.value(artistFieldNo).toString();
		tag->album = result.value(albumFieldNo).toString();
		tag->lastupdate = result.value(lastupdate).toInt();
		//tmpPlayListClass->setId3(count,id3);
		//count++;
		MediaFileClass *media = new MediaFileClass();
		media->setId3Tag(tag);
		media->setFullPath(fullpath);
		m_mediaList.append(media);
		m_stringlist.append(fullpath);
		if (tag->lastupdate == 0)
		{
			needupdateslist.append(fullpath);
		}
		else
		{
			//namelist.append(fullpath);
			//taglist.append(tag);
			//tagReceived(fullpath,tag);
		}
	}
	emit mediaList(m_mediaList);
	//emit tagsReceived(namelist,taglist);
	return needupdateslist;
}
int MediaDB::fileCount(QString dir)
{
	int count = 0;
	QDir tmpDir(dir);
	tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(QString newdir,tmpDir.entryList())
	{
		count += fileCount(tmpDir.absoluteFilePath(newdir));
	}
	tmpDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	//QFileInfoList tmpFilesInfoList = tmpDir.entryInfoList();
	//if (tmpFilesInfoList.count() == 0)
	//{
	//	return;
	//}
	return tmpDir.entryList().size();
}

bool  MediaDB::init(QString dir,QList<PlayListClass*> *playListClassList)
{
	QSqlDatabase dataBase;
	debugFile = new QFile("debug.log");
	debugFile->open(QIODevice::ReadWrite);

	//int rc = sqlite3_open("test.db",&dataBase);
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	QString debugLoc = "~/.revfe";
	debugLoc = debugLoc.replace("~",getenv("HOME"));
	if (QFile::exists(debugLoc))
	{
		qDebug() << "Using ~/.revfe for database file";
		//QFile::remove(debugLoc  + "/test.db");
		dataBase.setDatabaseName(debugLoc + "/test.db");
	}
	else
	{
		qDebug() << "Using application location for database file";
		//QFile::remove("test.db");
		dataBase.setDatabaseName("test.db");
	}
	//dataBase.transaction();
	open = true;
	if (!dataBase.open())
	{
		open = false;
		QString info = dataBase.lastError().text();
		//return;
		//QMessageBox::information(0,"Error","Error opening DB");
		qDebug() << "Error opening db";
	}
	if (!dataBase.transaction())
	{
		QString info = dataBase.lastError().text();
		//return;
		//QMessageBox::information(0,"Error","Error opening DB");
		qDebug() << "Error opening db for transaction";
	}
	//QFile tmpFile("music.conf");
	//if (!tmpFile.open(QIODevice::ReadOnly | QIODevice::Text))
	//{
	//	printf("Error opening configuration file!!!\n");
	//	return;
	//}
	//QString musicDir(tmpFile.readLine());
	//printf("Music Directory from configuration file: %s\n",musicDir.toStdString().c_str());
	//tmpFile.close();
	//musicDir = musicDir.mid(0,musicDir.length()-1);
	QString musicDir = dir;
	QDir tmpDir(musicDir);
	tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QDir tmpFiles(musicDir);
	tmpFiles.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList infoList = tmpDir.entryList();
	//QSqlRecord tableIndexRecord = dataBase.record("Table_Index");
	QSqlQuery result(dataBase);
	bool table = true;
	qDebug() << "Testing for valid Table_Index in database";
	if (!result.prepare("select * from 'Table_Index';"))
	{
		qDebug() << "Invalid prepare on Table_Index select:" << result.lastError().databaseText();
		//return;
	}
	if (!result.exec())
	{
		qDebug() << "No valid Table_Index found! Rescanning" << result.lastError().databaseText();
		table = false;
	}
	else
	{
		qDebug() << "Valid Table_Index found! Not rescanning";
	}
	//if (!result.prepare("select * from 'Table_Index';"))
	//{

	//}
	if (!table)
	{
		//result.clear();
		//No table created.
		table = false; 
		//printf("Error\n");
		//Nothing loaded, worthy of an initial rescan
		QSqlQuery tableCreater(dataBase);
		tableCreater.prepare("create table 'Table_Index' (tiKey INTEGER PRIMARY KEY, title TEXT);");
		if (!(tableCreater.exec()))
		{
			return false;
			//QMessageBox::information(0,"Error","Error opening DB");
			qDebug() << "Error MediaDB.cpp line 112";
		}
		enterDirectory(tmpFiles.absolutePath(),tmpFiles.absolutePath());
		qDebug() << "Table_Index Scan Complete";
	} 
	else
	{
		if (!result.exec())
		{
			//error here
			return false;
			qDebug() << "Error MediaDB.cpp line 122";
		}
	}

	result.clear();
	if (!dataBase.commit())
	{
		debugFile->write(QString("Error commiting DB\r\n").toStdString().c_str());
		debugFile->flush();
		qDebug() << "Error MediaDB.cpp line 131";
		return false;

	}
	if (!result.prepare("select * from 'Table_Index';"))
	{
		debugFile->write(QString("Error preparing select from table\r\n").toStdString().c_str());
		debugFile->flush();
		return false;
		//QMessageBox::information(0,"Error","Error opening DB");
	}
	if (!result.exec())
	{

	}
	QSqlQuery query(dataBase);
	int totalPlaylistCount=0;
	while (result.next())
	{
		totalPlaylistCount++;
		PlayListClass *tmpPlayListClass = new PlayListClass();
		int pltitleFieldNo = result.record().indexOf("title");
		QString title = result.value(pltitleFieldNo).toString();
		tmpPlayListClass->setTitle(title);
		//if (sqlite3_get_table(dataBase,QString("select * from '").append(tableList[i].toStdString().c_str()).append("';").toStdString().c_str(),&result,&nrow,&ncol,&zErrMsg) != SQLITE_OK)
		query.bindValue(0,title);
		query.exec(QString("select * from '").append(title).append("';"));
		////(tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER)
		int count=0;
		while (query.next())
		{
			int titleFieldNo = query.record().indexOf("title");
			int artistFieldNo = query.record().indexOf("artist");
			int albumFieldNo = query.record().indexOf("album");
			//int filenameFieldNo = query.record().indexOf("filename");
			int fullpathFieldNo = query.record().indexOf("fullPath");
			int lastupdate = query.record().indexOf("lastupdate");
			ID3Tag *id3 = new ID3Tag();
			id3->valid = false;
			QString fullpath = query.value(fullpathFieldNo).toString();
			id3->filename = fullpath;
			tmpPlayListClass->addFile(fullpath);
			id3->title = query.value(titleFieldNo).toString();
			id3->artist = query.value(artistFieldNo).toString();
			id3->album = query.value(albumFieldNo).toString();
			id3->lastupdate = query.value(lastupdate).toInt();
			tmpPlayListClass->setId3(count,id3);
			count++;
		}
		playListClassList->push_back(tmpPlayListClass);
	}

	query.clear();
	dataBase.commit();
	dataBase.close();
	QSqlDatabase::removeDatabase("QSQLITE");
	open = false;
	debugFile->close();
	//delete dataBase;
	qDebug() << "SQL Database access complete";
	return true;
}
void MediaDB::enterDirectory(QString musicdir,QString dir)
{
	QSqlDatabase dataBase = QSqlDatabase::database("QSQLITE");
	//debugFile->write(QString("Music: ").append(musicdir).append(" Dir: ").append(dir).append("\r\n").toStdString().c_str());
	//debugFile->flush();
	QDir tmpDir(dir);
	tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList tmpDirInfoList = tmpDir.entryInfoList();
	foreach(QFileInfo dirInfo,tmpDirInfoList)
	{
		enterDirectory(musicdir,dirInfo.absoluteFilePath());
	}
	tmpDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QFileInfoList tmpFilesInfoList = tmpDir.entryInfoList();
	if (tmpFilesInfoList.count() == 0)
	{
		return;
	}
	QSqlQuery tableInsert(dataBase);
	QSqlQuery tableFind(dataBase);
	//QSqlQuery songInsert(dataBase);
	//QSqlQuery tableCreate(dataBase);
	//QString tableName ="";
	//if (musicdir.length() >= dir.length())
	//{
	//	tableName = "/";
	//}
	//else
	//{
	//	tableName = dir.mid(musicdir.length()+1).replace("/","-");//tmpFiles.absolutePath().mid(tmpFiles.absolutePath().lastIndexOf("/")+1);
	//}
	//debugFile->write(QString("Table Inserted").toStdString().c_str());
	//tableName.replace("'","''");
	if (!tableInsert.prepare(QString("insert into 'Media_List' (title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?);")))
	{
		//QMessageBox::information(0,"Error","Error opening DB");
		qDebug() << "Error MediaDB.cpp line 458" << tableInsert.lastError().text();
		return;
	}
	//tableCreate.prepare(QString("create table '").append(tableName).append("' (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER);"));
	//if (!tableCreate.exec())
	//{
	//	int error =1;
	//	return;
	//	QMessageBox::information(0,"Error","Error opening DB");
	//}
	//if (!songInsert.prepare(QString("insert into '").append(tableName).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?,?);")))
	//{
	//	int error = 1;
	//	return;
	//	QMessageBox::information(0,"Error","Error opening DB");
	//}
	int tracknum = 0;
	foreach(QFileInfo fileInfo,tmpFilesInfoList)
	{
		
		QString filename = "";
		if (fileInfo.absoluteFilePath().lastIndexOf("/") < fileInfo.absoluteFilePath().length())
		{
			filename = fileInfo.absoluteFilePath().mid(fileInfo.absoluteFilePath().lastIndexOf("/")+1);	
		}
		else
		{
			//QMessageBox::information(0,"Error",QString("Error in file path"));
		}
		//debugFile->write(QString("File: ").append(filename).append("\r\n").toStdString().c_str());
		//debugFile->flush();
	//if ((filename.endsWith(".mp3")) || (filename.endsWith(".flac")) || (filename.endsWith(".wma")) || (filename.endsWith(".ogg")))
		bool found = false;
		if ((filename.endsWith(".mp3")) || (filename.endsWith(".wma")) || (filename.endsWith(".flac")) || (filename.endsWith(".ogg")))
		{
				if (!tableFind.exec("select * from 'Media_List' where fullPath is '" + fileInfo.absoluteFilePath().replace("'","''") + "';"))
				{
					qDebug() << "MediaDB.cpp: Error execing select from Media_List" << tableFind.lastError().text();
				}
				if (tableFind.next())
				{
					//If the file is in the database, we do NOT want to reload it.
					//Assuming it is currently in the database, then it will be loaded
					//at the appropriate time into the media list, so just ignore this
					//file entirely
					found = true;
				}
				else
				{

					QString title="title";
					QString artist="artist";
					QString album="album";

					QString fullPath = fileInfo.absoluteFilePath(); //tmpFiles.absolutePath().append("/").append(entries.at(j));
					//QString total = QString("insert into '").append(tableName).append("' (rowid,title, artist, album, filename,fullPath, tracknum,lastupdate) values ('").append(QString::number(j)).append("','").append(title.replace("'","''")).append("','").append(artist.replace("'","''")).append("','").append(album.replace("'","''")).append("','").append(filename.replace("'","''")).append("','").append(fullPath.replace("'","''")).append("',").append(QString::number(tracknum)).append(",").append("0").append(");");

					//tableInsert.bindValue(0,tracknum);
					tableInsert.bindValue(0,title);
					tableInsert.bindValue(1,artist);
					tableInsert.bindValue(2,album);
					if ((fullPath.lastIndexOf("/") < fullPath.length()) && (fullPath.lastIndexOf("/") != -1))
					{
						tableInsert.bindValue(3,fullPath.mid(fullPath.lastIndexOf("/")+1));
					}
					else
					{
						//QMessageBox::information(0,"Error",QString("Error in full path"));
						qDebug() << "Error MediaDB.cpp line 275";
					}
					tableInsert.bindValue(4,fullPath);
					m_stringlist.append(fullPath);

					tableInsert.bindValue(5,tracknum);
					tableInsert.bindValue(6,0);
					tracknum++;
					if (!found)
					{
						MediaFileClass *media = new MediaFileClass();
						ID3Tag *tag = new ID3Tag();
						tag->title = "title";
						tag->artist = "artist";
						tag->album = "album";
						tag->filename = fullPath;
						tag->valid = false;
						tag->lastupdate = 0;
						media->setId3Tag(tag);
						media->setFullPath(fullPath);
						m_mediaList.append(media);
						if (!tableInsert.exec())
						{
		//					QMessageBox::information(0,"Error",QString("Error in songINsert: ").append(errorStr));
							qDebug() << "Error MediaDB.cpp line 287";
						}
					}
				}
				//QString total = "";
				//if (sqlite3_exec(dataBase,total.toStdString().c_str(),0,0,&zErrMsg) != SQLITE_OK)
				//{
				//	printf("Error exec Sql: %s\n",zErrMsg);
				//	sqlite3_free(zErrMsg);
				//}
				//printf("Total Name: %s\n",total.toStdString().c_str());
		}
	}
	
	tableInsert.clear();
	//songInsert.clear();
	//tableCreate.clear();
	return;
}	
void MediaDB::setPlayListPointer(QList<PlayListClass*> &pointer)
{
	Q_UNUSED(pointer);
	//pointer = *playListClassList;
}
int MediaDB::addEntries(QString Target_Table,QList<EntryClass> classlist)
{
	QSqlDatabase dataBase;
	dataBase = QSqlDatabase::database("QSQLITE");
	QSqlQuery createItemQuery(dataBase);
	if (!createItemQuery.prepare(QString("insert into '").append(Target_Table).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?,?);")))
	{
		//QString query = createItemQuery.executedQuery();
		QString error = createItemQuery.lastError().text();
		emit mediaDBError(error.prepend("createItemQuery-insert-prepare in addEntry: "));
	}
	for (int i=0;i<classlist.count();i++)
	{
		createItemQuery.bindValue(0,classlist[i].tracknum);
		createItemQuery.bindValue(1,classlist[i].title);
		createItemQuery.bindValue(2,classlist[i].artist);
		createItemQuery.bindValue(3,classlist[i].album);
		createItemQuery.bindValue(4,classlist[i].filename);
		createItemQuery.bindValue(5,classlist[i].fullPath);
		createItemQuery.bindValue(6,classlist[i].tracknum);
		createItemQuery.bindValue(7,0);
		if (!createItemQuery.exec())
		{
			//QString query = createItemQuery.executedQuery();
			QString error = createItemQuery.lastError().text();
			emit mediaDBError(error.prepend("createItemQuery-exec in addEntries: "));
		}
	}
	return 0;

}
int MediaDB::addEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum)
{
	QSqlDatabase dataBase;
	dataBase = QSqlDatabase::database("QSQLITE");
	QSqlQuery createItemQuery(dataBase);
	if (!createItemQuery.prepare(QString("insert into '").append(Target_Table).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?,?);")))
	{
		//QString query = createItemQuery.executedQuery();
		QString error = createItemQuery.lastError().text();
		emit mediaDBError(error.prepend("createItemQuery-insert-prepare in addEntry: "));
	}
	createItemQuery.bindValue(0,Target_Entry);
	createItemQuery.bindValue(1,title);
	createItemQuery.bindValue(2,artist);
	createItemQuery.bindValue(3,album);
	createItemQuery.bindValue(4,filename);
	createItemQuery.bindValue(5,fullPath);
	createItemQuery.bindValue(6,tracknum);
	createItemQuery.bindValue(7,0);
	if (!createItemQuery.exec())
	{
		//QString query = createItemQuery.executedQuery();
		QString error = createItemQuery.lastError().text();
		emit mediaDBError(error.prepend("createItemQuery-exec in addEntry: "));
	}
	return 0;
}
int MediaDB::createTable(QString Target_Table)
{
	QSqlDatabase dataBase;
	//openDB();
	dataBase = QSqlDatabase::database("QSQLITE");
	if (!dataBase.isOpen())
	{
		emit mediaDBError(dataBase.lastError().text().prepend("DB not open in createTable1: "));
		dataBase = QSqlDatabase::addDatabase("QSQLITE");
		emit mediaDBError(dataBase.lastError().text().prepend("DB not open in createTable2: "));
		dataBase.setDatabaseName("test.db");
		emit mediaDBError(dataBase.lastError().text().prepend("DB not open in createTable3: "));
		dataBase.transaction();
		emit mediaDBError(dataBase.lastError().text().prepend("DB not open in createTable4: "));
		if (!dataBase.open())
		{
			emit mediaDBError(dataBase.lastError().text().prepend("DB not open in createTable5: "));
		}
	}
	QSqlQuery createQuery(dataBase);
	QSqlQuery tableInsert(dataBase);
	if (!tableInsert.exec(QString("delete from 'Table_Index' where title == '").append(Target_Table).append("';")))
	{
		QString error = tableInsert.lastError().text();
		emit mediaDBError(error.prepend("TableInsert-delete in CreateTable: "));
	}
	//if (!tableInsert.exec(QString("insert into 'Table_Index' (title) values ('").append(tableName).append("');")))
	if (!tableInsert.exec(QString("insert into 'Table_Index' (title) values ('").append(Target_Table).append("');")))
	{
		QString error = tableInsert.lastError().text();
		emit mediaDBError(error.prepend("TableInsert-insert in CreateTable: "));
		error = dataBase.lastError().text();
	}
	if (!createQuery.exec(QString("drop table '").append(Target_Table).append("';")))
	{
		QString error = createQuery.lastError().text();
		emit mediaDBError(error.prepend("createQuery-drop in CreateTable: "));
	}
	if (!createQuery.exec(QString("create table '").append(Target_Table).append("' (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER);")))
	{
		QString error = createQuery.lastError().text();
		emit mediaDBError(error.prepend("createquery-create in CreateTable: "));
	}
	return 0;
}
void MediaDB::openDB()
{
	QSqlDatabase dataBase;
	if (!open)
	{
		emit mediaDBError("openDB: DB is not open. Opening....");
		open = true;
		dataBase = QSqlDatabase::database("QSQLITE");
		if (!dataBase.isValid())
		{
			emit mediaDBError("openDB: DB NOt in list, adding");
			dataBase = QSqlDatabase::addDatabase("QSQLITE");
		}
		//dataBase = &QSqlDatabase::database("QSQLITE",false);
		dataBase.setDatabaseName("test.db");
		dataBase.transaction();
		if (!dataBase.open())
		{
			QString error = dataBase.lastError().text();
			emit mediaDBError(error.prepend("openDB: "));
			open = false;
		}
	}
	else
	{
		emit mediaDBError("openDB: Checking to see if DB is open....");
		//Test to see if it's really open
		dataBase = QSqlDatabase::database("QSQLITE");
		if (!dataBase.isOpen())
		{
			QString error = dataBase.lastError().text();
			emit mediaDBError(error.prepend("openDB: Database Thinks its open, but it's not: "));
			//Oh noes!
			if (!dataBase.open())
			{
				QString error = dataBase.lastError().text();
				emit mediaDBError(error.prepend("openDB: Unable to open DB: "));
			}
		}
	}

}
void MediaDB::closeDB()
{
	open = false;
	QSqlDatabase::database("QSQLITE").close();
	QSqlDatabase::removeDatabase("QSQLITE");
}
int MediaDB::swapEntries(QString Target_Table,int Target_Entry_One, int Target_Entry_Two)
{
	Q_UNUSED(Target_Table);
	Q_UNUSED(Target_Entry_One);
	Q_UNUSED(Target_Entry_Two);
	return -1;
}
void MediaDB::updateEntries(QString Target_Table,QList<EntryClass> classlist)
{
	QSqlDatabase dataBase;
	qDebug() << "updateEntries";
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	dataBase.setDatabaseName("test.db");
	if (!dataBase.isOpen())
	{
		if (!dataBase.open())
		{
			qDebug() << "Unable to open database for updateEntries";
		}
	}
	QSqlQuery songInsert(dataBase);
	if (!songInsert.prepare(QString("insert or replace into '").append(Target_Table).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?,?);")))
	{
		QString error = songInsert.lastError().text(); 
		qDebug() << "Error on prepare:" << error << dataBase.lastError().text();
	}
	for (int i=0;i<classlist.count();i++)
	{
		songInsert.bindValue(0,classlist[i].tracknum);
		songInsert.bindValue(1,classlist[i].title);
		songInsert.bindValue(2,classlist[i].artist);
		songInsert.bindValue(3,classlist[i].album);
		songInsert.bindValue(4,classlist[i].filename);
		songInsert.bindValue(5,classlist[i].fullPath);
		songInsert.bindValue(6,classlist[i].tracknum);
		songInsert.bindValue(7,0);
		if (!songInsert.exec())
		{
		}

	}
	songInsert.clear();

}
void MediaDB::updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum)
{
	emit updateEntrySignal(Target_Table,Target_Entry,title,artist,album,filename,fullPath,tracknum);
}
void MediaDB::private_updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum)
{
	QSqlDatabase dataBase;
	openDB();
	dataBase = QSqlDatabase::database("QSQLITE");
	QSqlQuery tableFind(dataBase);
	QSqlQuery songInsert(dataBase);
	QSqlQuery songDelete(dataBase);
	if (!songInsert.prepare(QString("insert into '").append(Target_Table).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values(?,?,?,?,?,?,?,?);")))
	{
	}
	if (!songDelete.prepare(QString("delete from '").append(Target_Table).append("' where rowid == '").append(QString::number(Target_Entry)).append("';")))
	{
	}
	if (!songDelete.exec())
	{
	}
	
	songInsert.bindValue(0,tracknum);
	songInsert.bindValue(1,title);
	songInsert.bindValue(2,artist);
	songInsert.bindValue(3,album);
	songInsert.bindValue(4,filename);
	songInsert.bindValue(5,fullPath);
	songInsert.bindValue(6,tracknum);
	songInsert.bindValue(7,0);
	if (!songInsert.exec())
	{
	}
	tableFind.clear();
	songInsert.clear();
	songDelete.clear();
	return;
}
