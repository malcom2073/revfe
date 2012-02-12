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

#include "revpluginmanager.h"

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QXmlStreamReader>
#include <QApplication>

PluginManager::PluginManager()
{
	m_name = "RevPluginManager";
}

void PluginManager::init()
{

}
PluginManager::~PluginManager()
{
}
QString PluginManager::getName()
{
	return m_name;
}
void PluginManager::passPluginMessage(QString sender,QString message)
{

}
void PluginManager::passPluginMessage(QString sender,IPCMessage message)
{
	if (message.getClass() == "event")
	{
		if (message.getMethod() == "throw")
		{

		}
		else if (message.getMethod() == "initstarted")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initstarted",QStringList()));
		}
		else if (message.getMethod() == "initcomplete")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "plugin_manager_install_status" << "Idle"));
			emit passCoreMessage(m_name,IPCMessage("core","event","initcomplete",QStringList()));
		}
		else if (message.getMethod() == "initclose")
		{
			emit passCoreMessage(m_name,IPCMessage("core","event","initclose",QStringList()));
		}
	}
	else if (message.getClass() == "plugin")
	{
		if (message.getMethod() == "install")
		{
			qDebug() << "Install called!";
			if (message.getArgs().size() > 0)
			{
				//There are always at least two files in a zip file. install.xml, and README (Case Sensitive)
				//install.xml defines what files to install, and README is for manual installation.
				//Both these files get removed, and the folder deleted when we are finished.
				QString filename = message.getArgs()[0];
				if (filename.endsWith(".zip")) //Zip files are currently the only accepted install type.
				{
					qDebug() << "File finished" << filename;
					QString folder = filename.mid(0,filename.lastIndexOf("/")+1);
					QString fname = filename.mid(filename.lastIndexOf("/")+1);
					fname = fname.mid(0,fname.length()-4);
					//filename = filename.mid(0,filename.length()-4);
					qDebug() << "File finished" << folder << fname;
					QProcess unzipproc;

					//Technically could make this just "unzip" for both OS's... since they use the same syntax.
				#ifdef Q_OS_WIN32
					if (QFile::exists("unzip.exe"))
					{
						unzipproc.start("unzip.exe",QStringList() << filename << "*" << "-d" + fname);
					}
					else
					{
						qDebug() << "Unable to find unzip.exe. Is it in the executable folder?";
						return;
					}
				#endif
				#ifdef Q_OS_LINUX
					unzipproc.start("unzip",QStringList() << filename << "*" << "-d" + fname);
				#endif
					unzipproc.waitForFinished();
					QFile myFile(fname +"/" + "install.xml");
					if (!myFile.open(QIODevice::ReadOnly))
					{
						qDebug() << "Unable to open install.xml";
						return;
					}
					QByteArray myArray = myFile.readAll();
					myFile.close();
					QXmlStreamReader xml(myArray);
					/*<install>
						<files count="2">
							<file name="FuelPricesPlugin.dll" target="%REVFEDIR%/plugins"/>
							<file name="FuelPrices.xml" target="%REVFEDIR%/plugins"/>
						</files>
					</install>*/
					QList<QString> filelist;
					QList<QString> targetlist;
					QList<QString> skinnamelist;
					QList<QString> skinfilelist;
					QList<QString> skintargetlist;
					while (!xml.atEnd())
					{
						if (xml.name() == "install" && xml.isStartElement())
						{
							xml.readNext();
							while (xml.name() != "install")
							{
								if (xml.name() == "files" && xml.isStartElement())
								{
									int count = xml.attributes().value("count").toString().toInt();
									xml.readNext();
									while (xml.name() != "files")
									{
										if (xml.name() == "file" && xml.isStartElement())
										{
										QString name = xml.attributes().value("name").toString();
										QString target = xml.attributes().value("target").toString();
										if (!(target.endsWith("/") && target.endsWith("\\")))
										{
											target.append("/");
										}
										filelist.append(name);
										targetlist.append(target.replace("%REVFEDIR%",QApplication::applicationDirPath()));
										}
										if (xml.name() == "skin" && xml.isStartElement())
										{
											QString name = xml.attributes().value("name").toString();
											QString file = xml.attributes().value("file").toString();
											QString target = xml.attributes().value("target").toString();
											skinnamelist.append(name);
											skinfilelist.append(file);
											skintargetlist.append(target);
										}

										xml.readNext();
									}
								}
								xml.readNext();
							}
						}
						xml.readNext();
					}
					qDebug() << "Done reading install.xml" << filelist.size() << skinnamelist.size();
					//At this point, we have a list of all the files that need to be installed.
					//Lets go through, and check if they exist. If they do, we should overwrite them.
					//Later, once notifications are complete, we will ask the user for verification that they want to overwrite.
					//If files do exist, we also need to check to see if the skin file is already installed.
					bool exists = false;
					for (int i=0;i<filelist.size();i++)
					{
						QString name = filelist[i];
						QString target = targetlist[i];
						if (QFile::exists(target + name))
						{
							exists = true;
							//File exists. Let's back it up so we can overwrite it.
							QFile::rename(target + name,target + name + ".bak");
						}
					}
					for (int i=0;i<skinfilelist.size();i++)
					{
						//QFile::rename(fname + "/" + skinfilelist[i],"QML/" + skinfilelist[i]);
						if (QFile::exists("QML/" + skinfilelist[i]))
						{
							exists = true;
							QFile::rename("QML/" + skinfilelist[i],"QML/" + skinfilelist[i] + ".bak");
						}
					}


					bool failure = false;
					for (int i=0;i<filelist.size();i++)
					{
						QString name = filelist[i];
						QString target = targetlist[i];
						qDebug() << "Installing file" << fname + "/" + name << "to" << target + name;
						if (!QFile::rename(fname + "/" + name,target + name))
						{
							qDebug() << "Error installing file!";
							failure = true;
						}
					}
					if (exists)
					{

					}
					if (!failure && skinnamelist.size() > 0)
					{
						//Try to install the skin here.
						QFile myFile("QML/Skin.xml");
						if (!myFile.open(QIODevice::ReadOnly))
						{
							qDebug() << "Unable to open Skin.xml";
							failure = true;
						}
						else
						{
							QByteArray myArray = myFile.readAll();
							QByteArray myEndArray;
							myFile.close();
							QXmlStreamReader skin(myArray);
							QXmlStreamWriter skinwriter(&myEndArray);
							skinwriter.writeStartDocument();
							skinwriter.writeStartElement("skin");
							skinwriter.writeCharacters("\n");
							while (!skin.atEnd())
							{
								if (skin.name() == "pagelist" && skin.isStartElement())
								{
									skinwriter.writeCharacters("\t");
									skinwriter.writeStartElement("pagelist");
									QString name = skin.attributes().value("name").toString();
									QString type = skin.attributes().value("type").toString();
									skinwriter.writeAttribute("name",skin.attributes().value("name").toString());
									skinwriter.writeAttribute("filename",skin.attributes().value("filename").toString());
									skinwriter.writeAttribute("main",skin.attributes().value("main").toString());
									skinwriter.writeAttribute("x",skin.attributes().value("x").toString());
									skinwriter.writeAttribute("y",skin.attributes().value("y").toString());
									skinwriter.writeAttribute("width",skin.attributes().value("width").toString());
									skinwriter.writeAttribute("height",skin.attributes().value("height").toString());
									skinwriter.writeAttribute("type",skin.attributes().value("type").toString());
									skinwriter.writeCharacters("\n\t");
									skin.readNext();
									while (skin.name() != "pagelist")
									{
										if (skin.name() == "page" && skin.isStartElement())
										{
											bool found = false;
											if (exists)
											{
												for (int i=0;i<skinnamelist.size();i++)
												{
													if (skinnamelist[i] == skin.attributes().value("name").toString() && skinfilelist[i] == skin.attributes().value("file").toString())
													{
														found = true;
													}
												}
											}
											if (!found)
											{
												skinwriter.writeCharacters("\t");
												skinwriter.writeStartElement("page");
												skinwriter.writeAttribute("name",skin.attributes().value("name").toString());
												skinwriter.writeAttribute("file",skin.attributes().value("file").toString());
												skinwriter.writeEndElement();
												skinwriter.writeCharacters("\n\t");
											}
										}
										skin.readNext();
									}
									if (type == "virtual")
									{
										for (int i=0;i<skintargetlist.size();i++)
										{
											if (skintargetlist[i] == name)
											{
												skinwriter.writeCharacters("\t");
												skinwriter.writeStartElement("page");
												skinwriter.writeAttribute("name",skinnamelist[i]);
												skinwriter.writeAttribute("file",skinfilelist[i]);
												skinwriter.writeEndElement();
												skinwriter.writeCharacters("\n\t");
											}
										}
									}
									skinwriter.writeEndElement();
									skinwriter.writeCharacters("\n");
								}
								skin.readNext();
							}
							skinwriter.writeEndElement();
							skinwriter.writeEndDocument();
							QFile::rename("QML/Skin.xml","QML/Skin.xml.Backup");
							QFile setFile("QML/Skin.xml");
							if (!setFile.open(QIODevice::ReadWrite))
							{
								qDebug() << "Unable to open new skin file!";
							}
							setFile.write(myEndArray);
							setFile.close();
							qDebug() << "Done writing settings";

							for (int i=0;i<skinfilelist.size();i++)
							{
								QFile::rename(fname + "/" + skinfilelist[i],"QML/" + skinfilelist[i]);
							}
						}
					}
					QFile::remove(filename);
					if (failure)
					{
						qDebug() << "Plugin failed to install. Removing files";

						for (int i=0;i<filelist.size();i++)
						{
							QString name = filelist[i];
							QString target = targetlist[i];
							QFile::remove(target + name);
							QFile::remove(fname + "/" + name);
						}
						QFile::remove(fname + "/install.xml");
						QFile::remove(fname + "/README");
						QDir tmpfolder;
						tmpfolder.rmdir(fname);
						//emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:notification" << "Plugin install failed:" << filename));
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "plugin_manager_install_status" << "Plugin install Failed: " + filename));
					}
					else
					{
						qDebug() << "Plugin succesfully installed!";
						for (int i=0;i<filelist.size();i++)
						{
							QString name = filelist[i];
							QString target = targetlist[i];
							QFile::remove(fname + "/" + name);
						}
						QFile::remove(fname + "/install.xml");
						QFile::remove(fname + "/README");
						QDir tmpfolder;
						tmpfolder.rmdir(fname);
						emit passCoreMessage(m_name,IPCMessage("core","event","throw",QStringList() << "event:propertyset" << "1" << "plugin_manager_install_status" << "Plugin install succeded: " + filename));
					}
				}
			}
		}
	}
}
Q_EXPORT_PLUGIN2(RevPluginManagerPlugin, PluginManager)
