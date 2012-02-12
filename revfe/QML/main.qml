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
import "Map.js" as Map

Rectangle {
	//property int sys_volume
	id: desktop
	width: 800
	height: 600
	//source: "plbg.jpg"
	color: "black"
	state: "MediaState"
	property variant apps;
	property int currentState;
	property Window currentWindow;

	TaskBar {
		id: taskBar
		x: 0
		y: 0
		z: 100
		width: 100
		height: 600
		appModel: propertyMap.totalapps
		onClicked: {
			currentWindow = Map.windowList[state];
			canvasMoveAnimation.target = appWrapperYPos;
			canvasMoveAnimation.from = -600 * currentState;
			canvasMoveAnimation.to = -600 * (state);
			canvasMoveAnimation.running = true;
			currentState = state;
		}
	}

	property int m_windowCount: 0;
	Item {
		id: appWrapperScale
		x:0
		y:0
		width: 5000
		height: 600
		Item {
			id: appWrapperYPos
			x:0
			y:0
			width: 5000
			height: 600

		}
	}
	function addSettingsPage(name) {
		settingsPage.addSettingsPage(name)
	}
	function addSettingsValue(page,setting,value,type,id) {
		settingsPage.addSettingsValue(page,setting,value,type,id)
	}
	property Item settingsPage;
	function loadComplete() {
		console.log("main.qml component loaded. Loading extra qml's");
		//addonfiles
		for (var i=0;i<propertyMap.topfiles.length;i++)
		{
			console.log(propertyMap.topfiles[i]);
			var component = Qt.createComponent(propertyMap.topfiles[i])
			var object = component.createObject(appWrapperYPos);
			if (object === null)
			{
				console.log("Error loading component: " + propertyMap.topfiles[i] + component.errorString());
				continue //This lets everything else continue to load, despite one page not loading
			}
			if (object.name == "settingspage")
			{
				settingsPage = object
			}
			object.m_windowID = i;
			object.x = 100;
			object.y = 600 * i + 1;
			Map.windowList[i] = object;
			m_windowCount++;
		}
		for (var i=0;i<propertyMap.addonfiles.length;i++)
		{
			console.log(propertyMap.addonfiles[i]);
			var component = Qt.createComponent(propertyMap.addonfiles[i])
			var object = component.createObject(appWrapperYPos);
			if (object === null)
			{
				console.log("Error loading component: " + propertyMap.addonfiles[i] + component.errorString());
				continue //This lets everything else continue to load, despite one page not loading
			}
			if (object.name == "settingspage")
			{
				settingsPage = object
			}
			object.m_windowID = i;
			object.x = 100;
			object.y = 600 * (i + propertyMap.topfiles.length) + 1;
			Map.windowList[propertyMap.topfiles.length + i] = object;
			m_windowCount++;
		}
		for (var i=0;i<propertyMap.bottomfiles.length;i++)
		{
			console.log(propertyMap.bottomfiles[i]);
			var component = Qt.createComponent(propertyMap.bottomfiles[i])
			var object = component.createObject(appWrapperYPos);
			if (object === null)
			{
				console.log("Error loading component: " + propertyMap.bottomfiles[i] + component.errorString());
				continue //This lets everything else continue to load, despite one page not loading
			}
			if (object.name == "settingspage")
			{
				settingsPage = object
			}
			object.m_windowID = i;
			object.x = 100;
			object.y = 600 * (i + propertyMap.topfiles.length + propertyMap.addonfiles.length) + 1;
			Map.windowList[i + propertyMap.topfiles.length + propertyMap.addonfiles.length] = object;
			m_windowCount++;
		}

	}

	Component.onCompleted: {

	}
	function saveSettings(name,key,value,type,id) {
		window.saveSetting(name,key,value,type,id)
	}
	function scrollLeft(num,sender) {
		canvasLeftAnimation.target = appWrapperYPos
		canvasLeftAnimation.from = (-800) * num;
		canvasLeftAnimation.to = (-800) * (num - 1);
		canvasLeftAnimation.running = true;
		if (num == 1)
		{
			taskBar.visible = true;
		}
		sender.destroy()
	}
	function scrollRight(num) {
		canvasLeftAnimation.target = appWrapperYPos
		canvasLeftAnimation.from = (-800) * (num-1);
		canvasLeftAnimation.to = (-800) * num
		canvasLeftAnimation.running = true;
		//currentWindow.visible = false;
		taskBar.visible = false;
	}
	PropertyAnimation {
		id: canvasLeftAnimation
		properties: "x"
		duration: 250
	}
	PropertyAnimation {
		id: canvasMoveAnimation
		properties: "y"
		duration: 250
	}

}
