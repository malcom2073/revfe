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

#include "revmediaengine.h"
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QSettings>
#include "videowidget.h"



MediaEngine::MediaEngine()
{
	m_name = "RevMediaEngine";
}

void MediaEngine::init()
{
	qmlRegisterType<VideoWidget>("VideoPlayer", 1, 0, "VideoPlayer");
	qDebug() << "MediaEngine init started";
	autoStartPos = -1;
	autoStartNextList = false;
	autoStartNextPos = false;
	die = false;
	muted = false;
	mediaPlayer = new MediaPlayer(this);
	connect(mediaPlayer,SIGNAL(timeChanged(ulong,ulong)),this,SLOT(audioTimeChanged(ulong,ulong)));
	connect(mediaPlayer,SIGNAL(finished()),this,SLOT(audioAlmostFinished()));
	connect(mediaPlayer,SIGNAL(mediaChanged()),this,SLOT(audioChanged()));
	mediaTimer = new QTimer();
	connect(mediaTimer,SIGNAL(timeout()),this,SLOT(mediaTimerTimeout()));
	mediaTimer->start(250);	

}
QString MediaEngine::getName()
{
	return m_name;
}
void MediaEngine::passPluginMessage(QString sender,QString message)
{

}
void MediaEngine::audioTimeChanged(unsigned long current,unsigned long total)
{
	QStringList propEvt;
	propEvt << "event:propertyset";
	propEvt << "3";
	propEvt << "totalTimeText";
	propEvt << formatTime(total);
	propEvt << "elapsedTimeText";
	propEvt << formatTime(current);
	propEvt << "remainingTimeText";
	propEvt << formatTime(total - current);
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",propEvt));
}

