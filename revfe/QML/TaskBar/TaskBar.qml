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

Image
{
	id: taskbar
	source: "images/TaskBarBG.png"
	property alias appModel: appView.model
	property bool expanded: true
	state: "Expanded"

	Item
	{
		id: appMenuButton
		height: 50
		anchors.left: parent.left
		anchors.right: parent.right
		z: 1
		Image {
			anchors.centerIn: parent  
		}
	}

	Component
	{
		id: taskDelegate
		Item
		{
			id: delegate
			width: 100; height: 100
			Image {
				width: 48
				height: 48
				anchors.centerIn: parent
				//source: pixmap
			}

			Text
			{
				text: modelData
				color: "white"
				//font.family: "Tachoma"
				font.pixelSize: 10
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
				anchors.fill: parent
			}

			MouseArea
			{
				anchors.fill: parent;
				onClicked: { delegate.ListView.view.currentIndex = index; 
				delegate.ListView.view.clicked(index) }
			}
		}
	}

	Component
	{
		id: highlight
		Item
		{
			width: 100; height: 100
			Image {
				id: highlightRect
				width: 100; height: 100
				anchors.fill: parent
				anchors.topMargin: 10
				anchors.bottomMargin: -10
				source: "images/TaskBarBlip.png"
			}
		}
	}
	signal clicked( int state )
	ListView {
		id: appView
		signal clicked( int state )
		height: 500
		anchors.top: appMenuButton.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		delegate: taskDelegate
		highlight: highlight
		highlightMoveDuration: 250
		highlightMoveSpeed: -1
		highlightFollowsCurrentItem: true
		focus: true
		clip: true
		onClicked: { taskBar.clicked(appView.currentIndex) } //if(desktop.state != state) desktop.state = state }
	}

}
