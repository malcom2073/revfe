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
import "Map.js" as Map
Window {
	name: "settingspage"
	id: settingspage
	x: 0
	y: 0
	width: 924
	height: 600
	property list<Item> settingsPages
	function addSettingsPage(name) {
		var component = Qt.createComponent("genericsettings.qml")
		var object = component.createObject(settingspage);
		if (object == null)
		{
			console.log("Error loading generic settings: " + component.errorString());

		}
		object.x = 100
		object.y = 0
		Map.settingsList[name] = object;
		if (settingsPageListModel.count > 0)
		{
			object.visible = false
		}
		object.settingName = name
		settingsPageListModel.append({"name":name,"object":object})
	}
	ListModel {
		id: settingsPageListModel
	}
	function saveSettings(name,key,value,type,id) {
		//At this point, the settings are saved on the settings screen, we just need to save them in the xml file
		console.log("Saving settings for plugin:" + name + " - " + key + ":" + value + "::" + id)
		parent.parent.parent.saveSettings(name,key,value,type,id)
	}
	function addSettingsValue(page,setting,value,type,id) {
		for(var i=0;i<settingsPageListModel.count;i++)
		{
			if (settingsPageListModel.get(i).name == page)
			{
				settingsPageListModel.get(i).object.addSetting(setting,value,type,id);
			}
		}
	}
	//ListModel {
	//	id: testModel
	//	ListItem
	//}
	property int currentPage: 0;
	Button {
		x:0
		y:500
		width:100
		height:100
		text: "Prev"
		onClicked: {
			settingsPageListModel.get(currentPage).object.visible = false
			if (currentPage == 0)
			{
				currentPage = settingsPageListModel.count-1;
			}
			else
			{
				currentPage--;
			}
			settingsPageListModel.get(currentPage).object.visible = true
			settingsPageListModel.get(currentPage).object.refresh()
		}
	}
	Button {
		x:600
		y:500
		width:100
		height:100
		text: "Next"
		onClicked: {
			settingsPageListModel.get(currentPage).object.visible = false
			if (currentPage == settingsPageListModel.count-1)
			{
				currentPage = 0;
			}
			else
			{
				currentPage++;
			}
			settingsPageListModel.get(currentPage).object.visible = true
			settingsPageListModel.get(currentPage).object.refresh()
		}
	}

}

