import Qt 4.7
import "../Style"
Rectangle {
	Style { id: style }
	signal clicked();
	property string text: "";
	color: style.buttonOutlineColor
	radius: style.buttonBorderRadius
	Rectangle {
		x: 1
		y: 1
		width:parent.width-3
		height:parent.height-3
		radius: style.buttonBorderRadius
		color: style.buttonBgColor
		gradient: Gradient {
			GradientStop { position: 0.0; color: style.buttonBgTopColor }
			GradientStop { position: 1.0; color: style.buttonBgBottomColor }
		}
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
