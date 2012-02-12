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
	name: "exitmenu"
	id: exitmenu
	x: 0
	y: 0
	width: 924
	height: 600

	Rectangle {
		x:50
		y:100
		radius: 30
		width:500
		height:450
		color: "grey"
		Rectangle {
			x: 5
			y:5
			radius: 25
			width: 490
			height: 440
			color: "#000000"
			Button {
				x:100
				y:350
				width:300
				height:75
				text: "Exit"
				onClicked: {
					window.exitClicked()
				}
			}
			Button {
				x:100
				y:200
				height:75
				width:300
				text: "Test"
				//onClicked: {
					//Nothing
				//}
			}
		}
	}
}

