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

	GaugeItem {
		id: speedGauge
		x: 0
		y: 0
		m_style: 3
		width:300
		height:300
		startAngle:0
		endAngle:180
		minimum: 10
		maximum: 110
		m_value: propertyMap.OBD_010D
	}
	GaugeItem {
		//id: speedGauge
		x: 0
		y: 300
		width:300
		height:300
		m_style: 3
		startAngle:180
		endAngle:360
		minimum: 10
		maximum: 110
		m_value: propertyMap.OBD_010D
	}
	GaugeItem {
		id: rpmGauge
		m_style: 3
		x:300
		y:0
		width:300
		height:300
		minimum: 1000
		startAngle: -45
		endAngle: 45
		maximum: 7000
		m_value: propertyMap.OBD_010C
	}
	GaugeItem {
		//id: speedGauge
		x: 300
		y: 300
		m_style: 3
		width:300
		height:300
		startAngle:90
		endAngle:270
		minimum: 10
		maximum: 110
		m_value: propertyMap.OBD_010D
	}
	Component.onCompleted:
	{
		//speedGauge.value = 0;
	}
}

