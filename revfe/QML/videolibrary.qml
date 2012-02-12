/***************************************************************************
*   Copyright (C) 2011 by Michael Carpenter (malcom2073)                  *
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

import Qt 4.7
//import QtQuick 1.0
import "Window"
//import "components"
import "Button"


Window {
	name: "videolibrary"
	id:videolibrary
	x: 0
	y: 0
	width: 924
	height: 600
	property string currentDir;
	Component {
		id: videolistListDelegate
		Item {
			width: 400; height: 60
			MouseArea {
				anchors.fill: parent
				Text { font.pixelSize: 20; text: model.text }
				onClicked: {
					//window.playlistListClicked(modelData,index)
					//window.passMessage("MediaLibrary:playlist set !" + modelData +"!");
					//testview.currentIndex = index
					//medialibrary.parent.parent.parent.scrollLeft(1,medialibrary);
				}
			}
		}
	}
	Component.onCompleted: {
		console.log("Media Editor loaded");
		window.passMessage("RevFileBrowser:dir list !" + "/home/michael" + "! !VIDEO_EDIT_DIRLIST!")
		window.passMessage("RevFileBrowser:file list !" + "/home/michael" + "! !VIDEO_EDIT_FILELIST!")
		currentDir = "/home/michael"
		//window.passMessage("MediaLibrary:media
	}
	Component {
		id: fileListDelegate
		Item {
			width:290
			height:30
			MouseArea {
				anchors.fill: parent
				Text {
					color: "white"
					font.pixelSize: 20
					anchors.fill: parent
					text: model.text
				}
				onClicked: {
					//window.passMessage("RevMediaLibrary:playlisteditor add !" + currentDir + "/" + modelData + "!")
					//currentDir = currentDir + "/" + modelData
					//window.passMessage("FileBrowser:dir list !" + currentDir + "! !VIDEO_EDIT_DIRLIST!")
					//window.passMessage("FileBrowser:file list !" + currentDir + "! !VIDEO_EDIT_FILELIST!")
					//currentDir + "/" + modelData == file we want to play.
					videoplayer.setVideo(currentDir + "/" + modelData)
					//videolibrary.parent.setVideo(currentDir + "/" + modelData)
					//parent.parent.parent.parent.scrollLeft(1,videolibrary);
					videolibrary.parent.parent.parent.scrollLeft(1,videolibrary)
				}
			}
		}
	}
	Component {
		id: dirListDelegate
		Item {
			width:290
			height:30
			MouseArea {
				anchors.fill: parent
				Text {
					color: "white"
					font.pixelSize: 20
					anchors.fill: parent
					text: model.text
				}
				onClicked: {
					currentDir = currentDir + "/" + modelData
					window.passMessage("RevFileBrowser:dir list !" + currentDir + "! !VIDEO_EDIT_DIRLIST!")
					window.passMessage("RevFileBrowser:file list !" + currentDir + "! !VIDEO_EDIT_FILELIST!")
				}
			}
		}
	}


	Rectangle {
		x:200
		y:400
		radius: 30
		width:600
		height:200
		color: "grey"
		Rectangle {
			x: 5
			y:5
			radius: 25
			width: 590
			height: 190
			color: "#000000"
			ListView {
				x:5
				y:20
				width:580
				height:160
				clip:true
				delegate: fileListDelegate
				model: VIDEO_EDIT_FILELIST
			}
		}
	}
	Rectangle {
		x:0
		y:0
		radius: 30
		width:300
		height:400
		color: "grey"
		Rectangle {
			x: 5
			y:5
			radius: 25
			width: 290
			height: 390
			color: "#000000"
			ListView {
				x:5
				y:20
				width:280
				height:360
				clip:true
				delegate: dirListDelegate
				model: VIDEO_EDIT_DIRLIST
			}
		}
	}
	/*
	Rectangle {
		x:300
		y:0
		radius: 30
		width:500
		height:400
		color: "grey"
		Rectangle {
			x: 5
			y:5
			radius: 25
			width: 490
			height: 390
			color: "#000000"
			ListView {
				x:5
				y:20
				width:480
				height:360
				clip:true
				delegate: currentListDelegate
				model: MEDIA_EDIT_CURRENT_LIST
			}
		}
	}*/
	Button {
		x:0
		y:500
		width:100
		height:100
		text: "Return";
		onClicked: {
			parent.parent.parent.parent.scrollLeft(1,parent);
		}
	}
	Button {
		x:110
		y:500
		width:100
		height:100
		text: "Up Folder"
		onClicked: {
			var tmp = currentDir
			tmp = tmp.substring(0,tmp.lastIndexOf("/"))

			if (tmp.length == 0)
			{
				tmp = "/";

			}
			currentDir = tmp
			console.log(currentDir);
			window.passMessage("RevFileBrowser:dir list !" + currentDir + "! !VIDEO_EDIT_DIRLIST!")
			window.passMessage("RevFileBrowser:file list !" + currentDir + "! !VIDEO_EDIT_FILELIST!")
		}
	}


}

