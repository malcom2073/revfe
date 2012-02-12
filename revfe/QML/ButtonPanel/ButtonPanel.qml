


import Qt 4.7

GridView
{
	//width: 300; height: 200
	model: propertyMap.utilityAppModel
	delegate: Column
	{
		Text { text: modelData; anchors.horizontalCenter: parent.horizontalCenter }
	}
}

