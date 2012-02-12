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
import "Button"
//import "components"



Window {
	name: "medaedit"
	id: mediaeditmenu
	x: 0
	y: 0
	width: 924
	height: 600


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
					text: modelData
				}
				onClicked: {
					currentDir = currentDir + "/" + modelData
					window.passMessage("RevFileBrowser:dir list !" + currentDir + "! !MEDIA_EDIT_DIRLIST!")
					window.passMessage("RevFileBrowser:file list !" + currentDir + "! !MEDIA_EDIT_FILELIST!")
				}
			}
		}
	}
	Component {
		id: currentListDelegate
		Item {
			width:290
			height:30
			MouseArea {
				anchors.fill: parent
				Text {
					color: "white"
					font.pixelSize: 20
					anchors.fill: parent
					text: "File:" + modelData
				}
				onClicked: {
					currentDir = currentDir + "/" + modelData
					window.passMessage("RevFileBrowser:dir list !" + currentDir + "! !MEDIA_EDIT_DIRLIST!")
					window.passMessage("RevFileBrowser:file list !" + currentDir + "! !MEDIA_EDIT_FILELIST!")
				}
			}
		}
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
					text: modelData
				}
				onClicked: {
					window.passMessage("RevMediaLibrary:playlisteditor add !" + currentDir + "/" + modelData + "!")
					//currentDir = currentDir + "/" + modelData
					//window.passMessage("FileBrowser:dir list !" + currentDir + "! !MEDIA_EDIT_DIRLIST!")
					//window.passMessage("FileBrowser:file list !" + currentDir + "! !MEDIA_EDIT_FILELIST!")
				}
			}
		}
	}

	property string currentDir: ""
	//Media List, by folder
	Component.onCompleted: {
		console.log("Media Editor loaded");
		window.passMessage("RevFileBrowser:dir list !" + "."/*propertyMap.MEDIA_DIRS*/ + "! !MEDIA_EDIT_DIRLIST!")
		window.passMessage("RevFileBrowser:file list !" + "." /*propertyMap.MEDIA_DIRS*/ + "! !MEDIA_EDIT_FILELIST!")
		currentDir = "."//propertyMap.MEDIA_DIRS
		//window.passMessage("MediaLibrary:media 
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
				model: MEDIA_EDIT_DIRLIST
			}
		}
	}
	//Current playlist that we're editing
	//This isnt actually the "current" playlist
	//Need to figure out how to enable editing of the current playlist. That oughta be a pain in the arse.
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
	}
	//Files in the directory selected
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
				model: MEDIA_EDIT_FILELIST
			}
		}
	}
	Button {
		x:10
		y:500
		width:100
		height:100
		text: "Done"
		onClicked: {
			parent.parent.parent.parent.scrollLeft(2,parent);
		}
	}
	Button {
		x:110
		y:500
		width:100
		height:100
		text: "Up Folder"
		onClicked: {
			var tmp = currentDir;
			tmp = tmp.substring(0,tmp.lastIndexOf("/"))
			currentDir = tmp
			window.passMessage("RevFileBrowser:dir list !" + currentDir + "! !MEDIA_EDIT_DIRLIST!")
			window.passMessage("RevFileBrowser:file list !" + currentDir + "! !MEDIA_EDIT_FILELIST!")
		}
	}

}

