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
	name: "gpsstatus"
	id:gpsstatus
	x: 0
	y: 0
	width: 924
	height: 600

	Component {
		id: satlistdelegate
		Item {
			width: 300; height: 100
			Rectangle {
				anchors.fill: parent
				color: "white"
				radius: 15
				Rectangle {
					x:5
					y:5
					width: parent.width-10
					height: parent.height-10
					radius: 15
					color: "black"
						Text { x:10; y:10; color: "white"; font.pixelSize: 20; text: "PRN: " + model.SAT_PRN + "   \t" + "Elevation: " + model.SAT_ELEVATION + "\n" + "Azimuth: " + model.SAT_AZIMUTH + "\t" + "SNR: " + model.SAT_SNR }
				}
			}
		}
	}


	Rectangle {
		x:0
		y:0
		width:200
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: "Latitude: " + propertyMap.GPS_LATITUDE
		}
	}

	Rectangle {
		x:0
		y:30
		width:200
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: "Longitude: " + propertyMap.GPS_LONGITUDE
		}
	}
	Rectangle {
		x:0
		y:60
		width:200
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: "Fix Type: " + propertyMap.GPS_FIXTYPESTRING
		}
	}
	Rectangle {
		x:0
		y:90
		width:200
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: "Status: " + propertyMap.GPS_STATUS_STRING
		}
	}

	ListView {
		id: testview
		x: 400
		y: 0
		width: 300
		clip: true
		height: 600
		highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
		//focus: true
		model: GPS_SAT_INFO
		delegate: satlistdelegate
	}
}

