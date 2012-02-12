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

Window {
	name: "utilitypage"
	id: utilitypage
	x: 0
	y: 0
	width: 924
	height: 600
	/*ButtonPanel {
		id: utilityPanel
		x: 0
		y: 0
		width: 800
		height: 600
	}*/
	ListModel {
		id: testmodel
		ListElement {
			description: "This plugin allows RevFE to have access to localized fuel-price information. Fuel prices are avaialble either via zip code, or GPS coordinate, and returned as a table of information"
			app_name: "Fuel Prices"
			image: "test.png"
			product_id: 5221
		}
	}

	Component {
		id:myDelegate
		Item {
			width:800
			height:100
			MouseArea {
				anchors.fill: parent
				onClicked: {
					console.log(model.app_name)
					//proppage.app_description = model.description
					/*proppage.appname.text = model.app_name
					proppage.app_name = model.app_name
					proppage.app_image = model.image*/
					proppage.setText(model.app_name,model.description,model.image,model.product_id);
					proppage.x = 0
				}
			}

			Rectangle {
				anchors.fill: parent
				color: "grey"
				Image {
					anchors.left:  parent.left;
					anchors.top: parent.top;
					anchors.bottom: parent.bottom;
					width:100
					source: model.image
				}

				Text {
					anchors.right: parent.right
					anchors.top: parent.top;
					height: 20
					width: parent.width-100
					text: model.app_name + " " + model.version
				}
				Text {
					anchors.right:  parent.right;
					anchors.bottom:  parent.bottom
					width:  parent.width-100
					height:80
					text: model.description
				}
			}
		}
	}

	Text {
		x:0
		y:0
		width:800
		height:25
		font.pixelSize: 20
		color: "white"
		text: propertyMap.autoappmart_status
	}
	Text {
		x:0
		y:25
		width:800
		height:25
		font.pixelSize: 20
		color: "white"
		text: propertyMap.plugin_manager_install_status
	}

	ListView {
		x:0
		y:50
		width:800
		height:450
		model: autoappmartapps
		//model: testmodel
		delegate: myDelegate
		clip: true

	}
	Rectangle {
		id: proppage
		color: "grey"
		property string app_name: ""
		property string app_description: ""
		property string app_image: ""
		property string app_id: ""
		x:1200
		y:0
		width:800
		height:600
		function setText(name, desc, img,id)
		{
			appname.text = name;
			appdesc.text = desc;
			appimg.source = img;
			app_id = id
		}

		Text {
			id: appname
			x:0
			y:0
			width:200
			height:30
			font.pixelSize: 30
			wrapMode: Text.WordWrap
		}
		Text {
			id: appdesc
			x:0
			y:35
			width:400
			height:400
			font.pixelSize: 25
			wrapMode: Text.WordWrap
		}
		Image {
			id: appimg
			x:400
			y:0
			width:400
			height:400
		}
		Button {
			x:600
			y:500
			width:100
			height:100
			text: "Return"
			onClicked: {
				proppage.x = 1200
			}
		}
		Button {
			x:500
			y:500
			width:100
			height: 100
			text: "Download"
			onClicked: {
				//We should download the file here.
				window.passMessage("RevAutoAppMart:api download !" + parent.app_id + "!")
			}
		}
	}

	Button {
		x:0
		y:500
		width:100
		height:100
		text: "Clicky!"
		onClicked: {
			window.passMessage("RevAutoAppMart:api get-category !6,22!") //6 is plugins, 22 is RevFE
			//window.passMessage("RevAutoAppMart:api get-category-list");
			//window.passMessage("RevPluginManager:plugin install !/home/michael/TBR/RevHDRadio.zip!");
		}
	}
}

