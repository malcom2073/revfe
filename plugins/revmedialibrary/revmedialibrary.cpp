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

#include "revmedialibrary.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>






MediaLibrary::MediaLibrary()
{
	m_name = "RevMediaLibrary";
}

void MediaLibrary::init()
{

	qRegisterMetaType<QList<PlayListClass*> >("QList<PlayListClass*>");
	qRegisterMetaType<ID3Tag>("ID3Tag");
	qRegisterMetaType<QList<ID3Tag> >("QList<ID3Tag>");

	dirfound = false;

	databaseTimer = new QTimer(this);
	databaseEntryTickCount = 0;
	connect(databaseTimer,SIGNAL(timeout()),this,SLOT(databaseTimerTick()));
	//databaseTimer->start(100);
	dataBase  = 0;
	NowPlayingList = 0;
	mediaScanner = new MediaScannerClass();
	singleScanner = new MediaScannerClass();
	connect(mediaScanner,SIGNAL(threadTagReceived(QString,int,ID3Tag*)),this,SLOT(mediaInformationUpdate(QString,int,ID3Tag*)));
	connect(singleScanner,SIGNAL(threadTagReceived(QString,int,ID3Tag*)),this,SLOT(mediaInformationUpdate(QString,int,ID3Tag*)));
	Playlists = new QList<PlayListClass*>();
	mediaDBThread = new MediaDBThread();
	connect(mediaDBThread,SIGNAL(tagReceived(QString,int,ID3Tag*)),this,SLOT(mediaInformationUpdate(QString,int,ID3Tag*)));
	connect(mediaDBThread,SIGNAL(tagReceived(QString,const ID3Tag)),this,SLOT(mediaInformationUpdate(QString,const ID3Tag)));
	connect(mediaDBThread,SIGNAL(tagsReceived(QList<QString>,QList<ID3Tag>)),this,SLOT(multipleMediaInformationUpdate(QList<QString>,QList<ID3Tag>)));
	connect(mediaDBThread,SIGNAL(mediaList(QList<MediaFileClass*>)),this,SLOT(mediaList(QList<MediaFileClass*>)));
	connect(mediaDBThread,SIGNAL(scanProgressUpdate(QString)),this,SLOT(mediaScanProgressUpdate(QString)));
	connect(mediaDBThread,SIGNAL(initialScanComplete()),this,SLOT(mediaDBInitialScanComplete()));
	//mediaScanner->setDataBase(dataBase);
	connect(mediaDBThread,SIGNAL(threadFinished(QList<PlayListClass*>)),this,SLOT(mediaDBThreadEnded(QList<PlayListClass*>)));
	//dataBase = new MediaDB(this);
	//connect(dataBase,SIGNAL(mediaDBError(QString)),this,SLOT(mediaDBError(QString)));
	//m_playlistEditModel = new ComplexModel();
	//Playlist model needs to be


	QHash<int,QByteArray> playlisteditroles;
	playlisteditroles[0] = "artist";
	playlisteditroles[1] = "text";
	playlisteditroles[2] = "title";
	playlisteditroles[3] = "album";
	playlisteditroles[4] = "file";
	m_playlistEditModel = new StandardModel(playlisteditroles);

	QHash<int,QByteArray> playlistroles;
	playlistroles[0] = "text";
	playlistroles[1] = "artist";
	playlistroles[2] = "title";
	playlistroles[3] = "album";
	playlistroles[4] = "file";
	m_playlistModel = new StandardModel(playlistroles);

	QHash<int,QByteArray> playlistlistroles;
	playlistlistroles[0] = ".";
	playlistlistroles[1] = "text";
	m_playlistListModel = new StandardModel(playlistlistroles);
}
void MediaLibrary::show()
{
}
QString MediaLibrary::getName()
{
	return m_name;
}
void MediaLibrary::passPluginMessage(QString sender,QString message)
{
	Q_UNUSED(sender);
	Q_UNUSED(message);
}
void MediaLibrary::passPluginMessage(QString sender,IPCMessage message)
{
	QStringList tmp;
	tmp.append("IPC Received");
	tmp.append(sender);
	tmp.append(message.getClass());
	tmp.append(message.getMethod());
	tmp.append(message.getArgs());
	emit passCoreMessage(m_name,IPCMessage("core","debug","debug",tmp));
	if (message.getClass() == "playlist")
	{
		if (message.getMethod() == "getlist")
		{
			QStringList strList;
			if (NowPlayingList)
			{
				//strList.append(NowPlayingList->title());
			}
			for (int i=0;i<Playlists->size();i++)
			{
				strList.append(Playlists->at(i)->title());
			}
			emit passCoreMessage(m_name,IPCMessage(sender,"playlist","list",strList));
			return;
		}
		else if (message.getMethod() == "set") //Set the current playlist: set Playlist_Title
		{
			if (message.getArgs().count() > 0)
			{
				for (int i=0;i<m_playlists.size();i++)
				{
					if (m_playlists.at(i)->title() == message.getArgs()[0])
					{
						if (NowPlayingList)
						{
							delete NowPlayingList;
						}
						//m_playlistModel->clearContents();
						m_playlistModel->clear();
						//m_playlistModel->setData(QList<QList<QString> >());
						NowPlayingList = new PlayListClass();
						*NowPlayingList = *m_playlists.at(i);
						NowPlayingList->setTitle("Current Playlist");
						NowPlayingBaseString = message.getArgs()[0];
						if (message.getArgs().count() > 1)
						{
							emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:playlistloaded" << "Current Playlist" << "autostart"));
						}
						else
						{
							emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:playlistloaded" << "Current Playlist"));
						}
						//playlistListModel
						//QStringList evt;
						//evt << "event:propertylistmodelset";
						//evt << "currentPlaylistModel";
						//qDebug() << "NEw Playlist";
						/*
						playlisteditroles[0] = "artist";
						playlisteditroles[1] = "title";
						playlisteditroles[2] = "album";
						playlisteditroles[3] = "file";
						*/
						for (int i=0;i<NowPlayingList->size();i++)
						{
							//album artist title track
							//evt << NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title;
							//m_playlistModel->addRow(QList<QString>() << NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title);
							QStandardItem *item = new QStandardItem();
							//item->setData(NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title,0);
							item->setData(NowPlayingList->getFileNum(i)->getId3Tag()->artist,1);
							item->setData(NowPlayingList->getFileNum(i)->getId3Tag()->title,2);
							item->setData(NowPlayingList->getFileNum(i)->getId3Tag()->album,3);
							item->setData(NowPlayingList->getFileNum(i)->getId3Tag()->filename,4);
							//m_playlistModel->appendRow(QList<QString>() << );
							m_playlistModel->appendRow(item);
							//qDebug() << "Adding item: " << NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title;

						}
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
						//qDebug() << "Playlist size:" << m_playlistModel->rowCount();
						return;
					}
				}
			}
		}
		else if (message.getMethod() == "addsong") //Add a song to the end of the Current Playlist: addsong File_Name
		{
			//filename
			if (NowPlayingList)
			{
				//filename pos album artist title track
				emit passCoreMessage(m_name,IPCMessage("RevMediaEngine","playlist","insertsong",QStringList() << message.getArgs()[0] << QString::number(-1) << "album" << "artist" << "title" << "track"));
				NowPlayingList->insertFile(message.getArgs()[0],-1);
				singleScanner->singleScan(message.getArgs()[0]);
				
			}
		}
		else if (message.getMethod() == "addsongs") //Add multiple songs to the end of the current playlist: addsongs File1_Name File2_Name ... FileN_Name
		{
			if (NowPlayingList)
			{
				for (int i=0;i<message.getArgs().count();i++)
				{
					NowPlayingList->insertFile(message.getArgs()[i],-1);
					emit passCoreMessageBlocking(m_name,IPCMessage("RevMediaEngine","playlist","insertsong",QStringList() << message.getArgs()[i] << QString::number(-1) << "album" << "artist" << "title" << "track"));
				}
				singleScanner->multiScan(message.getArgs());
				//filename album artist title track

			}
		}
		else if (message.getMethod() == "removesong") //Remove a song from the current playlist: removesong File_Number
		{
			if (NowPlayingList)
			{
				//NowPlayingList->r
				NowPlayingList->removeFile(message.getArgs()[0].toInt());
				emit passCoreMessage(m_name,IPCMessage("RevMediaEngine","playlist","removesong",QStringList() << message.getArgs()[0]));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:currentplaylist_removesong" << QString::number(message.getArgs()[0].toInt())));
				//emit passCoreMessage(m_name,IPCMessage(sender,"currentplaylist","remove",QStringList() << QString::number(message.getArgs()[0].toInt())));
			}
		}
		else if (message.getMethod() == "insertsong") //Insert a song in the current playlist: insertsong Unknown File_Name Position_To_Insert
		{
			if (NowPlayingList)
			{
				NowPlayingList->insertFile(message.getArgs()[1],message.getArgs()[2].toInt());
			}
		}
		else if (message.getMethod() == "adddir")
		{

		}
		else if (message.getMethod() == "addplaylist")
		{

		}
		else if (message.getMethod() == "savecurrent")
		{
			if (NowPlayingList)
			{
				
			}
		}
		else if (message.getMethod() == "findbyartist")
		{
			qDebug() << "Find By Artist" << message.toString();
			//MediaLibrary:playlist findbyartist !TargetName! !targetclass! !targetmethod! !filename1! !filename2! .... !filenameN!
			QStringList list;
			for (int i=0;i<Playlists->count();i++)
			{
				for (int j=0;j<Playlists->at(i)->size();j++)
				{
					ID3Tag *tag = Playlists->at(i)->id3(j);
					if (tag)
					{
						if (tag->artist.toLower() == message.getArgs()[3].toLower())
						{
							qDebug() << "Found!:" << tag->filename;
							list << tag->filename;
						}
					}
					else
					{

					}

				}
			}
			emit passCoreMessage(m_name,IPCMessage(message.getArgs()[0],message.getArgs()[1],message.getArgs()[2],list));
		}
		else if (message.getMethod() == "movesongdown")
		{
			if (NowPlayingList)
			{
				if (message.getArgs()[0].toInt() < NowPlayingList->size() - 1)
				{
					NowPlayingList->moveSong(message.getArgs()[0].toInt(),message.getArgs()[0].toInt()+1);
					emit passCoreMessage(m_name,IPCMessage("MediaEngine","playlist","movesong",QStringList() << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()+1)));
					//emit passCoreMessage(m_name,IPCMessage(sender,"currentplaylist","swap",QStringList() << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()+1)));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:currentplaylist_songswap" << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()+1)));
					//emit passCoreMessage(m_name,IPCMessage(sender,"currentplaylist","select",QStringList() << QString::number(message.getArgs()[0].toInt()+1)));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:currentplaylist_select" << QString::number(message.getArgs()[0].toInt()+1)));
				}
			}
		}
		else if (message.getMethod() == "movesongup")
		{
			if (NowPlayingList)
			{
				if (message.getArgs()[0].toInt() > 0)
				{
					NowPlayingList->moveSong(message.getArgs()[0].toInt(),message.getArgs()[0].toInt()-1);
					emit passCoreMessage(m_name,IPCMessage("MediaEngine","playlist","movesong",QStringList() << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()-1)));
					//emit passCoreMessage(m_name,IPCMessage(sender,"currentplaylist","swap",QStringList() << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()-1)));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:currentplaylist_songswap" << QString::number(message.getArgs()[0].toInt()) << QString::number(message.getArgs()[0].toInt()-1)));
					//emit passCoreMessage(m_name,IPCMessage(sender,"currentplaylist","select",QStringList() << QString::number(message.getArgs()[0].toInt()-1)));
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:currentplaylist_select" << QString::number(message.getArgs()[0].toInt()-1)));
				}
			}
		}
		else if (message.getMethod() == "savecurrentplaylist")
		{
			if (NowPlayingList)
			{
				bool overrideBool = false;
				QString tmpNameStr = "";
				if (message.getArgs().count() > 1)
				{
					if (message.getArgs()[1] == "no")
					{
						return;
					}
					overrideBool = true;
				}
				if (message.getArgs().count() > 0)
				{
					tmpNameStr = message.getArgs()[0];
				}
				else
				{
					tmpNameStr = NowPlayingList->title();
				}
				bool found = false;
				for (int i=0;i<Playlists->count();i++)
				{
					if (Playlists->at(i)->title() == tmpNameStr)
					{
						found = true;
					}
				}
				if ((found) && (!overrideBool))
				{
					passCoreMessage(m_name,IPCMessage("WindowManager","window","showconfirmation",QStringList() << QString("Do you want to overwrite the playlist").append(tmpNameStr) << "playlist" << "savecurrentplaylist" << tmpNameStr));
					return;
				}
				//dataBase->openDB();
				dataBase->createTable(tmpNameStr);
				QList<EntryClass> entries;
				for (int i=0;i<NowPlayingList->size();i++)
				{
					ID3Tag *tmpTag = NowPlayingList->getFileNum(i)->getId3Tag();
					EntryClass tmp;
					if (tmpTag)
					{
						tmp.title = tmpTag->title;
						tmp.artist = tmpTag->artist;
						tmp.album = tmpTag->album;
						tmp.filename = tmpTag->filename.mid(tmpTag->filename.lastIndexOf("/")+1);
						tmp.fullPath = tmpTag->filename;
						tmp.tracknum = i;
						//dataBase->addEntry(tmpNameStr,i,,tmpTag->artist,tmpTag->album,,tmpTag->filename,i);
					}
					else
					{
						passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "Error while saving playlist" << "Playlist entry does not contain ID3 tag" << NowPlayingList->getFileNum(i)->fullPath()));
						QString fullPath = NowPlayingList->getFileNum(i)->fullPath();
						tmp.title = "title";
						tmp.artist = "artist";
						tmp.album = "album";
						tmp.filename = fullPath.mid(fullPath.lastIndexOf("/")+1);
						tmp.fullPath = fullPath;
						tmp.tracknum = i;
						//dataBase->addEntry(tmpNameStr,i,"title","artist","album",fullPath.mid(fullPath.lastIndexOf("/")+1),fullPath,i);
					}
					entries.append(tmp);
				}
				for (int i=0;i<Playlists->count();i++)
				{
					if (tmpNameStr == Playlists->at(i)->title())
					{
						Playlists->removeAt(i);
						i = Playlists->count();
					}
				}
				
				//Playlists->append(NowPlayingList);
				dataBase->addEntries(tmpNameStr,entries);
				PlayListClass *tmp = new PlayListClass();
				*tmp = *NowPlayingList;
				tmp->setTitle(tmpNameStr);
				Playlists->append(tmp);
				
				passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList("event:newmedialist")));
				//dataBase->closeDB();
			}
		}
		else if (message.getMethod() == "getsongfilenames")
		{
			qDebug() << "getsongfilenames CALLED";
			if (message.getArgs().count() > 0)
			{
				if (message.getArgs()[0] == NowPlayingList->title())
				{
						QStringList strMediaList;
						strMediaList.append(NowPlayingList->title());
						strMediaList.append(NowPlayingBaseString);
						for (int j=0;j<NowPlayingList->size();j++)
						{
							//strList.append(Playlists.at(i)->getFileNum(j)->fileName());
							strMediaList.append(NowPlayingList->getFileNum(j)->fullPath());
							//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
							//album artist title track
							strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->album);
							strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->artist);
							strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->title);
							strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->track);
							//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
						}
						//emit passCoreMessage("PlaylistManager",IPCMessage("core","event","throw",strMediaList));
						emit passCoreMessage(m_name,IPCMessage(sender,"playlist","songlist",strMediaList));
						//emit passCoreMessage("PlaylistManager",IPCMessage("MediaEngine","playlist","songlist",strMediaList));
						return;
				}
				else
				{
					for (int i=0;i<Playlists->size();i++)
					{
						if (Playlists->at(i)->title() == message.getArgs()[0])
						{
							//QStringList strList;
							QStringList strMediaList;
							
							strMediaList.append(Playlists->at(i)->title());
							strMediaList.append(Playlists->at(i)->title());
							for (int j=0;j<Playlists->at(i)->size();j++)
							{
								//strList.append(Playlists.at(i)->getFileNum(j)->fileName());
								strMediaList.append(Playlists->at(i)->getFileNum(j)->fullPath());
								//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
								//album artist title track
								strMediaList.append(Playlists->at(i)->getFileNum(j)->getId3Tag()->album);
								strMediaList.append(Playlists->at(i)->getFileNum(j)->getId3Tag()->artist);
								strMediaList.append(Playlists->at(i)->getFileNum(j)->getId3Tag()->title);
								strMediaList.append(Playlists->at(i)->getFileNum(j)->getId3Tag()->track);
								//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
							}
							//emit passCoreMessage("PlaylistManager",IPCMessage("core","event","throw",strMediaList));
							emit passCoreMessage(m_name,IPCMessage(sender,"playlist","songlist",strMediaList));
							//emit passCoreMessage("PlaylistManager",IPCMessage("MediaEngine","playlist","songlist",strMediaList));
							return;
						}
						
					}
				}
				emit passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList("Error making playlists")));
			}
			else
			{
				if (NowPlayingList)
				{
					QStringList strMediaList;
					strMediaList.append(NowPlayingList->title());
					strMediaList.append(NowPlayingBaseString);
					for (int j=0;j<NowPlayingList->size();j++)
					{
						//strList.append(Playlists.at(i)->getFileNum(j)->fileName());
						strMediaList.append(NowPlayingList->getFileNum(j)->fullPath());
						//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
						//album artist title track
						strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->album);
						strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->artist);
						strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->title);
						strMediaList.append(NowPlayingList->getFileNum(j)->getId3Tag()->track);
						//strMediaList.append(Playlists->at(i)->getFileNum(j)->displayName());
					}
					//emit passCoreMessage("PlaylistManager",IPCMessage("core","event","throw",strMediaList));
					emit passCoreMessage(m_name,IPCMessage(sender,"playlist","songlist",strMediaList));
					//emit passCoreMessage("PlaylistManager",IPCMessage("MediaEngine","playlist","songlist",strMediaList));
					return;
				}
			}
		}
		else if (message.getMethod() == "setdir")
		{
			Playlists->clear();
			QString myString = message.getArgs()[0];
			QDir mainMusicDir(myString);
			foreach (QFileInfo dirInfo, mainMusicDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
			{
				//Playlist *tmpPlayList = new Playlist();
				PlayListClass *tmpPlayList = new PlayListClass();
				QDir tmpAlbumDir(dirInfo.absoluteFilePath());
				foreach (QFileInfo fileInfo, tmpAlbumDir.entryInfoList(QDir::Files))
				{
					tmpPlayList->addFile(fileInfo.absoluteFilePath());
				}
				int locTwo = dirInfo.absoluteFilePath().lastIndexOf('/');	
				tmpPlayList->setTitle(dirInfo.absoluteFilePath().mid(locTwo+1));
				Playlists->append(tmpPlayList);
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList("event:newmediadir")));
			return;
		}
	}
	else if (message.getClass() == "playlisteditor")
	{
		if (message.getMethod() == "add")
		{
			m_currentEditPlaylist.append(message.getArgs()[0]);
			QStringList evt;
			evt << "event:propertylistmodelset";
			evt << "MEDIA_EDIT_CURRENT_LIST";
			for (int i=0;i<m_currentEditPlaylist.size();i++)
			{
				bool found = false;
				for (int j=0;j<m_mediaList.size();j++)
				{
					if (m_mediaList[j]->fullPath() == m_currentEditPlaylist[i])
					{
						if (m_mediaList[j]->getId3Tag()->artist != "artist")
						{
							found = true;
							evt << m_mediaList[j]->getId3Tag()->artist + " - " + m_mediaList[j]->getId3Tag()->title;
						}
					}

				}
				if (!found)
				{
					evt << m_currentEditPlaylist[i].mid(m_currentEditPlaylist[i].lastIndexOf("/")+1);
				}
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
		}
		else if (message.getMethod() == "remove")
		{

		}
	}
	else if (message.getClass() == "setting")
	{
		if (message.getMethod() == "change")
		{
			if (message.getArgs()[0] == "mediadir")
			{
				QString key = message.getArgs()[0];
				QString value = message.getArgs()[1];
				QString type = message.getArgs()[2];
				QString id = message.getArgs()[3];

				if (m_musicDirMap.contains(id.toInt()))
				{
					if (m_musicDirMap[id.toInt()] == value)
					{
						//Setting is the same, no reason to change it.
					}
					else
					{
						//Need to eventually handle changing media directories
						m_musicDirMap[id.toInt()] = value;
					}
				}
				else
				{
					m_musicDirMap[id.toInt()] = value;
					mediaDBThread->addDirectory(value);
				}
			}

		}
	}
	else if (message.getClass() == "event")
	{	
		if (message.getMethod() == "throw")
		{
			if (message.getArgs()[0] == "event:close")
			{

			}
		}
		else if (message.getMethod() == "initstarted")
		{
			for (int i=0;i<message.getArgs().count();i++)
			{
				QString key = message.getArgs()[i];
				QString value = message.getArgs()[i+1];
				QString type = message.getArgs()[i+2];
				QString id = message.getArgs()[i+3];
				i++;
				i++;
				i++;
				qDebug() << "MediaLibrary settings:" << key << value << type << id;
				if (key.split("=")[1] == "musicdir")
				{
					//Add it to our musicdir list
					dirfound = true;
					m_baseDir = value.split("=")[1];
					m_musicDirMap[id.split("=")[1].toInt()] = m_baseDir;
					mediaDBThread->addDirectory(value.split("=")[1]);
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "MEDIA_DIRS" << m_baseDir));
				}
				else if (key.split("=")[1] == "videodir")
				{
					//Add it to our videodir list
					//Soon to be supported!
				}
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "2" << "MEDIA_DIRS" << "" << "Media_Scanner_Progress" << ""));
			passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			passCoreModel("playlistListModel",m_playlistListModel);
			passCoreModel("currentPlaylistModel",m_playlistModel);
			passCoreModel("playlistEditModel",m_playlistEditModel);

			//mediaDBThread->start();
			QSettings set("IFS","RevFE");
			set.beginGroup("MediaEngine"); 
			int song = set.value("currentsong").toInt();
			long pos = set.value("currentpos").toLongLong();
			double vol = set.value("volume",50).toDouble();
			qDebug() << "Volume read as:" << vol;
			emit passCoreMessage(m_name,IPCMessage("MediaEngine","media","setvolume",QStringList() << QString::number(vol)));
			QString state = set.value("currentstate").toString();
			set.beginGroup("Playlist");
			int count = set.value("count",0).toInt();
			//m_playlistModel->clearContents();
			m_playlistModel->clear();
			//m_playlistModel->setData(QList<QList<QString> >());
			//m_playlistEditModel->clearContents();
			m_playlistEditModel->clear();
			//m_playlistEditModel->setData(QList<QList<QString> >());
			if (count > 0)
			{
				NowPlayingList = new PlayListClass();
				NowPlayingList->setTitle("Current Playlist");
				for (int i=0;i<count;i++)
				{
					QString file = set.value(QString("song:file:").append(QString::number(i))).toString();
					QString album = set.value(QString("song:album:").append(QString::number(i))).toString();
					QString artist = set.value(QString("song:artist:").append(QString::number(i))).toString();
					QString title = set.value(QString("song:title:").append(QString::number(i))).toString();
					QString track = set.value(QString("song:track:").append(QString::number(i))).toString();
					NowPlayingList->addFile(file);
					ID3Tag *tmpTag = new ID3Tag();
					tmpTag->filename = file;
					tmpTag->album = album;
					tmpTag->artist = artist;
					tmpTag->title = title;
					tmpTag->track = track;
					NowPlayingList->setId3(i,tmpTag);
					/*playlisteditroles[0] = "artist";
					playlisteditroles[1] = "title";
					playlisteditroles[2] = "album";
					playlisteditroles[3] = "file";*/

					//m_playlistModel->addRow(QList<QString>() << QString(artist + " - " + title));
					//m_playlistEditModel->addRow(QList<QString>() << QString(artist + " - " + title));
					QStandardItem *listitem = new QStandardItem();
					//listitem->setData(QString(artist + " - " + title),0);
					listitem->setData(artist,1);
					listitem->setData(title,2);
					listitem->setData(album,3);
					listitem->setData(file,4);
					m_playlistModel->appendRow(listitem);
					QStandardItem *edititem = new QStandardItem();
					//edititem->setData(QString(artist + " - " + title),0);
					edititem->setData(artist,1);
					edititem->setData(title,2);
					edititem->setData(album,3);
					edititem->setData(file,4);
					m_playlistEditModel->appendRow(edititem);

				}
				if (state == "Playing")
				{
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:playlistloaded" << NowPlayingList->title() << "autostart" << QString::number(pos) << QString::number(song)));
				}
				else
				{
					emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:playlistloaded" << NowPlayingList->title()));
				}
				qDebug() << "Loaded current playlist";
			}
			set.endGroup();
			set.endGroup();

			if (dirfound)
			{
				qDebug() << "About to start mediaDB thread";
				mediaDBThread->start();
			}
			else
			{
				passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "No music dir found in settings.xml. Not starting media thread"));
				qDebug() << "No Music Dir Found";
			}

			passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
		}
	}
}
void MediaLibrary::generateFolderModel(QString dirstr)
{
	Q_UNUSED(dirstr);
	/*
	QStringList evt;
	evt << "event:propertylistmodelset";
	evt << "MEDIA_FILE_MODEL";
	QDir dir(dirstr);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString entry,dir.entryList())
	{
		evt << entry;//dir.absoluteFilePath(entry);
	}
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
	*/
}