void MediaEngine::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "media")
	{
		if (message.getMethod() == "play")
		{
			if (message.getArgs().count() > 0)
			{
				//mediaObject->stop();
				mediaPlayer->stop();
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
				m_currentIndex = message.getArgs()[0].toInt();
				//mediaObject->clearQueue();
				mediaPlayer->setMedia(sourceList[m_currentIndex]);
				//mediaObject->setCurrentSource(sourceList[m_currentIndex]);
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex] << QString::number(m_currentIndex)));
				QString file = tagList[m_currentIndex].filename;
				QString sourceFile = sourceList[m_currentIndex];
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number((audioObject->volume() * 100.0))));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));

				//mediaObject->play();
				mediaPlayer->play();
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "currentSongIndex" << QString::number(m_currentIndex)));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
			}
			else
			{
				//mediaObject->stop();
				//libvlc_media_player_stop(vlcMediaPlayer);
				mediaPlayer->stop();
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
				//mediaObject->play();
				mediaPlayer->play();
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
			}
			QString folderName = sourceList[m_currentIndex].mid(0,sourceList[m_currentIndex].lastIndexOf("/")) + "/Folder.jpg";
			if (QFile::exists(folderName))
			{
				//int stop = 1;
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:media_albumart" << folderName));
			}
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
			//mediaTimer->start(250);
		}
		else if (message.getMethod() == "pause")
		{
			/*if (mediaObject->state() == Phonon::PausedState)
			{
				//mediaObject->play();
				libvlc_media_player_play(vlcMediaPlayer);
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
			}
			else if (mediaObject->state() == Phonon::PlayingState)
			{
				//mediaObject->pause();
				libvlc_media_player_pause(vlcMediaPlayer);
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:paused"));
			}*/
		}
		else if (message.getMethod() == "stop")
		{
			//mediaObject->stop();
			//libvlc_media_player_stop(vlcMediaPlayer);
			mediaPlayer->stop();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
		}
		else if (message.getMethod() == "prev")
		{
			if (m_currentIndex > 0)
			{
				m_currentIndex--;
			}
			else
			{
				m_currentIndex = sourceList.count() - 1;
			}
			//mediaObject->stop();
			//libvlc_media_player_stop(vlcMediaPlayer);
			mediaPlayer->stop();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
			//mediaObject->clearQueue();
			//mediaObject->setCurrentSource(sourceList[m_currentIndex]);
			mediaPlayer->setMedia(sourceList[m_currentIndex]);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number((audioObject->volume() * 100.0))));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
			//mediaObject->play();
			mediaPlayer->play();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
		}
		else if (message.getMethod() == "next")
		{
			if (m_currentIndex < sourceList.count() - 1)
			{
				m_currentIndex++;
			}
			else
			{

				m_currentIndex = 0;
			}
			mediaPlayer->stop();
			//mediaObject->stop();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
			//mediaObject->clearQueue();
			//mediaObject->setCurrentSource(sourceList[m_currentIndex]);
			mediaPlayer->setMedia(sourceList[m_currentIndex]);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertySet" << "currentPlaylistIndex" << QString::number(m_currentIndex)));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex].fileName() << QString::number(m_currentIndex)));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number((audioObject->volume() * 100.0))));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
			//mediaObject->play();
			mediaPlayer->play();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
		}
		else if (message.getMethod() == "mute")
		{
			if (muted)
			{
				//audioObject->setMuted(false);
				muted = false;
			}
			else
			{
				muted = true;
				//audioObject->setMuted(true);
			}
		}
		else if (message.getMethod() == "setpos")
		{
			/*long max = mediaObject->totalTime();
			double pos = message.getArgs()[0].toDouble();
			mediaObject->seek((pos / 100.0) * max);*/

		}
		else if (message.getMethod() == "setvolume")
		{
			//QString arg1 = message.getArgs()[0];
			double vol = message.getArgs()[0].toDouble();
			qDebug() << "setvolume called as:" << vol;
			//audioObject->setVolume((vol / 100.0));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number(vol)));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
		}
		else if (message.getMethod() == "volup")
		{
			/*qDebug() << "volup" << audioObject->volume();
			double vol = 10;
			if (message.getArgs().count() > 0)
			{
				vol = message.getArgs()[0].toDouble();
			}
			if ((audioObject->volume() + (vol / 100.0))>= 1.0)
			{
				audioObject->setVolume(1.0);
			}
			else
			{
				audioObject->setVolume(audioObject->volume() + (vol / 100.0));
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number(audioObject->volume() * 100.0)));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
			*/
		}
		else if (message.getMethod() == "voldown")
		{
			/*
			double vol = 10;
			if (message.getArgs().count() > 0)
			{
				vol = message.getArgs()[0].toDouble();
			}
			if ((audioObject->volume() - (vol / 100.0)) <= 0)
			{
				audioObject->setVolume(0);
			}
			else
			{
				audioObject->setVolume(audioObject->volume() - (vol / 100.0));
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number(audioObject->volume() * 100.0)));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
			*/
		}
	}
	else if (message.getClass() == "playlist")
	{
		if (message.getMethod() == "songlist")
		{
			qDebug() << "MediaEngine playlist songlist";
			mediaPlayer->stop();
			//mediaObject->stop();
			sourceList.clear();
			sourceTitleList.clear();
			//mediaObject->clearQueue();
			tagList.clear();
			QStringList tmpList;
			tmpList.append("event:newplaylist");
			//event:newplaylist playlistname trackcount | filename album artist title track | filename album artist title track
			playlistName = "Current Playlist";
			tmpList.append("Current Playlist");
			tmpList.append("0");
			int itemCount = 0;
			for (int i=2;i<message.getArgs().count();i++)
			{
				itemCount++;
				//sourceList.append(Phonon::MediaSource(QString("file:///").append(message.getArgs().at(i))));
				sourceList.append(message.getArgs().at(i));
				ID3Tag tmpTag;
				tmpTag.filename = message.getArgs().at(i);
				tmpList.append(message.getArgs().at(i));
				i++;
				
				tmpTag.album = message.getArgs().at(i);
				tmpList.append(message.getArgs().at(i));
				i++;
				tmpTag.artist = message.getArgs().at(i);
				tmpList.append(message.getArgs().at(i));
				i++;
				tmpTag.title = message.getArgs().at(i);
				tmpList.append(message.getArgs().at(i));
				i++;
				tmpTag.track = message.getArgs().at(i);
				tmpList.append(message.getArgs().at(i));
				tagList.append(tmpTag);
				//album artist title track
				//sourceTitleList.append(message.getArgs().at(i));
			}
			tmpList[2] = QString::number(itemCount);
			m_currentIndex = 0;
			if (sourceList.count() > 0)
			{
				//mediaObject->setCurrentSource(sourceList[0]);
				/*emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex].fileName() << QString::number(m_currentIndex)));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number((audioObject->volume() * 100.0))));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
				*/
			}
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",tmpList));
			if (autoStartNextList)
			{
				autoStartNextList = false;
				//mediaObject->stop();
				mediaPlayer->stop();
				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:stopped"));
				m_currentIndex = autoStartNum;
				//mediaObject->clearQueue();
				autoStartNextPos = true;
				//mediaObject->setCurrentSource(sourceList[m_currentIndex]);

				mediaPlayer->setMedia(sourceList[m_currentIndex]);

				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex] << QString::number(m_currentIndex)));
				QString file = tagList[m_currentIndex].filename;
				QString sourceFile = sourceList[m_currentIndex];
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex].fileName() << QString::number(m_currentIndex)));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_volumechanged" << QString::number((audioObject->volume() * 100.0))));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTVOLUME%" << QString::number((audioObject->volume() * 100.0))));
				//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "sys_volume" << QString::number((audioObject->volume() * 100.0))));
				//mediaObject->play();
				mediaPlayer->seek(autoStartPos);
				mediaPlayer->play();

				emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediastate" << "media:playing"));
				//mediaObject->seek(autoStartPos);
				qDebug() << "AutoStart:" << autoStartPos;
				//passPluginMessage(sender,IPCMessage(m_name,"media","play",QStringList() << "0"));
			}
		}
		else if (message.getMethod() == "new")
		{
			emit passCoreMessage(m_name,IPCMessage("RevMediaLibrary","playlist","getsongfilenames",QStringList(message.getArgs()[0])));
		}
		else if (message.getMethod() == "getlist")
		{
			emit passCoreMessage(m_name,IPCMessage("core","debug","debug",QStringList() << "GetList" << sender));
			//event:newplaylist playlistname trackcount | filename album artist title track | filename album artist title track
			if (sourceList.count() == 0)
			{
				return;
			}
			QStringList tmpList;
			tmpList << "event:newplaylist";
			tmpList << "Current Playlist";
			tmpList << QString::number(sourceList.count());
			for (int i=0;i<sourceList.count();i++)
			{
				tmpList << sourceList[i];
				tmpList << tagList[i].album;
				tmpList << tagList[i].artist;
				tmpList << tagList[i].title;
				tmpList << tagList[i].track;
			}
			emit passCoreMessage(m_name,IPCMessage(sender,"event","throw",tmpList));
			audioChanged();
		}
		else if (message.getMethod() == "insertsong")
		{
			if (message.getArgs()[1] == "-1")
			{
				//filename pos album artist title track
				//sourceList.insert(message.getArgs()[1].toInt(),message.getArgs()[0]);
				//sourceList.append(Phonon::MediaSource(QString("file:///").append(message.getArgs()[0])));
				sourceList.append(QString("file:///").append(message.getArgs()[0]));
				ID3Tag tmpTag;
				tmpTag.album = message.getArgs()[2];
				tmpTag.artist = message.getArgs()[3];
				tmpTag.title = message.getArgs()[4];
				tmpTag.track = message.getArgs()[5];
				tagList.append(tmpTag);
			}
			else
			{
				//sourceList.insert(message.getArgs()[1].toInt(),Phonon::MediaSource(message.getArgs()[0]));
				sourceList.insert(message.getArgs()[1].toInt(),message.getArgs()[0]);
			}
			QStringList eventString;
			eventString <<  "event:mediaengine_currentplaylistchanged";
			eventString << "insert";
			eventString << message.getArgs();
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",eventString));
		}
		else if (message.getMethod() == "removesong")
		{
			int newIndex = message.getArgs()[0].toInt();
			sourceList.removeAt(newIndex);
			tagList.removeAt(newIndex);
		}
		else if (message.getMethod() == "movesong")
		{
			int oldIndex = message.getArgs()[0].toInt();
			int newIndex = message.getArgs()[1].toInt();
			//Phonon::MediaSource tmpSource = sourceList[oldIndex];
			ID3Tag tmpTag = tagList[oldIndex];
			sourceList.swap(oldIndex,newIndex);
			//sourceList.removeAt(oldIndex);
			//sourceList.insert(newIndex,tmpSource);
			tagList.swap(oldIndex,newIndex);
			//tagList.removeAt(oldIndex);
			//tagList.insert(newIndex,tmpTag);
			//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediaengine_currentplaylistchanged"));
		}
		else if (message.getMethod() == "edittag")
		{
			//MediaEngine:playlist edittag filename album artist title track 
			for (int i=0;i<sourceList.count();i++)
			{
				if (sourceList[i] == message.getArgs()[0])
				{					
					tagList[i].album = message.getArgs()[1];
					tagList[i].artist = message.getArgs()[2];
					tagList[i].title = message.getArgs()[3];
					tagList[i].track = message.getArgs()[4];
				}
			}
		}
	}
	else if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","subscribe",QStringList("event:currentplaylistupdate")));
			emit passCoreMessage(m_name,IPCMessage("core","event","subscribe",QStringList("event:playlistloaded")));
			QStringList evt;
			evt << "event:propertyset";
			evt << "5";
			evt << "currentSongText";
			evt << "";
			evt << "elapsedTimeText";
			evt << "";
			evt << "totalTimeText";
			evt << "";
			evt << "currentSongIndex";
			evt << "0";
			evt << "sys_volume";
			evt << "0.0";
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",evt));

			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));


		}
		else if (message.getMethod() == "initclose")
		{
			if (sourceList.count() > 0)
			{
				QSettings set("IFS","RevFE");
				set.beginGroup(m_name);
				set.setValue("currentsong",m_currentIndex);

				/*if (mediaObject->state() == Phonon::PlayingState)
				{
					set.setValue("currentstate","Playing");
					set.setValue("currentpos",mediaObject->currentTime());
				}
				else
				{
					set.setValue("currentstate","Stopped");
					set.setValue("currentpos",0);
				}
				set.setValue("volume",(audioObject->volume() * 100));
				qDebug() << "Volume saved as" << (audioObject->volume() * 100);
				*/
				set.beginGroup("Playlist");
				set.setValue("count",sourceList.count());
				for (int i=0;i<sourceList.count();i++)
				{
					set.setValue(QString("song:file:").append(QString::number(i)),sourceList[i]);
					set.setValue(QString("song:album:").append(QString::number(i)),tagList[i].album);
					set.setValue(QString("song:artist:").append(QString::number(i)),tagList[i].artist);
					set.setValue(QString("song:title:").append(QString::number(i)),tagList[i].title);
					set.setValue(QString("song:track:").append(QString::number(i)),tagList[i].track);
				}

				set.endGroup();
				set.endGroup();

			}
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}

		else if (message.getMethod() == "throw")
		{
			if (message.getArgs()[0] == "event:playlistloaded")
			{
				qDebug() << "MediaEngine saw event playlistloaded from MediaLibrary";
				playlistName = message.getArgs()[1];
				if (message.getArgs().count() > 2)
				{
					if (message.getArgs()[2] == "autostart")
					{
						autoStartNextList = true;
						autoStartPos = message.getArgs()[3].toLong();
						autoStartNum = message.getArgs()[4].toInt();
						qDebug() << "AutoStart1:" << autoStartPos;
					}
					
				}
				emit passCoreMessage(m_name,IPCMessage("RevMediaLibrary","playlist","getsongfilenames",QStringList(message.getArgs()[1])));
			}
			else if (message.getArgs()[0] == "event:currentplaylistupdate")
			{
				QString cmp1 = message.getArgs()[1];
				if (message.getArgs()[1] == playlistName)
				{
					//event:currentplaylistupdate playlistname tracknum album artist title track
					//int num = message.getArgs()[5].toInt();
					int num = message.getArgs()[2].toInt();
					//currentPlaylistBox->setItem(num,0,new QTableWidgetItem(QString(message.getArgs().at(3)).append(" - ").append(message.getArgs().at(4))));
					tagList[num].album = message.getArgs()[3];
					tagList[num].artist = message.getArgs()[4];
					tagList[num].title = message.getArgs()[5];
					tagList[num].track = message.getArgs()[6];
					//event:audiochanged filename album artist title track
					if (num == m_currentIndex)
					{
						QStringList tmpEvt;
						tmpEvt << "event:audiochanged";
						//tmpEvt << newSource.fileName().mid(newSource.fileName().lastIndexOf("/")+1);
						tmpEvt << sourceList[num].mid(sourceList[num].lastIndexOf("/")+1);
						//tmpEvt << sourceTitleList[m_currentIndex];
						//album artist title track
						tmpEvt << tagList.at(m_currentIndex).album;
						tmpEvt << tagList.at(m_currentIndex).artist;
						tmpEvt << tagList.at(m_currentIndex).title;
						tmpEvt << tagList.at(m_currentIndex).track;
						tmpEvt << playlistName;
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",tmpEvt));
						QString text = tagList.at(m_currentIndex).artist + "-" + tagList.at(m_currentIndex).title;
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "currentSongText" << text));
					}
				}
			}
		}
	}
}
void MediaEngine::play(int songNumber)
{
}
void MediaEngine::setPlayList()
{
}
void MediaEngine::stop()
{
}
void MediaEngine::pause()
{
}
void MediaEngine::audioAlmostFinished()
{
	if (m_currentIndex < sourceList.count() - 1)
	{
		m_currentIndex++;
	}
	else
	{
		m_currentIndex = 0;
	}
	mediaPlayer->setMedia(sourceList[m_currentIndex]);
	mediaPlayer->play();
	//mediaObject->clearQueue();
	//mediaObject->enqueue(sourceList[m_currentIndex]);
}
void MediaEngine::audioChanged()
{
	qDebug() << "Audio Changed!";
	//textvalues="%ID3-CURRENTARTIST%,%ID3-CURRENTTITLE%
	//textvalues="%ID3-CURRENTPLAYLIST%"/>
	QStringList tmpEvt;
	tmpEvt << "event:audiochanged";
	tmpEvt << sourceList[m_currentIndex].mid(sourceList[m_currentIndex].lastIndexOf("/")+1);
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "currentSongIndex" << QString::number(m_currentIndex)));
	QString text = tagList.at(m_currentIndex).artist + "-" + tagList.at(m_currentIndex).title;
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "currentSongText" << text));
	//tmpEvt << newSource.fileName().mid(newSource.fileName().lastIndexOf("/")+1);
	//tmpEvt << sourceTitleList[m_currentIndex];
	//album artist title track playlistname
	tmpEvt << tagList.at(m_currentIndex).album;
	tmpEvt << tagList.at(m_currentIndex).artist;
	tmpEvt << tagList.at(m_currentIndex).title;
	tmpEvt << tagList.at(m_currentIndex).track;
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%ID3-CURRENTARTIST%" << tagList.at(m_currentIndex).artist));
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%ID3-CURRENTTITLE%" << tagList.at(m_currentIndex).title));
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%ID3-CURRENTPLAYLIST%" << playlistName));
	emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:mediachange" << sourceList[m_currentIndex] << QString::number(m_currentIndex)));
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%ID3-CURRENTARTIST%" << tagList.at(m_currentIndex).artist));
	if (playlistName == "")
	{
		tmpEvt << "Null Playlist";
	}
	else
	{
		tmpEvt << playlistName;
	}
	//emit passCoreMessage(m_name,IPCMessage("core","event","throw",tmpEvt));
}
void MediaEngine::mediaTimerTimeout()
{
	if (die)
	{
		return;
	}
	/*if ((mediaObject->state() != Phonon::LoadingState) && (mediaObject->state() != Phonon::ErrorState))
	{
		QStringList propEvt;
		propEvt << "event:propertyset";
		propEvt << "3";
		propEvt << "totalTimeText";
		propEvt << formatTime(mediaObject->totalTime());
		propEvt << "elapsedTimeText";
		propEvt << formatTime(mediaObject->currentTime());
		propEvt << "remainingTimeText";
		propEvt << formatTime(mediaObject->totalTime() - mediaObject->currentTime());
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",propEvt));
		/*QStringList tmpEvt;
		tmpEvt << "event:audiopositionupdate";
		//tmpEvt << QString::number(mediaObject->totalTime());
		//tmpEvt << QString::number(mediaObject->currentTime());
		//tmpEvt << QString::number(mediaObject->totalTime() - mediaObject->currentTime()); 
		tmpEvt << formatTime(mediaObject->totalTime());
		tmpEvt << formatTime(mediaObject->currentTime());
		tmpEvt << formatTime(mediaObject->totalTime() - mediaObject->currentTime());
		tmpEvt << QString::number((double)mediaObject->currentTime() / (double)mediaObject->totalTime());
  		tmpEvt << QString::number(audioObject->volume());
		QString percent = QString::number(((double)mediaObject->currentTime() / (double)mediaObject->totalTime()) * 100.0);
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",tmpEvt));
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTTIME%" << formatTime(mediaObject->currentTime())));
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-TOTALTIME%" << formatTime(mediaObject->totalTime())));
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTTIMEPERCENT%" << percent));

		//event:audiopositionupdate totaltime currenttime timeleft
	}*/
	/*
	else
	{
		/*
		QStringList tmpEvt;
		tmpEvt << "event:audiopositionupdate";
		tmpEvt << "00:00";
		tmpEvt << "00:00";
		tmpEvt << "00:00";
		tmpEvt << "0";
		tmpEvt << "100";
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-CURRENTTIME%" << "00:00"));
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:textchange" << "%TT-TOTALTIME%" << "00:00"));
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",tmpEvt));
		//QMessageBox::information(0,"Error",mediaObject->errorString().append(" ").append(mediaObject->currentSource().fileName()));
		QStringList propEvt;
		propEvt << "event:propertyset";
		propEvt << "3";
		propEvt << "totalTimeText";
		propEvt << "00:00";
		propEvt << "elapsedTimeText";
		propEvt << "00:00";
		propEvt << "remainingTimeText";
		propEvt << "00:00";
		emit passCoreMessage(m_name,IPCMessage("core","event","throw",propEvt));
	}*/
}
QString MediaEngine::formatTime(long time)
{
	time /= 1000;
	long hours = time / 3600;
	long minutes = (time - (hours * 3600)) / 60;
	long seconds = ((time - (hours * 3600)) - (minutes * 60));
	QString retVar = "";
	if (hours > 0)
	{
		retVar.append((hours > 9) ? "" : "0").append(QString::number(hours)).append(":");
	}
	retVar.append((minutes > 9) ? "" : "0").append(QString::number(minutes)).append(":");
	retVar.append((seconds > 9) ? "" : "0").append(QString::number(seconds));
	return retVar;
	
}
Q_EXPORT_PLUGIN2(RevMediaEnginePlugin, MediaEngine)
