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
import VideoPlayer 1.0

Window {
	name: "videoplayer"
	id:videoplayer
	x: 0
	y: 0
	width: 924
	height: 600

	VideoPlayer
	{
		id: vidwidget
		x:0
		y:0
		width:800
		height:500
	}

	//
	Button {
		x:0
		y:500
		width:100
		height:100
		text: "Load"
		onClicked: {
			vidwidget.load("/home/michael/test.flv")
		}
	}
	Button {
		x:100
		y:500
		width:100
		height:100
		text: "Play"
		onClicked: {
			vidwidget.play();
		}
	}
	function setVideo(video)
	{
		vidwidget.load(video);
	}

	Button {
		x:200
		y:500
		width:100
		height:100
		text: "Browse"
		onClicked: {
			var component = Qt.createComponent("videolibrary.qml")
			var object = component.createObject(parent.parent);
			if (object == null)
			{
				console.log("Error loading media editor!");
			}
			object.x = parent.x + 800;
			object.y = parent.y
			parent.parent.parent.parent.scrollRight(1)
			//item = object;
			//m_windowCount++;
		}
	}
}
