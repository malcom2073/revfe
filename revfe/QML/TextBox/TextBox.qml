import Qt 4.7
import "../Style"
Item {
	Style { id: style }
	property alias text: _internalText.text
	Text {
		id: _internalText
		color: style.textBoxTextColor
		font.pixelSize: style.textBoxTextSize
		font.family: style.textBoxFontName
	}
}
