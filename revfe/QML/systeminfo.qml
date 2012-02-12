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
import "ButtonPanel"
import "Button"
import "TextBox"

Window {
	name: "utilitypage"
	id: utilitypage
	x: 0
	y: 0
	width: 924
	height: 600
	TextBox {
		x:50
		y:0
		width:50
		height:30
		text: "Battery: " + propertyMap.SYSINFO_BATT
	}

	TextBox {
		x:50
		y:30
		width:200
		height:30
		text: "SysInfo Msg: " + propertyMap.SYSINFO_MFG
	}

	TextBox {
		x:50
		y:60
		width:200
		height:30
		text: "SysInfo Model: " + propertyMap.SYSINFO_MODEL
	}
	TextBox {
		x:50
		y:90
		width:100
		height:30
		text: "SysInfo Product Name: " + propertyMap.SYSINFO_PROD_NAME
	}
	TextBox {
		x:50
		y:200
		width:100
		height:50
		text: "SysInfo Firmware: " + propertyMap.SYSINFO_VERSION_FIRMWARE
	}
	TextBox {
		x:50
		y:250
		width:100
		height:50
		text: "SysInfo DiskFree: " + propertyMap.SYSINFO_DRIVE_FREE
	}
	TextBox {
		x:50
		y:300
		width:100
		height:30
		text: "SysInfo WLan MAC: " + propertyMap.SYSINFO_WLAN_MAC
	}
	TextBox {
		x:50
		y:330
		width:100
		height:30
		text: "SysInfo WLan IP: " + propertyMap.SYSINFO_WLAN_IPV4
	}





}

