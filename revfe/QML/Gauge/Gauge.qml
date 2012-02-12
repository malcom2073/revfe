import Qt 4.7




Image {
	id: gauge
	Image {
		id: needle
		source: "needle.png"
		x: (gauge.width / 2) - (needle.width / 2)
		y: (gauge.height / 2) - (needle.height - 15)
		width: 20
		height: gauge.height / 2
		transform: Rotation { origin.x: needle.width / 2; origin.y: needle.height - 15; angle: gauge.angle }
	}
	Behavior on angle {  PropertyAnimation { properties: "angle"; duration: 300 } }
	onValueChanged:
	{
		angle = minAngle + (((300.0 / (maxValue - minValue)) * (value - minValue)));
	}
	property int minAngle;
	property int maxAngle;
	property int angle;
	property int minValue;
	property int maxValue;
	property int value: -1; //This makes it update on first load, even if you set it to zero.

}
