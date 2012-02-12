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

#include "mediadbthread.h"
#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#define SECONDS_IN_DAY 86400 //24 hours
#define SECONDS_IN_WEEK 604800
MediaDBThread::MediaDBThread()
{
	qRegisterMetaType<QList<MediaFileClass*> >("QList<MediaFileClass*>");
}
MediaDBThread::~MediaDBThread()
{
	delete mediaDatabase;
}
void MediaDBThread::initialScan()
{

}
void MediaDBThread::addDirectory(QString dir)
{
	qDebug() << "Adding directory for media scan:" << dir;
	m_dirListMutex.lock();
	m_dirList.append(dir);
	m_dirListMutex.unlock();
}

void MediaDBThread::scan(QList<QString> files)
{
	m_scanListMutex.lock();
	m_scanList.append(files);
	m_scanListMutex.unlock();
}

void MediaDBThread::run()
{
	mediaDatabase = new MediaDB();
	connect(mediaDatabase,SIGNAL(tagReceived(QString,const ID3Tag)),this,SIGNAL(tagReceived(QString,const ID3Tag)));
	connect(mediaDatabase,SIGNAL(tagsReceived(QList<QString>,QList<ID3Tag>)),this,SIGNAL(tagsReceived(QList<QString>,QList<ID3Tag>)));
	connect(mediaDatabase,SIGNAL(mediaList(QList<MediaFileClass*>)),this,SIGNAL(mediaList(QList<MediaFileClass*>)));
	/*if (!mediaDatabase->init(musicDir,&Playlists))
	{
		qDebug() << "Error reading database files";
	}
	emit threadFinished(Playlists);
	*/
	scanProgressUpdate("Initialize Database...");
	QList<QString> filenames = mediaDatabase->init();
	//m_internalScanList.append(filenames);



	QSqlQuery result;
	QSqlQuery replace;
	QSqlDatabase dataBase;
	bool first = true;
	while (true)
	{
		m_scanListMutex.lock();
		m_internalScanList.append(m_scanList);
		m_scanList.clear();
		m_scanListMutex.unlock();
		m_dirListMutex.lock();
		m_internalDirList.append(m_dirList);
		m_dirList.clear();
		m_dirListMutex.unlock();


		if (m_internalDirList.size() > 0)
		{
			for (int i=0;i<m_internalDirList.size();i++)
			{
				qDebug() << "About to scan:" << m_internalDirList[i];
				emit scanProgressUpdate("Scanning directory: " + m_internalDirList[i]);
				QList<QString> filenames = mediaDatabase->add(m_internalDirList[i]);
				m_internalScanList.append(filenames);
			}
			m_internalDirList.clear();
		}

		if (m_internalScanList.size() > 0)
		{
			qDebug() << "Total files still requiring tags:" << m_internalScanList.size();

			dataBase = QSqlDatabase::addDatabase("QSQLITE");
			dataBase.setDatabaseName("test.db");
			if (!dataBase.open())
			{
				qDebug() << "Error opening db";
			}
			if (!dataBase.transaction())
			{
				qDebug() << "Error opening db for transaction";
			}
			result = QSqlQuery(dataBase);
			replace = QSqlQuery(dataBase);
			if (!replace.prepare("insert or replace into 'Media_List' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) VALUES (?,?,?,?,?,?,?,?);"))
			{
				qDebug() << "Error preparing replace query" << dataBase.lastError().text();
				qDebug() << replace.lastError().databaseText();
				continue;
			}
		}

		for (int i=0;i<m_internalScanList.size();i++)
		{
			emit scanProgressUpdate(QString("Scanning id3 tags: ").append(QString::number(i) + "/" + QString::number(m_internalScanList.size())));
			if (!(i % 50))
			{
				qDebug() << "Scanning file:" << i << "of" << m_internalScanList.size();
			}
			if (!result.exec("select * from 'Media_List' where fullPath is '" + m_internalScanList[i].replace("'","''") + "';"))
			{
				qDebug() << "Unable to select from media list:" << m_internalScanList[i];
				continue;
			}
			m_internalScanList[i].replace("''","'");
			if (!result.next())
			{
				qDebug() << "No next record!" << result.lastQuery();
				continue;
			}
			//qDebug() << result.record();
			int rowid = result.value(result.record().indexOf("tikey")).toString().toInt();
			ID3Tag tmpTag;
			TagLib::FileRef testfile(m_internalScanList[i].toStdString().c_str());
			if (!testfile.isNull())
			{
				tmpTag.lastupdate = QDateTime::currentDateTime().toTime_t();
				if (TagLib::MPEG::File *file = dynamic_cast<TagLib::MPEG::File *>( testfile.file()))
				{
					if (file->ID3v2Tag(false))
					{
						//Check to see if there are images in the tag
						#ifdef Q_OS_WIN32 //Currently this only works in the windows version of taglib??
						if (!file->ID3v2Tag()->frameListMap()["APIC"].isEmpty())
						{
							TagLib::ID3v2::FrameList frameList = file->ID3v2Tag()->frameListMap()["APIC"];
							TagLib::ID3v2::FrameList::Iterator iter;
							for( iter = frameList.begin(); iter != frameList.end(); ++iter )
							{
								TagLib::ID3v2::AttachedPictureFrame::Type t = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->type();
								if (t == TagLib::ID3v2::AttachedPictureFrame::Type::FrontCover)
								{
									TagLib::ByteVector pic = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->picture();
									QImage tmpPicture;
									tmpPicture.loadFromData((uchar*)pic.data(),pic.size());
									tmpTag.frontcover = tmpPicture;
								}
								else if (t == TagLib::ID3v2::AttachedPictureFrame::Type::BackCover)
								{
									TagLib::ByteVector pic = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->picture();
									QImage tmpPicture;
									tmpPicture.loadFromData((uchar*)pic.data(),pic.size());
									tmpTag.backcover = tmpPicture;
								}

							}
						}
						#endif //Q_OS_WIN32
						TagLib::Tag *tag = testfile.tag();
						tmpTag.artist = tag->artist().toCString();
						tmpTag.title = tag->title().toCString();
						tmpTag.album = tag->album().toCString();
						tmpTag.track = tag->track();
						tmpTag.filename = m_internalScanList[i];
						//replace
						/*replace.bindValue(0,j);
						replace.bindValue(1,tmpTag->title);
						replace.bindValue(2,tmpTag->artist);
						replace.bindValue(3,tmpTag->album);
						replace.bindValue(4,tmpTag->filename);
						replace.bindValue(5,tmpTag->filename);
						replace.bindValue(6,j);
						replace.bindValue(7,(int)tmpTag->lastupdate);
						if (!replace.exec())
						{
							qDebug() << "Error executing replace!";
						}*/
						// (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER) VALUES (?,?,?,?,?,?,?);"))
						//emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
					}
					else
					{
						//There's no tag! We still need an empty tag in there.
						//emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
					}

				}
				else
				{
					TagLib::Tag *tag = testfile.tag();
					tmpTag.artist = tag->artist().toCString();
					tmpTag.title = tag->title().toCString();
					tmpTag.album = tag->album().toCString();
					tmpTag.track = tag->track();
					tmpTag.filename = m_internalScanList[i];
					//emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
				}
			}
			else
			{
				tmpTag.artist = "artist";
				tmpTag.title = "title";
				tmpTag.filename = m_internalScanList[i];
				tmpTag.lastupdate = QDateTime::currentDateTime().toTime_t();
				//emit tagReceived(Playlists.at(i)->title(),j,tmpTag);

				//qDebug() << "MediaDBThread: Invalid File in multipass" << Playlists.at(i)->getFileNum(j)->fullPath();
			}
			//Do something with tmptag here.
			//(rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) VALUES (?,?,?,?,?,?,?,?);"))
			replace.bindValue(0,rowid);
			replace.bindValue(1,tmpTag.title);
			replace.bindValue(2,tmpTag.artist);
			replace.bindValue(3,tmpTag.album);
			replace.bindValue(4,tmpTag.filename.mid(tmpTag.filename.lastIndexOf("/")+1));
			replace.bindValue(5,tmpTag.filename);
			replace.bindValue(6,tmpTag.track);
			replace.bindValue(7,(long long)tmpTag.lastupdate);
			//qDebug() << "Media entry:" << tmpTag.title << tmpTag.artist << tmpTag.album;
			if (!replace.exec())
			{
				qDebug() << "Error executing replace in table" << replace.lastError().text() << replace.lastQuery();
			}
			emit tagReceived(m_internalScanList[i],tmpTag);
			dataBase.commit();
		}
		if (first)
		{
			//qDebug() << "Emitting";
			first = false;
			emit initialScanComplete();
		}
		m_internalScanList.clear();
		//qDebug() << "Idle loop";
		//If we are really idle, no reason to loop through but once a second checking for new files to scan.
		emit scanProgressUpdate("Done");
		msleep(1000);
	}


	return;

	//This is the thread that should be scanning through the playlists looking for id3 tags. Let's move that here.
	//MediaDB should have released all sql access, so we can freely open and access it here while scanning.
	//We want to essentially scan through each playlist, and scan through each file in each playlist.
	//Generate an id3 tag on the heap for each file, and pass it to MediaLibrary, since at this point
	//MediaLibrary has control of the PlayLists list. A signal with a Playlist and Filenumber
	//parameter would be best, since at this moment (Well any moment after threadFinished is fired)
	//we have an exact copy of the playlist list that MediaLibrary has.

	//The issue is, how to scan through id3 tags, if at any point the playlists list can change.
	//Being as media scanning should in theory be performed before any other actions, I'm going to
	//operate on that assumption. At some point this should be re-written in a more thread safe way
	//since EVERYTHING is in a different thread now.

	//There is already a slot for this in MediaLibrary, mediaInformationUpdate(QString,int,ID3Tag*).
	//Let's use it for now

	//QSqlDatabase dataBase;
	//debugFile = new QFile("debug.log");
	//debugFile->open(QIODevice::ReadWrite);

	//int rc = sqlite3_open("test.db",&dataBase);
	dataBase = QSqlDatabase::addDatabase("QSQLITE");
	QString debugLoc = "~/.revfe";
	debugLoc = debugLoc.replace("~",getenv("HOME"));
	/*if (QFile::exists(debugLoc))
	{
		qDebug() << "Using ~/.revfe for database file";
		//QFile::remove(debugLoc  + "/test.db");
		dataBase.setDatabaseName(debugLoc + "/test.db");
	}
	else
	{*/
		qDebug() << "Using application location for database file";
		//QFile::remove("test.db");
		dataBase.setDatabaseName("test.db");
	//}
	//dataBase.transaction();
	//open = true;
	if (!dataBase.open())
	{
		//open = false;
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
	//QSqlQuery result(dataBase);
	//QSqlQuery replace(dataBase);
	for (int i=0;i<Playlists.size();i++)
	{
		if (!result.prepare("select * from 'Table_Index' where title is '" + Playlists.at(i)->title().replace("'","''") + "';"))
		{
			qDebug() << "Invalid prepare on Table_Index select:" << result.lastError().databaseText();
			//return;
		}
		if (!result.exec())
		{
			qDebug() << "No valid Playlist found!" << result.lastError().databaseText();
			continue;
		}
		if (!replace.prepare("insert or replace into '" + Playlists.at(i)->title().replace("'","''") + "' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) VALUES (?,?,?,?,?,?,?,?);"))
		{
			qDebug() << "Error preparing replace query" << dataBase.lastError().text();
			qDebug() << replace.lastError().databaseText();
			continue;
		}

		for (int j=0;j<Playlists.at(i)->size();j++)
		{
			/*if (!result.prepare("select * from '" + Playlists.at(i)->title() + "' where fullPath is '" + Playlists.at(i)->getFileNum(j)->fullPath() + "';"))
			{
				qDebug() << "Error preparing for individual file select";
				continue;
			}
			result.next();
			int lastupdate = result.value(result.record().indexOf("lastupdate"));
			*///TagLib::FileRef testfile(fullPath.toStdString().c_str());
			ID3Tag *tmpTag = Playlists.at(i)->getFileNum(j)->getId3Tag();
			if (!tmpTag)
			{
				tmpTag = new ID3Tag();
				tmpTag->valid = false;
			}
			else
			{
				//If the tag has ever been updated, no reason to re-update it. Chances are it hasnt changed
				if (tmpTag->lastupdate != 0)
				{
					continue;
				}
				//If it's been less than a week since the last update, don't do another
				if ((QDateTime::currentDateTime().toTime_t() - tmpTag->lastupdate) < SECONDS_IN_WEEK)
				{
					//qDebug() << "Not rescanning";
					continue;
				}
				else
				{
					//qDebug() << "Old tag:" << tmpTag->lastupdate;
				}
			}
			if (j == 0)
			{
				qDebug() << "Scanning playlist:" << i << "of" << Playlists.size();
			}
			TagLib::FileRef testfile(Playlists.at(i)->getFileNum(j)->fullPath().toStdString().c_str());
			if (!testfile.isNull())
			{
				tmpTag->lastupdate = QDateTime::currentDateTime().toTime_t();
				if (TagLib::MPEG::File *file = dynamic_cast<TagLib::MPEG::File *>( testfile.file()))
				{
					if (file->ID3v2Tag(false))
					{
						//Check to see if there are images in the tag
						#ifdef Q_OS_WIN32 //Currently this only works in the windows version of taglib??
						if (!file->ID3v2Tag()->frameListMap()["APIC"].isEmpty())
						{
							TagLib::ID3v2::FrameList frameList = file->ID3v2Tag()->frameListMap()["APIC"];
							TagLib::ID3v2::FrameList::Iterator iter;
							for( iter = frameList.begin(); iter != frameList.end(); ++iter )
							{
								TagLib::ID3v2::AttachedPictureFrame::Type t = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->type();
								if (t == TagLib::ID3v2::AttachedPictureFrame::Type::FrontCover)
								{
									TagLib::ByteVector pic = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->picture();
									QImage tmpPicture;
									tmpPicture.loadFromData((uchar*)pic.data(),pic.size());
									tmpTag->frontcover = tmpPicture;
								}
								else if (t == TagLib::ID3v2::AttachedPictureFrame::Type::BackCover)
								{
									TagLib::ByteVector pic = (((TagLib::ID3v2::AttachedPictureFrame*)*iter))->picture();
									QImage tmpPicture;
									tmpPicture.loadFromData((uchar*)pic.data(),pic.size());
									tmpTag->backcover = tmpPicture;
								}

							}
						}
						#endif //Q_OS_WIN32
						TagLib::Tag *tag = testfile.tag();
						tmpTag->artist = tag->artist().toCString();
						tmpTag->title = tag->title().toCString();
						tmpTag->filename = Playlists.at(i)->getFileNum(j)->fullPath();
						//replace
						replace.bindValue(0,j);
						replace.bindValue(1,tmpTag->title);
						replace.bindValue(2,tmpTag->artist);
						replace.bindValue(3,tmpTag->album);
						replace.bindValue(4,tmpTag->filename);
						replace.bindValue(5,tmpTag->filename);
						replace.bindValue(6,j);
						replace.bindValue(7,(int)tmpTag->lastupdate);
						if (!replace.exec())
						{
							qDebug() << "Error executing replace!";
						}
						// (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER) VALUES (?,?,?,?,?,?,?);"))
						emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
					}
					else
					{
						//There's no tag! We still need an empty tag in there.
						emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
					}

				}
				else
				{
					TagLib::Tag *tag = testfile.tag();
					tmpTag->artist = tag->artist().toCString();
					tmpTag->title = tag->title().toCString();
					tmpTag->filename = Playlists.at(i)->getFileNum(j)->fullPath();
					replace.bindValue(0,j);
					replace.bindValue(1,tmpTag->title);
					replace.bindValue(2,tmpTag->artist);
					replace.bindValue(3,tmpTag->album);
					replace.bindValue(4,tmpTag->filename);
					replace.bindValue(5,tmpTag->filename);
					replace.bindValue(6,j);
					replace.bindValue(7,(int)tmpTag->lastupdate);
					if (!replace.exec())
					{
						qDebug() << "Error executing replace!";
					}
					emit tagReceived(Playlists.at(i)->title(),j,tmpTag);
				}
			}
			else
			{
				tmpTag->artist = "artist";
				tmpTag->title = "title";
				tmpTag->filename = Playlists.at(i)->getFileNum(j)->fullPath();
				tmpTag->lastupdate = QDateTime::currentDateTime().toTime_t();
				replace.bindValue(0,j);
				replace.bindValue(1,tmpTag->title);
				replace.bindValue(2,tmpTag->artist);
				replace.bindValue(3,tmpTag->album);
				replace.bindValue(4,tmpTag->filename);
				replace.bindValue(5,tmpTag->filename);
				replace.bindValue(6,j);
				replace.bindValue(7,(int)tmpTag->lastupdate);
				if (!replace.exec())
				{
					qDebug() << "Error executing replace!";
				}
				emit tagReceived(Playlists.at(i)->title(),j,tmpTag);

				qDebug() << "MediaDBThread: Invalid File in multipass" << Playlists.at(i)->getFileNum(j)->fullPath();
			}
		}
		dataBase.commit();
	}
	dataBase.close();
	qDebug() << "Scan of id3 tags complete";

}
void MediaDBThread::setPlayListPointer(QList<PlayListClass *> &pointer)
{
	Q_UNUSED(pointer)
	//mediaDatabase->setPlayListPointer(pointer); //Sets current pointer to the playlist that the MediaDB found
}
