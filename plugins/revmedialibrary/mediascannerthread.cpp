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

#include "mediascannerthread.h"
#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QDebug>
MediaScannerThread::MediaScannerThread()
{
	//TagLib::FileRef testfile("C:\\Documents and Settings\\Michael\\My Documents\\Visual Studio 2008\\Projects\\CarPal\\CarPal\\Music\\Evanescence - The Open Door\\Evanescence - The open door - 01 - Sweet sacrifice.mp3");
	//TagLib::Tag *tmpTag = testfile.tag();
	//QMessageBox::information(0,"Test",QString(tmpTag->artist().to8Bit().c_str()));
	singlePass = false;
	multiPass = false;
}
void MediaScannerThread::setMediaList(QList<PlayListClass*> *playlists)
{
	m_playLists = playlists;
}
void MediaScannerThread::setMultiPass(QStringList filenames)
{
	multiPassFileNames.append(filenames);
	multiPass = true;
}
void MediaScannerThread::setSinglePass(QString filename)
{
	singlePass = true;
	singlePassFileName = filename;
}
void MediaScannerThread::setDirectory(QString dir)
{
	scanDir = dir;
}
void MediaScannerThread::run()
{
	if (multiPass)
	{
		qDebug() << "MediaScannerThread: multi non-single pass selected";
		multiPass = false;
		for (int i=0;i<multiPassFileNames.count();i++)
		{
			QString fullPath = multiPassFileNames.at(i);
			TagLib::FileRef testfile(fullPath.toStdString().c_str());
			if (!testfile.isNull())
			{
				ID3Tag *tmpTag = 0;
				if (!tmpTag)
				{
					tmpTag = new ID3Tag();
					tmpTag->valid = false;
				}
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
						if (!tmpTag->valid)
						{
							tmpTag->artist = tag->artist().toCString();
							tmpTag->title = tag->title().toCString();
							tmpTag->filename = fullPath;
							emit tagReceived("",0,tmpTag);
						}	
					}

				}
				else
				{
					TagLib::Tag *tag = testfile.tag();
					if (!tmpTag->valid)
					{
						tmpTag->artist = tag->artist().toCString();
						tmpTag->title = tag->title().toCString();
						tmpTag->filename = fullPath;
						emit tagReceived("",0,tmpTag);
					}
				}
			}
			else
			{
				qDebug() << "MediaScannerThread: Invalid File in multipass" << fullPath;
				int stop = 1;
			}
			QThread::msleep(50);
		}
		multiPassFileNames.clear();
		
	}
	else if (singlePass)
	{
		qDebug() << "MediaScannerThread: Non-multi single pass selected";
		singlePass = false;
		QString fullPath = singlePassFileName;
		TagLib::FileRef testfile(fullPath.toStdString().c_str());
		if (!testfile.isNull())
		{
			TagLib::Tag *tag = testfile.tag();
		//if (tag)
		//{
			ID3Tag *tmpTag = 0;
			if (!tmpTag)
			{
				tmpTag = new ID3Tag();
				tmpTag->valid = false;
			}
			if (!tmpTag->valid)
			{
				tmpTag->artist = tag->artist().toCString();
				tmpTag->title = tag->title().toCString();
				tmpTag->filename = fullPath;
				emit tagReceived("",0,tmpTag);
			}
			//ID3Tag *tmpTag = new ID3Tag();
		}
		else
		{
			qDebug() << "MediaScannerThread: Invalid File in singlepass" << fullPath;
		}

	}
	else
	{
		qDebug() << "MediaScannerThread: Non-multi non-single pass selected";
		bool firstScan = true;
		int delay=0;
		bool anyValid = true;
		while (anyValid)
		{
			anyValid = false;
			if (firstScan)
			{
				delay=0;
				//firstScan = false;
			}
			else
			{	
				delay=20;
			}
			for (int i=0;i<m_playLists->count();i++)
			{
				for (int j=0;j<m_playLists->at(i)->size();j++)
				{

					QString fullPath = m_playLists->at(i)->getFileNum(j)->fullPath();
//					qDebug() << fullPath;
					TagLib::FileRef testfile(fullPath.toStdString().c_str());
					if (!testfile.isNull())
					{
						ID3Tag *tmpTag = m_playLists->at(i)->getFileNum(j)->getId3Tag();
						if (!tmpTag)
						{
							tmpTag = new ID3Tag();
							tmpTag->valid = false;
							anyValid = true;
						}
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
								#endif
								TagLib::Tag *tag = testfile.tag();
								if (!tmpTag->valid)
								{
									tmpTag->artist = tag->artist().toCString();
									tmpTag->title = tag->title().toCString();
									tmpTag->filename = fullPath;
									emit tagReceived(m_playLists->at(i)->title(),j,tmpTag);
								}	
							}
						}
						else
						{
							TagLib::Tag *tag = testfile.tag();
							if (!tmpTag->valid)
							{
								tmpTag->artist = tag->artist().toCString();
								tmpTag->title = tag->title().toCString();
								tmpTag->valid = true;
								tmpTag->filename = fullPath;
								m_playLists->at(i)->setId3(j,tmpTag);
								anyValid = true;
								emit tagReceived(m_playLists->at(i)->title(),j,tmpTag);
							}
							else
							{
								if (tmpTag->filename != fullPath)
								{
									tmpTag->artist = tag->artist().toCString();
									tmpTag->title = tag->title().toCString();
									tmpTag->filename = fullPath;
									tmpTag->valid = true;
									m_playLists->at(i)->setId3(j,tmpTag);
									anyValid = true;
									emit tagReceived(m_playLists->at(i)->title(),j,tmpTag);								
								}
							}
						}
						//ID3Tag *tmpTag = new ID3Tag();
					}
					else
					{
						qDebug() << "MediaScannerThread: Invalid File in standardpass" << fullPath;
					}
					QThread::msleep(delay);
				}
			}
			if (firstScan)
			{
				firstScan = false;
				qDebug() << "Initial scan of id3 tags completed. ";
				if (anyValid)
				{
					qDebug() << "Moving to background  mode...";
				}
				else
				{
					qDebug() << "All files scanned, exiting thread";
				}
			}
			else
			{
				qDebug() << "Secondary id3 scan complete... continuing...";
			}
		}
	}
	/*
	int done = 1;
	return;
	QFile tmpFile("music.conf");
	if (!tmpFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{	
		printf("Error opening configuration file!!!\n");
		return;
	}
	QString musicDir(tmpFile.readLine());
	printf("Music Directory from configuration file: %s\n",musicDir.toStdString().c_str());
	tmpFile.close();
	musicDir = musicDir.mid(0,musicDir.length()-1);
	QDir tmpDir(musicDir);

	tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QDir tmpFiles(musicDir);
	tmpFiles.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList infoList = tmpDir.entryList();
	foreach (QString info,infoList)
	{
		TagLib::FileRef testfile(info.toStdString().c_str());
		TagLib::Tag *tag = testfile.tag();
		ID3Tag *tmpTag = new ID3Tag();
		tmpTag->artist = tag->artist().to8Bit().c_str();
		tmpTag->title = tag->title().to8Bit().c_str();
		//emit tagReceived(tmpTag);		
	}
	bool inLoop = true;
	//while (inLoop)
	//{
		//QThread::
	//	QThread::sleep(1);
		
	//	ID3Tag *tmpTag = new ID3Tag();
		//tmpTag->artist = 
		
	//}
	*/
}