void MediaLibrary::mediaInformationUpdate(QString playlistname,int tracknum,ID3Tag *tag)
{
	bool found = false;
	if ((playlistname == "") || (NowPlayingBaseString == playlistname))
	{
		for (int i=0;i<NowPlayingList->size();i++)
		{
			//if (NowPlayingList->getFileNum(i)->fullPath() == playlistname)
			if (NowPlayingList->getFileNum(i)->fullPath() == tag->filename)
			{
				
				//QStringList tmpList;
				//tmpList << tag->filename;
				//tmpList << tag->album;
				//tmpList << tag->artist;
				//tmpList << tag->title;
				//tmpList << tag->track;
				//MediaEngine:playlist edittag filename album artist title track 
				//emit passCoreMessage(m_name,IPCMessage("MediaEngine","playlist","edittag",tmpList));
				found = true;
				NowPlayingList->getFileNum(i)->setId3Tag(tag);
				QStringList eventList;
				eventList << "event:currentplaylistupdate";
				eventList << NowPlayingList->title();
				eventList << QString::number(i);
				eventList << tag->album;
				eventList << tag->artist;
				eventList << tag->title;
				eventList << QString::number(i);
				//album artist title track
				//event playlistname tracknum filename album artist title track
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",eventList));

				QStringList evt;
				evt << "event:propertylistmodelset";
				evt << "currentPlaylistModel";
				for (int i=0;i<NowPlayingList->size();i++)
				{
					//album artist title track
					 evt << NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title;
				}
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
				//This return statement was in place to prevent long loops after you located the song to change as a performance enhancer.
				//It was removed due to the chance of intentionally adding a song more than once to the same playlist
				//so the duplicate song wouldn't just show up as artist - title and look like an untagged mp3 file.
				//return;
			}
		}
		if (!found)
		{
			//int stop = 1; // Debug breakpoint!
		}
	}
	else
	{
		EntryClass tmpEntry;
		tmpEntry.Target_Table = playlistname;
		tmpEntry.title = tag->title;
		tmpEntry.artist = tag->artist;
		tmpEntry.album = tag->album;
		tmpEntry.filename = tag->filename.mid(tag->filename.lastIndexOf("/")+1);
		tmpEntry.fullPath = tag->filename;
		tmpEntry.tracknum = tracknum;
		tmpEntry.Target_Entry = tracknum;
		databaseEntryList.append(tmpEntry);
		//dataBase->updateEntry(playlistname,tracknum,tag->title,tag->artist,tag->album,tag->filename.mid(tag->filename.lastIndexOf("/")+1),tag->filename,tracknum);
		QStringList eventList;
		eventList << "event:medialibraryupdate";
		eventList << playlistname;
		eventList << QString::number(tracknum);
		//eventList << 
		eventList << tag->album;
		eventList << tag->artist;
		eventList << tag->title;
		eventList << QString::number(tracknum);
		//album artist title track
		//event playlistname tracknum filename album artist title track
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",eventList));
	}
}

