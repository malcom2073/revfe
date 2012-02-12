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
import "Gauge"


Window {
	name: "obdpage"
	id: obdpage
	x: 0;
	y: 0;
	width: 924
	height: 600
	Rectangle {
		x:0
		y:0
		width:300
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: propertyMap.OBD_STATUS_TEXT
		}
	}

	Rectangle {
		x:0
		y:30
		width:300
		height:30
		color:"grey"
		Text {
			color: "white"
			font.pixelSize: 20
			text: propertyMap.OBD_TOOL_VERSION
		}
	}

	Gauge {
		id: speedGauge
		source: "gauge.png"
		x: 100
		y: 100
		width:300
		height:300
		minAngle:-150
		maxAngle:150
		minValue: 10
		maxValue: 110
		value: propertyMap.OBD_010D
	}
	Gauge {
		id: rpmGauge
		source: "rpmgauge.png"
		x:400
		y:100
		width:300
		height:300
		minValue: 1000
		minAngle: -135
		maxAngle: 135
		maxValue: 7000
		value: propertyMap.OBD_010C
	}
	Component.onCompleted:
	{
		//speedGauge.value = 0;
	}
}

