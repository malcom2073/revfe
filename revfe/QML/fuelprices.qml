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
import "TaskBar"
import "ButtonPanel"
import "Window"
//import "components"
import "Button"
Window {
	name: "fuelprices";
	id: fuelprices
	width: 800
	height: 600
	ListModel {
		id: testmodel
		ListElement {
			Station_Name: "Exxon"
			Station_Address: "1234 Tech Ct. Westminster, MD 21157"
			Regular: "$3.459"
			Midgrade: "$3.599"
			Premium: "$3.999"
		}
		ListElement {
			Station_Name: "Wawa"
			Station_Address: "1234 Baltimore blvd. Westminster, MD 21157"
			Regular: "$3.459"
			Midgrade: "$3.599"
			Premium: "$3.999"
		}
	}
	Rectangle {
		id: root
		x: 0
		y: 0
		width: 800
		height: 600
		//color: "red"
		Button {
			x:0
			y:400
			width:100
			height:100
			text: "Click!";
			onClicked: {
				window.passMessage("RevFuelPrices:fuel get-zip !21157!");
			}
		}
		Component {
			id: delegateview
			Item {
				width: 75
				height:30
				Text {
					color: "black"
					font.pixelSize: 20
					anchors.fill: parent
					text: ".." + model.Station_Name + ((model.Station_Name.length > 8) ? "\t" : "\t\t") + model.Regular + "\t\t" + model.Midgrade + "\t\t" + model.Premium
				}
			}
		}
		Rectangle {
			x:0
			y:0
			width:500
			height:400
			color:"orange"
			ListView {
				id: nameview
				x:0
				y:0
				width:500
				height:400
				delegate: delegateview
				model: FuelPricesList
				//model: testmodel
			}
		}

	}
	
}
