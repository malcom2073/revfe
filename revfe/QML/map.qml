import Qt 4.7
import "Window"
import MapView 0.1

Window {
	name: "mapview"
	id: mapview
	x: 0
	y: 0
	width: 924
	height: 600
	Flickable {
		id: flicker
		x:0
		y:0
		width: 924
		height:600
		contentX:0
		flickableDirection: Flickable.HorizontalAndVerticalFlick
		contentY:0
		contentWidth:2400
		contentHeight: 1800
		onMovementEnded: {
			mapviewctl.scrollFinished();
		}

	}
	MapView {
		id: mapviewctl;
		x:0
		y:0
		width:2400
		height:1800
		contentX: flicker.contentX;
		contentY: flicker.contentY;
	}
	clip: true


}
