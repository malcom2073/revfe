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
	name: "hdradiomenu"
	id: hdradiomenu
	x: 0
	y: 0
	width: 924
	height: 600

	Rectangle {
		x:50
		y:100
		radius: 30
		width:600
		height:450
		color: "grey"
		Rectangle {
			x: 5
			y:5
			radius: 25
			width: 590
			height: 440
			color: "#000000"


			//This is the preset button on the bottom
			Rectangle {
				x:10
				y:340
				radius: 30
				width:570
				height:90
				color: "grey"
				Rectangle {
					x:5
					y:5
					radius: 25
					width: 560
					height: 80
					color: "#000000"
					Button {
						x:10
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET1
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !1!") 
						}
					}
					Button {
						x:90
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET2
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !2!") 
						}
					}
					Button {
						x:170
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET3
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !3!") 
						}
					}
					Button {
						x:250
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET4
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !4!") 
						}
					}
					Button {
						x:330
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET5
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !5!") 
						}
					}
					Button {
						x:410
						y:5
						width:70
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: propertyMap.RADIOPAL_PRESET6
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !tune! !6!") 
						}
					}
					Button {
						x:490
						y:5
						width:60
						height:70
						Text {
							anchors.fill: parent
							horizontalAlignment: Text.AlignHCenter
							verticalAlignment: Text.AlignVCenter
							text: "Set"
						}
						onClicked: {
							window.passMessage("RevHDRadio:radio preset !set!") 
						}
					}

				}
			}
		}
	}
}

