import Qt 4.7
QtObject {

	property string globalFontName: "Ariel"
	property int globalTextSize: 20
	property color globalTextColor: "white"

	//Button properties
	property color buttonBgColor: "black"
	property color buttonOutlineColor: "red"
	property color buttonTextColor: globalTextColor
	property int buttonTextSize:  globalTextSize
	property string buttonFontName: globalFontName
	property int buttonBorderSize: 5
	property int buttonBorderRadius: 10

	//Textbox properties
	property int textBoxTextSize:  globalTextSize
	property string textBoxFontName: globalFontName
	property color textBoxTextColor: globalTextColor


}
