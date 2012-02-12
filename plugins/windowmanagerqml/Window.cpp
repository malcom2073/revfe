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

#include "Window.h"
#include <QMessageBox>
#include <QDebug>
#include <QDeclarativeEngine>
#include <QVariant>
#include <QDeclarativeItem>
#include <QGraphicsObject>
#include <QThread>
#include <QGLFormat>
#include <QGLWidget>

Window::Window(bool opengl) : QDeclarativeView()
{
	setWindowFlags(Qt::FramelessWindowHint);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
	//QGLFormat format = QGLFormat::defaultFormat();
	//format.setSampleBuffers(false);
	//format.setDoubleBuffer(true);
	//setViewport(new QGLWidget(format));
	context = rootContext();
	context->setContextProperty("backgroundColor",QColor(Qt::yellow));
	context->setContextProperty("playlistListModel",QVariant::fromValue(m_playList));
	context->setContextProperty("window",this);
	context->setContextProperty("propertyMap",&propertyMap);
	qmlRegisterType<MapView>("MapView",0,1,"MapView");

	//setCurrentSongText("Test");
	engine()->addImportPath(".");
	engine()->addPluginPath(".");

	playlistListFlickTimer = new QTimer(this);
	connect(playlistListFlickTimer,SIGNAL(timeout()),this,SLOT(playlistListFlickTimerTick()));

	QStringList test;
	test << "App One";
	test << "App Two";
//	context->setContextProperty("utilityAppModel",QVariant::fromValue(test));
	QList<QList<QString> > tmp;
	tmp.append(QList<QString>());
	tmp.append(QList<QString>());
	tmp[0].append("Name1");
	tmp[0].append("Value1");
	tmp[1].append("Name2");
	tmp[1].append("Value2");
	QStringList roles;
	roles << "name" << "value";
	setComplexList("TestVal",roles,tmp);
}
void Window::setOpengl(bool opengl)
{
	if (opengl)
	{
		QGLFormat format = QGLFormat::defaultFormat();
		format.setSampleBuffers(false);
		format.setDoubleBuffer(true);
		setViewport(new QGLWidget(format));
	}
	else
	{
		setViewport(new QWidget());
	}
}
void Window::saveSetting(QString name,QString key,QString value,QString type,int id)
{
	emit passCoreMessageSignal("Window",IPCMessage("core","settings","save",QStringList() << name << key << value << type << QString::number(id)));
}
void Window::setComplexList(QString title,QStringList roles,QList<QList<QString> > data)
{
	//qDebug() << "Complex list:" << title << roles << data;
	ComplexModel *mod = new ComplexModel();
	mod->setRoles(roles);
	mod->setData(data);
	if (m_complexModelList.contains(title))
	{
		context->setContextProperty(title,QVariant());
		m_complexModelList[title]->deleteLater();
	}
	m_complexModelList[title] = mod;
	context->setContextProperty(title,mod);
	//context->setContextProperty();

	//propertyMap.setProperty(title,mod);
}

void Window::createSettingsPage(QString name)
{
	qDebug() << "Creating settings page:" << name;
	if (!QMetaObject::invokeMethod(rootObject(),"addSettingsPage",Q_ARG(QVariant,QVariant(name))))
	{
		qDebug() << "Unable to invoke method: addSettingsPage";
	}
}

void Window::addSetting(QString page,QString setting,QString value,QString type,int id)
{
	if (!QMetaObject::invokeMethod(rootObject(),"addSettingsValue",Q_ARG(QVariant,QVariant(page)),Q_ARG(QVariant,QVariant(setting)),Q_ARG(QVariant,QVariant(value)),Q_ARG(QVariant,QVariant(type)),Q_ARG(QVariant,QVariant(id))))
	{
		qDebug() << "Unable to invoke method: addSettingsValue";
	}
}

void Window::load()
{
	setSource(QUrl::fromLocalFile("QML/main.qml"));
	this->setGeometry(-1,-1,800,600);
}

void Window::setList(QString title,QStringList list)
{
	//qDebug() << "List set:" << title << list.size();
	//context->setContextProperty(title,QVariant::fromValue(list));
	propertyMap.setProperty(title.toAscii(),QVariant(list));
}
void Window::addToListModel(QString title,QStringList value)
{
	//qDebug() << "Add to list model:" << title << value;
	if (m_complexModelList.contains(title))
	{
		for (int i=0;i<value.size();i++)
		{
			QList<QString> list;
			list.append(value[i]);
			m_complexModelList[title]->addRow(list);
		}
	}
	else
	{
		qDebug() << "No such model in m_complexModelList";
	}
}

void Window::setListModel(QString title,QStringList list)
{
	//qDebug() << "List Model set:" << title << list.size();
	ComplexModel *mod = new ComplexModel();
	QList<QList<QString> > data;
	//data.append(QList<QString>());
	for (int i=0;i<list.size();i++)
	{
		data.append(QList<QString>());
		data[data.size()-1].append(list[i]);
		//data[0].append(list[i]);
	}
	QStringList roles;
	roles << "text";
	mod->setRoles(roles);
	mod->setData(data);
	if (m_complexModelList.contains(title))
	{
		context->setContextProperty(title,QVariant());
		m_complexModelList[title]->deleteLater();
	}
	m_complexModelList[title] = mod;
	context->setContextProperty(title,mod);
}

void Window::addToList(QString title,QStringList list)
{
	//qDebug() << "Add to list:" << title;
	if (propertyMap.contains(title))
	{

		//QVariant val = propertyMap.value(title);
		//val.append(list);
		//qDebug() <<  propertyMap.value(title).toStringList().size();
	}
	else
	{
		qDebug() << "No such list exists!";
	}
}

void Window::currentPlaylistViewMovementEnded()
{
	qDebug() << "Flick finished";
	if (!playlistListFlickTimer->isActive())
		playlistListFlickTimer->start(1000);

}
void Window::playlistListFlickTimerTick()
{
	qDebug() << "Resetting";
	playlistListFlickTimer->stop();
	//setCurrentSongIndex(m_currentSongIndex);
}

void Window::passMessage(QString message)
{
	qDebug() << "Window Thread:" << QThread::currentThread();
	qDebug() << "Window message:" << message;
	emit passCoreMessageSignal(message);
}
void Window::setQMLProperty(QString name,QString value)
{
	//qDebug() << "Set QML Property:" << name << value;
	propertyMap.setProperty(name.toAscii(),QVariant(value));
}

void Window::exitClicked()
{
	emit exitSignal();
}
