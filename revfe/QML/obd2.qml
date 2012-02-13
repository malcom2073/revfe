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
import GaugeItem 0.1

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
	Component.onCompleted: {
		window.passMessage("RevOBD2:obd add !010C!");
		window.passMessage("RevOBD2:obd add !010D!");
		window.passMessage("RevOBD2:obd add !0105!");
		window.passMessage("RevOBD2:obd add !0110!");
	}

	GaugeItem {
		id: speedGauge
		x: 0
		y: 275
		m_style: 3
		width:350
		height:350
		startAngle:45
		endAngle:315
		minimum: 0
		maximum: 140
		numLabels: 7
		Behavior on m_value {  PropertyAnimation { properties: "m_value"; duration: (propertyMap["OBD_010D_DURATION"] ? propertyMap["OBD_010D_DURATION"] : 500) } }
		m_value: (propertyMap["OBD_010D"] ? propertyMap["OBD_010D"] : 0)
	}
	GaugeItem {
		id: rpmGauge
		m_style: 3
		x:350
		y:275
		width:350
		height:350
		minimum: 0
		numLabels: 7
		startAngle: 45
		endAngle: 315
		maximum: 7000
		Behavior on m_value {  PropertyAnimation { properties: "m_value"; duration: (propertyMap["OBD_010C_DURATION"] ? propertyMap["OBD_010C_DURATION"] : 500) } }
		m_value: (propertyMap["OBD_010C"] ? propertyMap["OBD_010C"] : 0)
	}
	GaugeItem {
		id: mafGauge
		x: 375
		y: 60
		width:200
		height:200
		m_style: 3
		startAngle:360
		endAngle:180
		numLabels: 3
		minimum: 0
		maximum: 600
		Behavior on m_value {  PropertyAnimation { properties: "m_value"; duration: (propertyMap["OBD_010D_DURATION"] ? propertyMap["OBD_010D_DURATION"] : 500) } }
		m_value: (propertyMap["OBD_0110"] ? propertyMap["OBD_0110"] : 0)
	}
	GaugeItem {
		id: tempGauge
		x: 325
		y: 60
		m_style: 3
		width:200
		height:200
		startAngle:0
		endAngle:180
		numLabels: 5
		minimum: 0
		maximum: 200
		Behavior on m_value {  PropertyAnimation { properties: "m_value"; duration: (propertyMap["OBD_0105_DURATION"] ? propertyMap["OBD_0105_DURATION"] : 500) } }
		m_value: (propertyMap["OBD_0105"] ? propertyMap["OBD_0105"] : 0)
	}
}

