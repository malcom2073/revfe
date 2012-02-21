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
//import "components"
import "Button"
import "TextBox"
import "Style"

Window {
	name: "mediaplayer"
	id: mediaplayer
	x: 0
	y: 0
	width: 924
	height: 600
	Style { id: style }


	TextBox {
		x:50
		y:25
		width:400
		height:50
		text: propertyMap.currentSongText
	}
	TextBox {
		id: timeElapsed
		x:500
		y:25
		width:50
		height:50
		text: propertyMap.elapsedTimeText
	}
	TextBox {
		x:550
		y:25
		width:50
		height:50
		text: propertyMap.totalTimeText
	}
	Rectangle {
		x:100
		y:50
		width:500
		height:400
		radius:style.listBorderRadius
		color: "orange"
	        Rectangle {
			x:5
			y:5
			width:490
			height: 390
			gradient: Gradient {
				GradientStop { position: 0.0; color: style.listBgTopColor }
				GradientStop { position: 1.0; color: style.listBgBottomColor }
			}
			radius: style.listBorderRadius
		}
	}
	property int volSliderDummyValue: volSlider.value //Stupid hack to make onValueChanged event work
	/*Slider {
		id: volSlider
		x: 600
		y: 50
		width:50
		height:400
		minimumValue: 0
		maximumValue: 100
		orientation: Qt.Vertical
		value: propertyMap.sys_volume
		updateValueWhileDragging: false
		onValueChanged:
		{
			window.passMessage("MediaEngine:media setvolume !" + value + "!");
			//console.log("Value Changed");
		}
	}*/
	Button {
		//Prev button
		x: 50
		y:500
		width:100
		height:100
		text: "Prev"
		onClicked:
		{
			window.passMessage("RevMediaEngine:media prev");
		}
	}
	Button {
		//Play button
		x: 150
		y:500
		width:100
		height:100
		text: "Play"
		onClicked:
		{
			window.passMessage("RevMediaEngine:media play");
		}
	}
	Button {
		//Pause button
		x: 250
		y:500
		width:100
		height:100
		text: "Pause"
		onClicked:
		{
			window.passMessage("RevMediaEngine:media pause");
		}
	}
	Button {
		//Stop button
		x: 350
		y:500
		width:100
		height:100
		text: "Stop"
		onClicked:
		{
			window.passMessage("RevMediaEngine:media stop");
		}
	}
	Button {
		//Next button
		x: 450
		y:500
		width:100
		height:100
		text: "Next"
		onClicked:
		{
			window.passMessage("RevMediaEngine:media next");
		}
	}
	property Item item;
	Button {
		//Media button
		x: 550
		y:500
		width:100
		height:100
		text: "Media"
		onClicked:
		{
			//window.passMessage("MediaEngine:media next");
			var component = Qt.createComponent("media.qml")
			var object = component.createObject(parent.parent);
			if (object == null)
			{
				console.log("Error loading app prices!");
			}
			object.x = parent.x + 700;
			object.y = parent.y
			parent.parent.parent.parent.scrollRight(1)
			item = object;
			//m_windowCount++;
		}
	}

	Component {
		id: currentPlaylistDelegate
		Item {
			/*opacity: PathView.iconOpacity*/
			width: 400; height: 60
			//color: "blue"
			//border.color: "black"
			//border.width: 1
			//radius: 15
			MouseArea {
				anchors.fill: parent
				Text {
					anchors.fill: parent
					text: model.artist + " - " + model.title
					color: "black"
					verticalAlignment: Text.AlignVCenter
					horizontalAlignment: Text.AlignLeft
					font.pixelSize: 20 
				}
				onClicked: {
					window.passMessage("RevMediaEngine:media play !" + index +"!");


					//window.currentPlaylistClicked(modelData,index)
					/*if (index-2 < 0 )
						currentPlaylistView.currentIndex = currentPlaylistView.count+(index-2)
					else
						currentPlaylistView.currentIndex = index-2;*/
				}
			}
		}
	}
	//Enable this block and remove the ListView directly below it for a PathView version
	//of the playlist. I never got it looking right, so I've just relegated it to being
	//a permanent comment should I ever want to come back to it.
	/*
	Component {
		id: currentPlaylistViewHighlight
		Rectangle { 
			visible: PathView.onPath
			opacity: PathView.iconOpacity
			width: 400
			height: 60
			//color: "red"
			border.color: "white"
			border.width: 1
			radius: 30
		}
	}
	
	Timer {
		id: currentPlaylistIndexTimer
		interval: 1000
		repeat: false
		running: false 
		onTriggered: currentPlaylistView.offset = mediaplayer.str
	}
	property real str: 1.0
	PathView {
		id: currentPlaylistView
		x: 50
		y: 100
		dragMargin: 200
		width:500
		height:400
		clip:true
		preferredHighlightBegin: 0.5
		preferredHighlightEnd: 0.5
		highlightRangeMode: PathView.ApplyRange
		currentIndex: window.currentSongIndex
		highlight: currentPlaylistViewHighlight
		model: currentPlaylistModel
		pathItemCount: 5
		delegate: currentPlaylistDelegate
		path: Path {
			startX: 200
			startY: 30
			PathAttribute { name: "iconOpacity"; value: 0.5 }
			PathQuad { x: 300; y: 200; controlX: 300; controlY: 100 }
			PathAttribute { name: "iconOpacity"; value: 1.0 }
			PathQuad { x: 200; y: 370; controlX: 300; controlY: 300 }
			//PathQuad { x: 120; y: 100; controlX: -20; controlY: 75 }

		}
		onMovementStarted: {
			//mediaplayer.str = currentPlaylistView.offset 
			//currentPlaylistIndexTimer.running = false;
		}
		onMovementEnded: {
			//currentPlaylistIndexTimer.running = true
		}
	}*/
	
	//This is the ListView to remove should you choose to use the PathView in the 
	//comment block above
	ListView {
		id: currentPlaylistView
		x: 150
		y: 75
		width: 400
		height: 350
		clip: true
		currentIndex: propertyMap.currentSongIndex
		highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
		//focus: true
		model: currentPlaylistModel
		delegate: currentPlaylistDelegate
	}

}


