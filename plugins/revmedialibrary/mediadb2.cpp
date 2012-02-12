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

#include "MediaDB.h"
#include <QDateTime>


MediaDB::MediaDB()
{

}
void MediaDB::Init()
{
		playListClassList = new QList<PlayListClass*>();
	//QFile::remove("test.db");
	int rc = sqlite3_open("test.db",&dataBase);
	//sqlite3_close(dataBasePtr);

	//printf("In Media\n");
	char *zErrMsg = 0;
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

	//QString musicDir("/home/Michael/Music");
	//QDir tmpDir(musicDir);
	tmpDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QDir tmpFiles(musicDir);
	tmpFiles.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList infoList = tmpDir.entryList();
	//printf("A: %s\nB: %i\n",tmpDir.absolutePath().toStdString().c_str(),infoList.count());
	char **result;
	int nrow,ncol;

	if (sqlite3_get_table(dataBase,"select * from 'Table_Index';",&result,&nrow,&ncol,&zErrMsg) == SQLITE_OK)
	{
 		printf("Table Index Already Created... skipping...\n");
	}
	else
	{
 		printf("Table Index Missing, Creating now...\n");
		//If you don't have a Table_Index, you need to create one and go to find files
		sqlite3_exec(dataBase,QString("CREATE TABLE '").append("Table_Index").append("' (tikey INTEGER PRIMARY KEY, title TEXT);").toStdString().c_str(),0,0,&zErrMsg);
		for (int i=0;i<infoList.count();i++)
		{
			//tmpDir.cd(infoList.at(i));
			tmpFiles.cd(infoList.at(i));
			
			//printf("InfoList %s\nCount %i\n",infoList.at(i).toStdString().c_str(),tmpFiles.entryList().count());
			QString tableName = tmpFiles.absolutePath().mid(tmpFiles.absolutePath().lastIndexOf("/")+1);
			tableName.replace("'","''");
			sqlite3_exec(dataBase,QString("insert into 'Table_Index' (title) values ('").append(tableName).append("');").toStdString().c_str(),0,0,&zErrMsg);
			if (sqlite3_exec(dataBase,QString("CREATE TABLE '").append(tableName).append("' (tikey INTEGER PRIMARY KEY, title TEXT, artist TEXT,album TEXT, filename TEXT, fullPath TEXT,tracknum INTEGER,lastupdate INTEGER);").toStdString().c_str(),0,0,&zErrMsg) != SQLITE_OK)
			{
				printf("Error exec Sql: %s\n",zErrMsg);
				printf("Table Name: %s\n",tableName.toStdString().c_str());
				sqlite3_free(zErrMsg);
			}
			
			//playListManager->createPlayListFromDirectory(tmpDir.absolutePath());
			QStringList entries = tmpFiles.entryList();
			for (int j=0;j<entries.count();j++)
			{
				QString filename = entries.at(j).mid(entries.at(j).lastIndexOf("/")+1);
				if (filename.endsWith(".mp3"))
				{
					QString title="title";
					QString artist="artist";
					QString album="album";
					
					QString fullPath = tmpFiles.absolutePath().append("/").append(entries.at(j));
					int tracknum=j;
					QString total = QString("insert into '").append(tableName).append("' (rowid,title, artist, album, filename,fullPath, tracknum,lastupdate) values ('").append(QString::number(j)).append("','").append(title.replace("'","''")).append("','").append(artist.replace("'","''")).append("','").append(album.replace("'","''")).append("','").append(filename.replace("'","''")).append("','").append(fullPath.replace("'","''")).append("',").append(QString::number(tracknum)).append(",").append("0").append(");");
					//QString total = "";
					if (sqlite3_exec(dataBase,total.toStdString().c_str(),0,0,&zErrMsg) != SQLITE_OK)
					{
						printf("Error exec Sql: %s\n",zErrMsg);	
						sqlite3_free(zErrMsg);
					}
					//printf("Total Name: %s\n",total.toStdString().c_str());
				}
			}
			tmpFiles.cd("..");
		}
	}
	
	//if (sqlite3_exec(dataBase,"select * from t1;",callback,0,&zErrMsg) != SQLITE_OK)

	//sqlite3_exec(dataBase,"CREATE table 'Smile Empty Soul' (t1key INTEGER PRIMARY KEY, artist TEXT,album TEXT,filename TEXT, tracknum double);",0,0,&zErrMsg);
	//sqlite3_exec(dataBase,"insert into 'Smile Empty Soul' (artist,album,filename,tracknum) values ('Smile Empty Soul','Anxiety','Smile Empty Sould - Anxiety.mp3',2);;",0,0,&zErrMsg);
	//sqlite3_exec(dataBase,"insert into 'Smile Empty Soul' (artist,album,filename,tracknum) values ('Smile Empty Soul','Anxiety','Smile Empty Sould - Something else.mp3',3);;",0,0,&zErrMsg);
	//sqlite3_exec(dataBase,"insert into 'Smile Empty Soul' (artist,album,filename,tracknum) values ('Smile Empty Soul','Anxiety','Smile Empty Sould - Bright Side.mp3',4);;",0,0,&zErrMsg);
	//sqlite3_exec(dataBase,"insert into 'Smile Empty Soul' (artist,album,filename,tracknum) values ('Smile Empty Soul','Anxiety','Smile Empty Sould - No one knows.mp3',5);;",0,0,&zErrMsg);
	//if (sqlite3_exec(dataBase,"insert into Justice (artist,album,filename,tracknum) values ('Justice','JusticeAlbum','Justice-101.mp3',1);;",0,0,&zErrMsg) != SQLITE_OK)
	
	//if (sqlite3_exec(dataBase,"replace into 'Smile Empty Soul' (artist,album,filename,tracknum) select * from 'Smile Empty Soul' while tracknum == 3;",0,0,&zErrMsg) != SQLITE_OK)
	//if (sqlite3_exec(dataBase,"delete from 'Smile Empty Soul' where tracknum == 3",0,0,&zErrMsg) != SQLITE_OK)
	//sqlite3_exec(dataBase,"delete from 'Smile Empty Soul' where rowid == 123",0,0,&zErrMsg);
	//if (sqlite3_exec(dataBase,"insert into 'Smile Empty Soul' (rowid,artist,album,filename,tracknum) values (123,'Smile Empty Soul','Anxiety','Smile Empty Sould - No one knowsASDF.mp3',5);",0,0,&zErrMsg) != SQLITE_OK)
	//{
	//	printf("Error exec Sql: %s\n",zErrMsg);
	//	sqlite3_free(zErrMsg);
	//}
	//sqlite3_exec(dataBase,"create index 2 on 'Smile Empty Soul'
	//if (sqlite3_exec(dataBase,"select * from 'Smile Empty Soul - Smile Empty Soul';",dbCallBack,(void*)this,&zErrMsg) != SQLITE_OK)
	//{
	//	printf("Error exec Sql: %s\n",zErrMsg);
	//	sqlite3_free(zErrMsg);
	//}
	//else
	//{
	//	printf("Good!\n");
	//}
	QList<QString> tableList;
	if (sqlite3_get_table(dataBase,"select * from 'Table_Index';",&result,&nrow,&ncol,&zErrMsg) != SQLITE_OK)
	{
		printf("Error exec Sql: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		//printf("NCol: %i NRow: %i\n",ncol,nrow);
		//for (int i=0;i<ncol;i++)
		//{
			//printf("%s ",result[i]);
		//}
		//printf("\n");
		for (int i=0;i<ncol*nrow;i++)
		{
			//printf("Key: %s Value: %s\n",result[ncol + i],result[ncol + i+1]);
			tableList.push_back(result[ncol+i+1]);
			i++;
		}
		//printf("\n");
		sqlite3_free_table(result);
		//printf("Index Table Size: %i\n",tableList.size());
		
		for (int i=0;i<tableList.size();i++)
		{
			PlayListClass *tmpPlayListClass = new PlayListClass();
			tmpPlayListClass->setTitle(tableList.at(i));
			if (sqlite3_get_table(dataBase,QString("select * from '").append(tableList[i].toStdString().c_str()).append("';").toStdString().c_str(),&result,&nrow,&ncol,&zErrMsg) != SQLITE_OK)
			{
				printf("ERROR!!!!\n");
			}
			else
			{
				//printf("Rows: %i\nColumns: %i\n",nrow,ncol);
				//for (int j=0;j<ncol;j++)
				//{
					//printf("%s ",result[j]);
				//}
				//printf("\n");
				int count =0;
				for (int j=0;j<ncol*nrow;j++)
				{
					ID3Tag *id3 = new ID3Tag();
					id3->valid = false;
					//This is hit once per file in a play list.
					//printf("Database File %s\n",result[ncol+j+5]);
					tmpPlayListClass->addFile(result[ncol+j+5]);
					tmpPlayListClass->setTitle(tableList[i]);
					//artist album title track
					//(title, artist, album, filename,fullPath, tracknum)
					id3->artist = result[ncol+j+2];
					id3->album = result[ncol+j+3];
					id3->title = result[ncol+j+1];
					id3->track = QString(result[ncol+j+6]).toInt();
					tmpPlayListClass->setId3(count,id3);
					count++;
					//for (int k=0;k<ncol;k++)
					//{
						
						//printf("%s ",result[ncol+j+k]);
					//}
					//printf(" : ");
					//printf("%s - %s : ",result[ncol+j],result[ncol + j + 1]);
					j+=(ncol - 1);
					//if (!(j % ncol)) printf("\n");
				}
				//printf("\n");
				sqlite3_free_table(result);
				playListClassList->push_back(tmpPlayListClass);
			}
		}
		printf("Playlists Loaded from database, %i playlists found\n",tableList.size());

	}

}
void MediaDB::setPlayListPointer(QList<PlayListClass*> &pointer)
{
	pointer = *playListClassList;
}
int MediaDB::addEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum)
{
	return -1;
}
int MediaDB::updateEntry(QString Target_Table,int Target_Entry,QString title, QString artist, QString album,QString filename, QString fullPath, int tracknum)
{
	
	PlayListClass *targetPlayListFile;
	char *zErrMsg = 0;
	char **result = 0;
	int nrow,ncol;
	QString database_found;
	if (sqlite3_get_table(dataBase,QString("select * from 'Table_Index' where title == '").append(Target_Table).append("';").toStdString().c_str(),&result,&nrow,&ncol,&zErrMsg) != SQLITE_OK)
	{
		printf("Error exec Sql 'select * from 'Table_Index': %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	for (int j=0;j<ncol*nrow;j++)
	{
		//printf("Database hits: %s\n",result[ncol+j+1]);
		database_found = result[ncol+j+1];
		j++;
		//This is hit once per file in a play list.
		//printf("Database File %s\n",result[ncol+j+5]);
		//ID3Tag tmpTag;
		//tmpTag.artist = artist;
		//tmpTag.album = album;
		//tmpTag.title = title;
		//targetPlayListFile->setId3(Target_Entry,tmpTag);
	}
		
	if (ncol == 0)
	{
		printf("First NO DATABASE FOUND: %s\n",Target_Table.toStdString().c_str());
		return 0;
	}
	sqlite3_free_table(result);
	//printf("Trying to grab from %s and id %i and replace with %s\n",database_found.toStdString().c_str(),Target_Entry,title.toStdString().c_str());
	if (sqlite3_get_table(dataBase,QString(QString("select * from '").append(database_found).append("' where ROWID == ").append(QString::number(Target_Entry)).append(";")).toStdString().c_str(),&result,&nrow,&ncol,&zErrMsg) != SQLITE_OK)
	{
		QString err = "Error exec Sql 'select * from '";
		err.append(database_found).append("' where ROWID == ").append(QString::number(Target_Entry)).append("; %s \n");
		printf(err.toStdString().c_str(),zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	


	//for (int j=0;j<ncol*nrow;j++)
	//{
		//printf("Second Database hits: %s\n",result[ncol + j]);
	//}
	sqlite3_free_table(result);
	if (sqlite3_exec(dataBase,QString("delete from '").append(database_found).append("' where rowid == ").append(QString::number(Target_Entry)).toStdString().c_str(),0,0,&zErrMsg) != SQLITE_OK)
	{
		QString err = "Error exec Sql 'delete from '";
		err.append(database_found).append("' where ROWID == ").append(QString::number(Target_Entry)).append("; %s \n");
		printf(err.toStdString().c_str(),zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;	
	}
	
	QString req = QString("insert into '").append(database_found).append("' (rowid,title,artist,album,filename,fullPath,tracknum,lastupdate) values (").append(QString::number(Target_Entry)).append(",'").append(title).append("','").append(artist).append("','").append(album).append("','").append(filename).append("','").append(fullPath).append("',").append(QString::number(tracknum)).append(",").append(QString::number(QDateTime::currentDateTime().toTime_t())).append(");");
	//printf("Target Entry %s\n",req.toStdString().c_str());
	if (sqlite3_exec(dataBase,req.toStdString().c_str(),0,0,&zErrMsg) != SQLITE_OK)
	{
		QString err = "Error exec Sql: ";
		err.append(req).append(":::: %s\n");
		printf(err.toStdString().c_str(),zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	
	return 0;
}