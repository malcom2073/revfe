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

#include "embeddingengine.h"
#include <QMessageBox>

#ifdef Q_OS_WIN32
#include <windows.h>

BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam);
#endif //Q_OS_WIN32
#ifdef Q_OS_LINUX

Display *g_pdisplayX11;

void iterWindows(Window p_win,const char* p_cpszWindowTitle,Window& p_winFound)
{
	XWindowAttributes attr;
	XGetWindowAttributes( g_pdisplayX11, p_win, &attr);
	if ( attr.map_state == IsViewable)
	{
		XTextProperty tp1;
		XGetWMName( g_pdisplayX11, p_win, &tp1);
		if (tp1.value != 0x0)
		{
			if (!strcmp(p_cpszWindowTitle,(char *)tp1.value))
			p_winFound = p_win;
		}
	}
	unsigned int uiNumChildren;
	Window winNew, winParent, *pwinChildren;
	if (p_winFound == 0)
	{
		if ( XQueryTree( g_pdisplayX11, p_win, &winNew, &winParent,&pwinChildren, &uiNumChildren))
		{
			for (unsigned int ui = 0; ui < uiNumChildren; ++ui)
			{
				iterWindows( pwinChildren[ui], p_cpszWindowTitle, p_winFound);
				if (p_winFound != 0) { break; }
			}
			XFree( pwinChildren);
		}
	}
}

Window FindWindowByTitle(Window p_winRoot, const char* p_cpszWindowTitle)
{
	Window winFound = 0;
	if (strlen(p_cpszWindowTitle) > 0)
	{
		iterWindows( p_winRoot, p_cpszWindowTitle, winFound);
	}
	return winFound;
}

#endif //Q_OS_LINUX