void MediaLibrary::mediaDBThreadEnded(QList<PlayListClass*> playlists)
{
	qDebug() << __FUNCTION__;
	Playlists = new QList<PlayListClass*>();
	*Playlists = playlists;
	//dataBase = mediaDBThread->getDatabase();
	//dataBase = new MediaDB(this);
	//dataBase->openDB();
	//dataBase->moveToThread(QThread::currentThread());
	//mediaDBThread->setPlayListPointer(*Playlists); //Sets current pointer to the playlist that the MediaDB found
	//delete mediaDBThread;
//	mediaScanner->setDataBase(dataBase);
	//mediaScanner->setPlaylists(Playlists);
	//mediaScanner->startScannerThread();
	//passPluginMessage("core",IPCMessage(m_name,"playlist","getlist",QStringList()));
	qDebug() << "PLAYLIST MODEL SET!!!";
	QStringList evt;
	evt << "event:propertylistset";
	evt << "playlistListModel";
	for (int i=0;i<Playlists->size();i++)
	{
		evt << Playlists->at(i)->title();
	}

	//QList<QList<QString> > data;
	//data.append(QList<QString>());
	for (int i=0;i<playlists.size();i++)
	{
		//m_playlistListModel->addRow(QList<QString>() << playlists[i]->title());
		QStandardItem *item = new QStandardItem();
		item->setData(playlists[i]->title(),1);
		m_playlistListModel->appendRow(item);
		//data.append(QList<QString>());
		//data[data.size()-1].append(playlists[i]->title());
		//data[0].append(list[i]);
	}

	//QStringList roles;
	//roles << "text";
	//m_playlistListModel->setRoles(roles);
	//m_playlistListModel->setData(data);

	/*if (m_complexModelList.contains(title))
	{
		context->setContextProperty(title,QVariant());
		m_complexModelList[title]->deleteLater();
	}*/
	//m_complexModelList[title] = mod;
	//context->setContextProperty(title,mod);

	passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
	passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList("event:newmedialist")));
	passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediascanfinished"));

	qDebug() << "Media scanning finished. Total playlists found: " << Playlists->count();
	qDebug() << "Starting to scan id3 tags";
}
void MediaLibrary::createPlaylists(QString from)
{
	qDebug() << "Creating playlists:" << from;
	QStringList newplnames;
	QString m_baseDir = "";
	for (int i=0;i<m_playlists.size();i++)
	{
		delete m_playlists[i];
	}
	m_playlists.clear();
	for (int j=0;j<m_mediaList.size();j++)
	{
		QList<int> keys = m_musicDirMap.keys();
		//qDebug() << m_mediaList[j]->getId3Tag()->filename;
		for (int i=0;i<keys.size();i++)
		{
			//qDebug() << m_musicDirMap[keys[i]];
			if (m_mediaList[j]->getId3Tag()->filename.startsWith(m_musicDirMap[keys[i]]))
			{
				m_baseDir = m_musicDirMap[keys[i]];
				//qDebug() << m_baseDir << m_mediaList[j]->getId3Tag()->filename;
				break;
			}
		}
		bool found = false;
		QString name = "";
		if (from == "folder")
		{
			name = m_mediaList[j]->getId3Tag()->filename.mid(m_baseDir.size());
			if (name.count("/") < 2)
			{
				//It's a base level file.
				name = "/";
			}
			else
			{
				name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");
			}
		}
		else if (from == "album")
		{
			name = m_mediaList[j]->getId3Tag()->album;
		}
		for (int k=0;k<m_playlists.size();k++)
		{
			bool foundsong = false;
			QString title = m_playlists[k]->title();
			if (m_playlists[k]->title() == name)
			{
				found = true;
				for (int l=0;l<m_playlists[k]->size();l++)
				{
					if (m_playlists[k]->getFileNum(l)->fullPath() == m_mediaList[j]->fullPath())
					{
						foundsong = true;
					}
				}
				if (!foundsong)
				{
					m_playlists[k]->addFile(m_mediaList[j]);
				}
			}
			else
			{
				bool weird = true;
			}
		}
		if (!found)
		{
			//newpl = false;
			//qDebug() << "New playlist";
			//qDebug() << i << filename.size();
			PlayListClass *pl = new PlayListClass();
			if (false)
			{
				pl->setTitle(m_mediaList[j]->getId3Tag()->album);
			}
			else
			{
				QString name = m_mediaList[j]->getId3Tag()->filename.mid(m_baseDir.size());
				name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");
				pl->setTitle(name);
				newplnames.append(name);
				//m_playlistListModel->addRow(QList<QString>() << name);
				QStandardItem *item = new QStandardItem();
				item->setData(name,1);
				m_playlistListModel->appendRow(item);
			}
			//qDebug() << "Playlist" << name << "File" << j << "of" << m_mediaList.size();
			pl->addFile(m_mediaList[j]);
			m_playlists.append(pl);
			//newplnames.append(newtag->album);

		}
	}
	if (newplnames.size() > 0)
	{
		QStringList evt;
		evt << "event:propertylistmodeladd";
		evt << "playlistListModel";
		for (int i=0;i<newplnames.size();i++)
		{
			//evt << newplnames[i];

		}
		//passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
		qDebug() << "Playlists:" << evt;
	}

	//QList<QList<QString> > data;
	//data.append(QList<QString>());
	/*for (int i=0;i<playlists.size();i++)
	{
		data.append(QList<QString>());
		data[data.size()-1].append(playlists[i]->title());
		//data[0].append(list[i]);
	}
	QStringList roles;
	roles << "text";
	m_playlistListModel->setRoles(roles);
	m_playlistListModel->setData(data);
	*/
	qDebug() << "Finished creating playlists";
}
//This gets called after the first scan of the media list.
void MediaLibrary::mediaDBInitialScanComplete()
{
	qDebug() << "Initial scan of MediaDB complete. Creating playlists";
	createPlaylists("folder");
}

