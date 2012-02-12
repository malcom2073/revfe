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
#include <QDir>
#include <QString>
#include <QPluginLoader>
#include <QSessionManager>
#include <QScriptEngine>
#include <QDateTime>
#include <QTimer>
#include "debugwindow.h"
#include "windowinterface.h"
#include "pluginclass.h"
#include "complexmodel.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativePropertyMap>



class SettingValue
{
public:
	QString type;
	QString value;
	QString key;
	int id;
};

class SettingsClass
{
public:
	bool debug;
	SettingsClass() { }
	QMap<QString,QMap<QString,SettingValue> > m_settingsMap;
	QMap<QString,QMap<QString,QList<SettingValue> > > m_multiSettingsMap;
	QMap<QString,QList<QString> > m_keyList;
	QList<QString> m_pluginList;
};




class CoreApp : public QObject
{
	Q_OBJECT
public:
	CoreApp();
	~CoreApp();
	Q_INVOKABLE void passMessage(QString message);
	Q_INVOKABLE void saveSetting(QString name,QString key,QString value,QString type,int id);
	Q_INVOKABLE void exitClicked();

private:
	QMap<QString,ComplexModel*> m_modelMap;
	QDeclarativeView *window;
	QList<QDeclarativeView*> m_windowList;
	void sendInitStarted();
	void sendInitComplete();
	QMap<QString,bool> m_initStartedMap;
	QDateTime m_initStartedTime;
	QDateTime m_initCompleteTime;
	QMap<QString,bool> m_initCompleteMap;
	QMap<QString,bool> m_initCloseMap;
	QScriptEngine *eng;
	void saveSettings();
	SettingsClass settings;
	QList<PluginClass*> pluginList;
	QList<QPair<QString,QObject*> > eventHandlerList;
	QList<QPair<QString,QObject*> > extraHandlerList;
	DebugWindow *debugWindow;
	PluginClass* findPluginByName(QString name);
	QList<QPair<QString,QObject*> > threadEventHandlerList;
	void passedCoreMessage(QString,IPCMessage,bool blocking);
	QMap<QString,QStringList> pluginAttributes;
	bool initsThrown;
	QDeclarativePropertyMap propertyMap;
private slots:
	void initStartedTimeTick();
	void initCompleteTimerTick();
signals:
	void passPluginMessage(QString,IPCMessage);
public slots:
	void passedCoreMessage(QString,IPCMessage);
	void passedCoreMessageBlocking(QString,IPCMessage);
	void passedCoreModel(QString name, QObject *model);
	void passedCoreGUIItem(QObject *item);
	void pluginLoadFail(QString pluginfilename);
	void dataRequest(QSessionManager &manager);
};