void EmbeddingEngine::init()
{
	appRunning = false;
}
QString EmbeddingEngine::getName()
{
	return "EmbeddingEngine";
}
void EmbeddingEngine::passPluginMessage(QString sender,QString message)
{

}
void EmbeddingEngine::passPluginMessage(QString sender,IPCMessage message)
{
	#ifdef Q_OS_WIN32
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			passCoreMessage("EmbeddingEngine",IPCMessage("WindowManager","window","getwinidlist",QStringList() << "EmbeddingEngine" << "window" << "set-windowid"));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","initclose",QStringList()));
		}
	}
	else if (message.getClass() == "window")
	{
		if (message.getMethod() == "embed")
		{
			if (appRunning)
			{
				//return;
			}
			if (message.getArgs().count() > 1)
			{
				bool apprun = false;
				for (int i=0;i<appRunningList.count();i++)
				{
					if (appRunningList[i].first == message.getArgs()[0])
					{
						apprun = appRunningList[i].second;
					}
				}
				if (apprun)
				{
					return;
				}
				//windowname currenthandle fullpath x y width height
				QProcess *tmpEmbeddedProcess = new QProcess(this);
				QString path = message.getArgs()[2];
				tmpEmbeddedProcess->start(QString("\"").append(path).append("\""));
				appRunning = true;
				if (!tmpEmbeddedProcess->waitForStarted(10000))
				{
					QProcess::ProcessError error = tmpEmbeddedProcess->error();
					QString errorStr = tmpEmbeddedProcess->errorString();
					int ierror = 1;
					appRunning = false;
					delete tmpEmbeddedProcess;
					passCoreMessage("EmbeddingEngine",IPCMessage("core","debug","debug",QStringList() << QString("Failed to start app: ").append(errorStr)));
					return;
				}
				//Sleep(100);
				
				bool end = false;
				_PROCESS_INFORMATION* tmp = tmpEmbeddedProcess->pid();
				int attempt = 0;
				bool endLoop = false;
				HWND h = ::GetTopWindow(0);
				while (!endLoop)
				{
					h = ::GetTopWindow(0);
					Sleep(10);
					attempt++;
					if (attempt > 1000)
					{
						endLoop = true;
					}
					while ((h) || (end))
					{
						DWORD pid;
						DWORD dwThreadId = ::GetWindowThreadProcessId(h,&pid);
						
						
						if (pid == tmp->dwProcessId)
						{
							char *tmpChar = new char[50];
							GetWindowTextA(h,tmpChar,50);
							QString title = QString(tmpChar);
							delete tmpChar;
							if (message.getArgs()[0] == title)
							{
								end = true;
								endLoop = true;
								break;
							}
						}
						//if (dwThreadId == tmp->dwThreadId)
						//{
						//	end = true;
						//	endLoop = true;
						//	break;
						//}
						DWORD t1=tmp->dwThreadId;
						HANDLE t2=tmp->hProcess;
						HANDLE t3=tmp->hThread;
						h = ::GetNextWindow(h,GW_HWNDNEXT);
					}
				}
				if (end)
				{
					
					QString first = message.getArgs()[0];
					QString second = "";
					QString cmp1 = message.getArgs()[1];
					if (message.getArgs()[1].split(":")[0] == "HANDLE")
					{
						second = message.getArgs()[1].split(":")[1];
					}
					else
					{
						second = QString::number(windowHandleList[message.getArgs()[1].split(":")[1]]);
					}
					//QString second = message.getArgs()[1];
					char *tmpChar = new char[50];
					GetWindowTextA(h,tmpChar,50);
					QString title = QString(tmpChar);
					delete tmpChar;
					if (!SetParent(h,(HWND)second.toLong()))
					{
						emit passCoreMessage("EmbeddingEngine",IPCMessage("core","debug","debug",QStringList() << "Failed to embed window directly: " << title));
					}
					else
					{
						emit passCoreMessage("EmbeddingEngine",IPCMessage("core","debug","debug",QStringList() << "Embedding window directly: " << title));
					}
					//SendMessage(h, WM_KEYDOWN, 70, 0);
					//SendMessage(h, WM_KEYDOWN, 70, 0);
					//if (!SetWindowPos(h,0,message.getArgs()[3].toInt(),message.getArgs()[4].toInt(),message.getArgs()[5].toInt(),message.getArgs()[6].toInt(),NULL))
					//{
						
					//}
					//else
					//{

					//}
					if (message.getArgs()[1].split(":")[0] == "HANDLE")
					{
						if (!SetWindowPos(h,HWND_BOTTOM,message.getArgs()[3].toInt(),message.getArgs()[4].toInt(),message.getArgs()[5].toInt(),message.getArgs()[6].toInt(),SWP_NOSENDCHANGING))
						{
							int failure = 1;
						}
						SendMessage(h, WM_KEYDOWN, 70, 0);
						SendMessage(h, WM_KEYUP, 70, 0);
						SetWindowPos(h,HWND_BOTTOM,message.getArgs()[3].toInt(),message.getArgs()[4].toInt(),message.getArgs()[5].toInt(),message.getArgs()[6].toInt(),SWP_NOSENDCHANGING);
						QStringList args;
						args << "event:embeddingengine_windowembedded" << QString::number((long)h) << title << message.getArgs()[3] << message.getArgs()[4] << message.getArgs()[5] << message.getArgs()[6];
						emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","throw",args));
					}
					else
					{
						QString winName = message.getArgs()[1].split(":")[1];
						if (!SetWindowPos(h,HWND_BOTTOM,windowRectList[winName].x(),windowRectList[winName].y(),windowRectList[winName].width(),windowRectList[winName].height(),SWP_NOSENDCHANGING))
						{
							int failure = 1;
						}
						QStringList args;
						args << "event:embeddingengine_windowembedded" << QString::number((long)h) << title << QString::number(windowRectList[winName].x()) << QString::number(windowRectList[winName].y()) << QString::number(windowRectList[winName].width()) << QString::number(windowRectList[winName].height());
						emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","throw",args));
					}
					EnumChildWindows((HWND)second.toLong(),&EnumChildProc,(LPARAM)h);
					windowList.append(QPair<QString,HWND>(message.getArgs()[0],h));
					processList.append(QPair<QString,QProcess*>(message.getArgs()[0],tmpEmbeddedProcess));
					appRunningList.append(QPair<QString,bool>(message.getArgs()[0],true));
				}
				else
				{
					QString first = message.getArgs()[0];
					QString cmp3 = message.getArgs()[1];
					QString second = "";
					if (message.getArgs()[1].split(":")[0] == "HANDLE")
					{
						second = message.getArgs()[1].split(":")[1];
					}
					else
					{
						second = windowHandleList[message.getArgs()[1].split(":")[1]];
					}
					//QString second = message.getArgs()[1];
					HWND tmpHwnd = FindWindowA(NULL,first.toStdString().c_str());
					if (tmpHwnd)
					{
						char *tmpChar = new char[50];
						GetWindowTextA(tmpHwnd,tmpChar,50);
						QString title = QString(tmpChar);
						delete tmpChar;
						passCoreMessage("EmbeddingEngine",IPCMessage("core","debug","debug",QStringList() << "Embedding window by name: " << title));
						SetParent(tmpHwnd,(HWND)second.toLong());
						//SetWindowPos(tmpHwnd,0,message.getArgs()[3].toInt(),message.getArgs()[4].toInt(),message.getArgs()[5].toInt(),message.getArgs()[6].toInt(),NULL);						
						if (message.getArgs()[1].split(":")[0] == "HANDLE")
						{
							SetWindowPos(tmpHwnd,HWND_BOTTOM,message.getArgs()[3].toInt(),message.getArgs()[4].toInt(),message.getArgs()[5].toInt(),message.getArgs()[6].toInt(),NULL);
							QStringList args;
							args << "event:embeddingengine_windowembedded" << QString::number((long)h) << title << message.getArgs()[3] << message.getArgs()[4] << message.getArgs()[5] << message.getArgs()[6];
							emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","throw",args));
						}
						else
						{
							QString winName = message.getArgs()[1].split(":")[1];
							SetWindowPos(tmpHwnd,HWND_BOTTOM,windowRectList[winName].x(),windowRectList[winName].y(),windowRectList[winName].width(),windowRectList[winName].height(),NULL);
							QStringList args;
							args << "event:embeddingengine_windowembedded" << QString::number((long)h) << title << QString::number(windowRectList[winName].x()) << QString::number(windowRectList[winName].y()) << QString::number(windowRectList[winName].width()) << QString::number(windowRectList[winName].height());
							emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","throw",args));
						}

						windowList.append(QPair<QString,HWND>(message.getArgs()[0],tmpHwnd));
						processList.append(QPair<QString,QProcess*>(message.getArgs()[0],tmpEmbeddedProcess));
						appRunningList.append(QPair<QString,bool>(message.getArgs()[0],true));
					}
					else
					{
						passCoreMessage("EmbeddingEngine",IPCMessage("core","debug","debug",QStringList() << "Error embedding..."));
						appRunning = false;
						tmpEmbeddedProcess->kill();
						delete tmpEmbeddedProcess;
					}
				}

				//HWND tmpHwnd = FindWindowA(NULL,first.toStdString().c_str());
				//HWND parentHwnd = FindWindowA(NULL,second.toStdString().c_str());
			}
		}
		else if (message.getMethod() == "close-embed")
		{
			for (int i=0;i<processList.count();i++)
			{
				if (processList[i].first == message.getArgs()[0])
				{
					processList[i].second->kill();
					appRunning = false;
					delete processList[i].second;
					processList.removeAt(i);
					windowList.removeAt(i);
				}
			}
		}
		else if (message.getMethod() == "set-windowid")
		{
			for (int i=0;i<message.getArgs().count();i++)
			{
				QString nameStr = message.getArgs()[i];
				QString listStr = message.getArgs()[i+1];
				windowHandleList[message.getArgs()[i]] = message.getArgs()[i+1].toLong();
				i++;
			}
			
		}
		else if (message.getMethod() == "setgeometry")
		{
			windowRectList[message.getArgs()[0]] = QRect(message.getArgs()[1].toInt(),message.getArgs()[2].toInt(),message.getArgs()[3].toInt(),message.getArgs()[4].toInt());
		}
	}
	#endif //Q_OS_WIN32
	#ifdef Q_OS_LINUX
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			//passCoreMessage("EmbeddingEngine",IPCMessage("WindowManager","window","getwinidlist",QStringList() << "EmbeddingEngine" << "window" << "set-windowid"));
			emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage("EmbeddingEngine",IPCMessage("core","event","initclose",QStringList()));
		}
	}
	else if (message.getClass() == "window")
	{
		if (message.getMethod() == "embed")
		{
		}
	}
	#endif //Q_OS_LINUX
}
Q_EXPORT_PLUGIN2(EmbeddingEnginePlugin, EmbeddingEngine)


#ifdef Q_OS_WIN32
BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam)
{
	if (hwnd == (HWND)lParam)
	{
		//SetWindowPos(hwnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE);
		return true;
	}
	//SetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE);
	return true;
}
#endif //Q_OS_WIN32
