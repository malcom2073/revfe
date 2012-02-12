import Qt 4.7
import "../Style"
Rectangle {
	Style { id: style }
	signal clicked();
	property string text: "";
	color: style.buttonOutlineColor
	radius: 10
	Rectangle {
		x: 5
		y: 5
		width:parent.width-10
		height:parent.height-10
		radius: style.buttonBorderRadius
		color: style.buttonBgColor
		MouseArea {
			anchors.fill: parent
			Text {
				anchors.fill: parent
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
				color: "White"
				font.pixelSize: style.buttonTextSize
				font.family: style.buttonFontName
				text: parent.parent.parent.text
			}
			onClicked: {
				parent.parent.clicked();
			}
		}

	}
}