void MediaLibrary::mediaInformationUpdate(QString filename,const ID3Tag tag)
{
	bool newpl = false;
	QString newplname = "";
	for (int j=0;j<m_mediaList.size();j++)
	{
		if (m_mediaList[j]->fullPath() == filename)
		{
			ID3Tag *newtag = new ID3Tag();
			*newtag = tag;
			m_mediaList[j]->setId3Tag(newtag);
			bool found = false;
			bool foundsong = false;
			if (NowPlayingList)
			{
				for (int k=0;k<NowPlayingList->size();k++)
				{
					//if (NowPlayingBaseString == m_playlists[k]->title())
					if (NowPlayingList->getFileNum(k)->fullPath() == filename)
					{
						//It's the currently playing playlist.
						//We need to send an update to the GUI.
						QStringList evt;
						evt << "event:propertylistmodelset";
						evt << "currentPlaylistModel";
						for (int i=0;i<NowPlayingList->size();i++)
						{
							//album artist title track
							 evt << NowPlayingList->getFileNum(i)->getId3Tag()->artist + "-" + NowPlayingList->getFileNum(i)->getId3Tag()->title;
						}
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
					}
				}
			}
			continue;
			for (int k=0;k<m_playlists.size();k++)
			{
				QString name = "";
				name = newtag->filename.mid(m_baseDir.size());
				//qDebug() << name;
				if (name.count("/") < 2)
				{
					//It's a base level file.
					name = "/";
				}
				else
				{
					name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");
				}
				//qDebug() << name;
				//name = newtag->album;
				if (m_playlists[k]->title() == name)
				{
					//qDebug() << "Album:" << newtag->album;
					found = true;
					newpl = true;
					for (int l=0;l<m_playlists[k]->size();l++)
					{
						if (m_playlists[k]->getFileNum(l)->fullPath() == filename)
						{
							foundsong = true;
						}
					}
					if (!foundsong)
					{
						m_playlists[k]->addFile(m_mediaList[j]);
					}
					k = m_playlists.size();
				}
			}
			if (!found)
			{
				PlayListClass *pl = new PlayListClass();
				QString name = newtag->filename.mid(m_baseDir.size());
				name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");

				//pl->setTitle(newtag->album);
				pl->setTitle(name);
				pl->addFile(m_mediaList[j]);
				m_playlists.append(pl);
				//newplname = newtag->album;
				newplname = name;
			}
			j = m_mediaList.size();
		}
	}
	return;
	if (!newpl)
	{
		QStringList evt;
		evt << "event:propertylistmodeladd";
		evt << "playlistListModel";
		evt << newplname;
		passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
	}
}
void MediaLibrary::multipleMediaInformationUpdate(QList<QString> filename,const QList<ID3Tag> tag)
{
	bool newpl = false;
	QList<QString> newplnames;
	for (int i=0;i<filename.size();i++)
	{
		qDebug() << i << filename.size();
		for (int j=0;j<m_mediaList.size();j++)
		{
			if (m_mediaList[j]->fullPath() == filename[i])
			{

				ID3Tag *newtag = m_mediaList[j]->getId3Tag();
				*newtag = tag[i];
				//newtag
				m_mediaList[j]->setId3Tag(newtag);
				bool foundsong = false;
				bool found = false;
				continue;
				for (int k=0;k<m_playlists.size();k++)
				{
					QString name = "";
					name = newtag->filename.mid(m_baseDir.size());
					if (name.count("/") < 2)
					{
						//It's a base level file.
						name = "/";
					}
					else
					{
						name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");
					}
					//name = newtag->album;
					if (m_playlists[k]->title() == name)
					{
						found = true;
						for (int l=0;l<m_playlists[k]->size();l++)
						{
							if (m_playlists[k]->getFileNum(l)->fullPath() == filename[i])
							{
								foundsong = true;
							}
						}
						if (!foundsong)
						{
							m_playlists[k]->addFile(m_mediaList[j]);
						}
					}
				}
				if (!found)
				{
					newpl = false;
					//qDebug() << "New playlist";
					qDebug() << i << filename.size();
					PlayListClass *pl = new PlayListClass();
					if (false)
					{
						pl->setTitle(newtag->album);
					}
					else
					{
						QString name = newtag->filename.mid(m_baseDir.size());
						name = name.mid(1,name.lastIndexOf("/")-1).replace("/"," - ");
						pl->setTitle(name);
						newplnames.append(name);
					}
					pl->addFile(m_mediaList[j]);
					m_playlists.append(pl);
					//newplnames.append(newtag->album);

				}
				j = m_mediaList.size();
			}
		}
	}
	return;
	if (newplnames.size() > 0)
	{
		QStringList evt;
		evt << "event:propertylistmodeladd";
		evt << "playlistListModel";
		for (int i=0;i<newplnames.size();i++)
		{
			evt << newplnames[i];
		}
		passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
	}
}

