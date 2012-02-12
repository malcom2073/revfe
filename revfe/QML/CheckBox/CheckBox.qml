import Qt 4.7

Rectangle {
	signal textChanged();
	signal checkedChanged();
        property string text: "";
	property bool checked: false;
        MouseArea {
                anchors.fill: parent
		/*Text {
			//anchors.fill: parent
			x:0
			y:0
			width:parent.width - 50
			height:parent.height
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
			text: text
		}*/
		Rectangle {
			x:0
			y:0
			width:50
			height:parent.height
			color: "white"
			Rectangle {
				x:10
				y:10
				radius:10
				width: 30
				height:parent.height-30
				color: ((checked) ? "blue" : "white");
			}
		}

                onClicked: {
			if (checked)
			{
				checked = false;
			}
			else
			{
				checked = true;
			}
			checkedChanged()
			//parent.clicked();
                }
        }


}
