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
	name: "medialbirary"
	id:medialibrary
	x: 0
	y: 0
	width: 924
	height: 600
	Text {
		x:100
		y:0
		width:100
		height:50
		font.pixelSize: 20
		color: "white"
		text: { propertyMap.Media_Scanner_Progress }
	}
	Rectangle {
		x:100
		y:50
		width:500
		height:400
		radius:20
		color: "orange"
	        Rectangle {
			x:5
			y:5
			width:490
			height: 390
			color: "grey"
			radius: 20
		}
	}

	Component {
		id: playlistListDelegate
		Item {
			width: 400; height: 60
			MouseArea {
				anchors.fill: parent
				Text { font.pixelSize: 20; text: model.text }
				onClicked: {
					//window.playlistListClicked(modelData,index)
					window.passMessage("RevMediaLibrary:playlist set !" + model.text +"!");
					testview.currentIndex = index
					medialibrary.parent.parent.parent.scrollLeft(1,medialibrary);
				}
			}
		}
	}

	ListView {
		id: testview
		x: 150
		y: 75
		width: 400
		clip: true
		height: 350
		currentIndex: propertyMap.currentPlaylistIndex
		highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
		//focus: true
		model: playlistListModel
		delegate: playlistListDelegate
	}


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
		x:200
		y:500
		width:100
		height:100
		text: "Edit Playlists"
		onClicked:
		{
			//window.passMessage("MediaEngine:media next");
			var component = Qt.createComponent("mediaedit.qml")
			var object = component.createObject(parent.parent);
			if (object == null)
			{
				console.log("Error loading media editor!");
			}
			object.x = parent.x + 800;
			object.y = parent.y
			parent.parent.parent.parent.scrollRight(2)
			item = object;
			//m_windowCount++;
		}
	}
}