//This function gets called every time a folder is added to the media list,
//and it is scanned. media contains an list of files read from the database
//If the tags have already been scanned, it will contain tag data. If not, then
//tags will be populaed via the mediaInformationUpdate slot
//It also gets called when the media is first loaded from the .db file.
//This is just a generic list of all media. It should be used for the base for playlists
//and anything else requiring a list of all media currently scanned in the system.

//This needs to be moved to a seperate thread, it's taking way too much cpu time.
//Better yet, this needs to be optimized. I need a better way to find duplicate items.
void MediaLibrary::mediaList(QList<MediaFileClass*> media)
{
	//This is new media to be added to the list.
	QStringList evt;
	evt << "event:propertylistmodelset";
	evt << "playlistListModel";
	passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
	for (int i=0;i<media.size();i++)
	{
		//MediaFileClass *file = new MediaFileClass();
		//QString folder = filenames[i].mid(0,filenames[i].lastIndexOf("/"));
		//QString name = folder.mid(folder.lastIndexOf("/")+1);
		//qDebug() << name << filenames[i];
		//file->setFullPath(filenames[i]);
		bool found = false;
		/*for (int j=0;j<m_mediaList.size();j++)
		{
			if (media[i]->fullPath() == m_mediaList[j]->fullPath())
			{
				found = true;
				//qDebug() << "Duplicate:" << media[i]->fullPath();
			}
		}*/
		if (!found)
		{
			m_mediaList.append(media[i]);
		}
	}
	qDebug() << "Done adding files to m_mediaList." << media.size() << "files added succesfully" << "Total:" << m_mediaList.size();
	//createPlaylists("folder");
}
void MediaLibrary::mediaScanProgressUpdate(QString update)
{
	QStringList evt;
	evt << "event:propertyset";
	evt << "1";
	evt << "Media_Scanner_Progress";
	evt << update;
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));
}

void MediaLibrary::databaseTimerTick()
{
	if (databaseEntryList.count() > 0)
	{
		if ((databaseEntryList.count() > 100) || databaseEntryTickCount > 10)
		{
			qDebug() << "Updating Database";
			databaseEntryTickCount = 0;
			dataBase->updateEntries(databaseEntryList[0].Target_Table,databaseEntryList);
			databaseEntryList.clear();
		}
		databaseEntryTickCount++;
	}
}
void MediaLibrary::mediaDBError(QString error)
{
	passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "Error in MediaDB: " << error));
}
Q_EXPORT_PLUGIN2(RevMediaLibraryPlugin, MediaLibrary)

