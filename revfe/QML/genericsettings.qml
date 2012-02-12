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
//import "components"
import "Button"
import "CheckBox"
import "Map.js" as Map
Window {
	name: "genericsettingspage"
	id: genericsettingspage
	x: 0
	y: 0
	width: 924
	height: 500

	property variant settingsList: ListModel {}
	property variant savedSettingsList: ListModel {}
	function addSetting(name,value,type,id) {
		console.log("Adding setting to " + settingName + " - " + name + ":" + value + "::" + id + ":::" + type)
		settingsList.append({ "setting":name, "value":value,"type":type,"id":id})
		savedSettingsList.append({ "setting":name, "value":value,"type":type,"id":id})
	}
	Button {
		x:0
		y:500
		width:100
		height:100
		text: "Save"
		onClicked: {
			savedSettingsList.clear();
			for (var i=0;i<settingsList.count;i++)
			{
				parent.parent.saveSettings(settingName,settingsList.get(i).setting,settingsList.get(i).value,settingsList.get(i).type,settingsList.get(i).id)
				savedSettingsList.append({"setting":settingsList.get(i).setting,"value":settingsList.get(i).value,"type":settingsList.get(i).type,"id":settingsList.get(i).id})
			}
			//savedSettingsList = settingsList;
		}
	}
	property string settingName

	Component {
		id: testDelegate
		Item
		{
			height:60
			Loader {
				sourceComponent: id != 0 && type == "boo" ? boolMultiDelegate : id != 0 ? textMultiDelegate : type == "bool" ? boolDelegate : type == "int" ? textDelegate : textDelegate
			}
			Component {
				id: boolDelegate
				Item {
					width: 400; height: 60
					Text {
						x:0
						y:0
						width:200
						height:60
						font.pixelSize: 20;
						color: "white";
						text: setting
					}
					CheckBox {
						x:170
						y:0
						width:50
						height:50
						checked: value == "true" ? true : false
						onCheckedChanged: {
							genericsettingspage.settingsList.set(index,{ "setting":setting,"value":checked == true ? "true" : "false","type":type})
						}
						text:value
					}
				}
			}
			Component {
				id: textDelegate
				Item {
					width: 400; height: 60
					Text {
						x:0
						y:0
						width:200
						height:60
						font.pixelSize: 20;
						color: "white";
						text: setting
					}
					TextInput {
						x:170
						y:0
						width:200
						height:60
						text:value
						onTextChanged: {
							console.log(text);
							genericsettingspage.settingsList.set(index,{ "setting":setting, "value":text,"type":type})
							//value = text
						}
					}
				}

			}

			Component {
				id: textMultiDelegate
				Item {
					width: 400; height: 60
					Text {
						x:0
						y:0
						width:170
						height:60
						font.pixelSize: 20;
						color: "white";
						text: setting
					}
					TextInput {
						x:170
						y:0
						width:170
						height:60
						text:value
						onTextChanged: {
							//console.log(text);
							genericsettingspage.settingsList.set(index,{ "setting":setting, "value":text,"type":type})
							//value = text
						}
					}
					Button {
						x:340
						y:0
						width:60
						height:60
						text: "Add"
						onClicked: {
							genericsettingspage.settingsList.insert(index,{"setting":settingsList.get(index).setting,"value":"","type":settingsList.get(index).type,"id":settingsList.get(index).id + 1})
							genericsettingspage.savedSettingsList.insert(index,{"setting":settingsList.get(index).setting,"value":"","type":settingsList.get(index).type,"id":settingsList.get(index).id + 1})
						}
					}
				}

			}
		}
	}
	function refresh() {
		testview.model = 0
		testview.model = savedSettingsList
	}
	Text {
		x:0
		y:0
		width:300
		height:50
		color: "white"
		font.pixelSize: 30
		text: settingName
	}
	Rectangle {
		x:0
		y:50
		width:500
		height:400
		radius:20
		color: "orange"
	        Rectangle {
			x:5
			y:5
			width:490
			height: 390
			color: "grey"
			radius: 20
		}
	}
	ListView {
		id: testview
		x: 50
		y: 75
		width: 400
		clip: true
		height: 350
		highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
		//focus: true
		model: savedSettingsList
		delegate: testDelegate
	}

}

