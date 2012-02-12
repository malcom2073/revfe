/***************************************************************************
*   Copyright (C) 2010 by Michael Carpenter (malcom2073)                  *
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

#ifndef WINDOW_H
#define WINDOW_H
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativePropertyMap>
#include <QTimer>
#include "ComplexModel.h"
#include "../../CarPal/IPCMessage.h"
#include "mapview.h"

class Window : public QDeclarativeView
{
	Q_OBJECT
public:
	Window(bool opengl);
	void setOpengl(bool opengl);
	Q_INVOKABLE void exitClicked();
	Q_INVOKABLE void currentPlaylistViewMovementEnded();
	Q_INVOKABLE void passMessage(QString message);
	QDeclarativePropertyMap propertyMap;
	void setQMLProperty(QString name,QString value);
	void setList(QString title,QStringList list);
	void setListModel(QString title,QStringList list);
	void addToListModel(QString title,QStringList value);
	void addToList(QString title,QStringList list);
	void setComplexList(QString title,QStringList roles,QList<QList<QString> > data);
	void load();
	void createSettingsPage(QString name);
	void addSetting(QString page,QString setting,QString value,QString type,int id);
	Q_INVOKABLE void saveSetting(QString name,QString key,QString value,QString type,int id);
private:
	QMap<QString,ComplexModel*> m_complexModelList;
	QTimer *playlistListFlickTimer;

	QString m_currentSongText;
	QString m_timeRemainingText;
	QString m_timeElapsedText;
	QString m_totalTimeText;
	QString m_currentPlaylistText;
	int m_currentPlaylistIndex;
	int m_currentSongIndex;

	QDeclarativeContext *context;
	QStringList m_playList;
	QStringList m_currentPlayList;
signals:
	void passCoreMessageSignal(QString message);
	void passCoreMessageSignal(QString sender,IPCMessage msg);
	void exitSignal();
private slots:
	void playlistListFlickTimerTick();
};


#endif // WINDOW_H
