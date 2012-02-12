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

#include <QApplication>

#include <QTimer>

#include "baseinterface.h"
#include "ipcmessage.h"
#include "id3tag.h"
#include "mediaplayer.h"
class MediaEngine : public BaseInterface
{
	Q_OBJECT
	Q_INTERFACES(BaseInterface)

public:
	MediaEngine();
	void init();
	QString getName();
private:
	QString m_name;
	bool autoStartNextList;
	long autoStartPos;
	bool autoStartNextPos;
	int autoStartNum;
	bool die;
	//Phonon::MediaObject *mediaObject;
	//Phonon::AudioOutput *audioObject;
	//QList<Phonon::MediaSource> sourceList;
	QList<QString> sourceList;
	QList<QString> sourceTitleList;
	QList<ID3Tag> tagList;
	void play(int songNumber);
	void setPlayList();
	void stop();
	void pause();
	int m_currentIndex;
	QTimer *mediaTimer;
	QString playlistName;
	QString formatTime(long time);
	bool muted;
	//Phonon::Path newPath;
	MediaPlayer *mediaPlayer;
signals:
	void passCoreMessage(QString sender,QString message);
	void passCoreMessage(QString sender,IPCMessage message);
	void passCoreMessageBlocking(QString sender,IPCMessage message);
public slots:
	void passPluginMessage(QString sender,QString message);
	void passPluginMessage(QString sender,IPCMessage message);
private slots:
	void audioTimeChanged(unsigned long current,unsigned long total);
	void audioAlmostFinished();
	void audioChanged();
	void mediaTimerTimeout();
};
