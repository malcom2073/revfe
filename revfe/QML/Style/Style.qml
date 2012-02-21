import Qt 4.7
QtObject {

	property string globalFontName: "Ariel"
	property int globalTextSize: 20
	property color globalTextColor: "white"

	//Button properties
	property color buttonBgColor: "black"
	property color buttonOutlineColor: "grey"
	property color buttonTextColor: globalTextColor
	property int buttonTextSize:  globalTextSize
	property string buttonFontName: globalFontName
	property int buttonBorderSize: 5
	property int buttonBorderRadius: 20
	property color buttonBgTopColor: "grey";
	property color buttonBgBottomColor: "black";

	//Textbox properties
	property int textBoxTextSize:  globalTextSize
	property string textBoxFontName: globalFontName
	property color textBoxTextColor: globalTextColor

	//Listbox properties
	property color listBgTopColor: "dark grey"
	property color listBgBottomColor: "light grey"
	property int listBorderRadius: 50


}
