#include "mapview.h"
#include <QDebug>
#include <QPixmap>
MapView::MapView(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
	setFlag(QGraphicsItem::ItemHasNoContents, false);
	database = new osmscout::Database(databaseParameter);
	QString map = "/home/michael/map"; // map base directory
	QString style = "/home/michael/map/standard.oss"; //style file
	if (!database->Open(map.toStdString().c_str()))
	{
		qDebug() << "Unable to open map";
		return;
	}
	styleConfig = new osmscout::StyleConfig(database->GetTypeConfig());
	if (!osmscout::LoadStyleConfig(style.toStdString().c_str(),*styleConfig))
	{
		qDebug() << "Cannot open style";
		return;
	}
	width = 800;
	height = 600;
	m_mapX = 0;
	m_mapY = 0;
	//top = 0;
	//bottom = 0;
	//middle = 0;
	m_bgPixMap = new QPixmap(width,height);
	//top = new QPixmap(width,height);
	//middle = new QPixmap(width,height);
	//bottom = new QPixmap(width,height);
	topleft = new QPixmap(width,height);
	middleleft = new QPixmap(width,height);
	bottomleft = new QPixmap(width,height);
	topcenter = new QPixmap(width,height);
	middlecenter = new QPixmap(width,height);
	bottomcenter = new QPixmap(width,height);
	topright = new QPixmap(width,height);
	middleright = new QPixmap(width,height);
	bottomright = new QPixmap(width,height);
	lat = 39.3305;
	lon = -76.6881;
	zoom = 1000.0;
	//QPixmap *pixmap=new QPixmap(width,height);
	//drawMap(true);

	drawMap();


}
void MapView::drawMap()
{

	QPainter *topleftpainter = new QPainter(topleft);
	QPainter *middleleftpainter = new QPainter(middleleft);
	QPainter *bottomleftpainter= new QPainter(bottomleft);
	QPainter *topcenterpainter = new QPainter(topcenter);
	QPainter *middlecenterpainter = new QPainter(middlecenter);
	QPainter *bottomcenterpainter= new QPainter(bottomcenter);
	QPainter *toprightpainter = new QPainter(topright);
	QPainter *middlerightpainter = new QPainter(middleright);
	QPainter *bottomrightpainter= new QPainter(bottomright);

	projection.Set(lon,lat,zoom,width,height);
	double mag = projection.GetMagnification();
	double width = projection.GetWidth();
	double latmin = projection.GetLatMin();
	double latmax = projection.GetLatMax();
	double lonmin = projection.GetLonMin();
	double lonmax = projection.GetLonMax();

	projection.Set(lonmin + (lonmax-lonmin),latmin + (latmax-latmin),lonmax + (lonmax-lonmin),latmax + (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,toprightpainter);

	projection.Set(lonmin + (lonmax-lonmin),latmin,lonmax + (lonmax-lonmin),latmax,mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,middlerightpainter);

	projection.Set(lonmin + (lonmax-lonmin),latmin - (latmax-latmin),lonmax + (lonmax-lonmin),latmax - (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,bottomrightpainter);



	projection.Set(lonmin,latmin + (latmax-latmin),lonmax,latmax + (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,topcenterpainter);

	projection.Set(lonmin,latmin,lonmax,latmax,mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,middlecenterpainter);

	projection.Set(lonmin,latmin - (latmax-latmin),lonmax,latmax - (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,bottomcenterpainter);


	projection.Set(lonmin - (lonmax-lonmin),latmin + (latmax-latmin),lonmax - (lonmax-lonmin),latmax + (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,topleftpainter);

	projection.Set(lonmin - (lonmax-lonmin),latmin,lonmax - (lonmax-lonmin),latmax,mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,middleleftpainter);

	projection.Set(lonmin - (lonmax-lonmin),latmin - (latmax-latmin),lonmax - (lonmax-lonmin),latmax - (latmax-latmin),mag,width);
	database->GetObjects(*styleConfig,projection.GetLonMin(),projection.GetLatMin(),projection.GetLonMax(),projection.GetLatMax(),projection.GetMagnification(),searchParameter,data.nodes,data.ways,data.areas,data.relationWays,data.relationAreas);
	mapPainter.DrawMap(*styleConfig,projection,drawParameter,data,bottomleftpainter);


	topleftpainter->end();
	middleleftpainter->end();
	bottomleftpainter->end();
	topcenterpainter->end();
	middlecenterpainter->end();
	bottomcenterpainter->end();
	toprightpainter->end();
	middlerightpainter->end();
	bottomrightpainter->end();

	delete topleftpainter;
	delete middleleftpainter;
	delete bottomleftpainter;
	delete topcenterpainter;
	delete middlecenterpainter;
	delete bottomcenterpainter;
	delete toprightpainter;
	delete middlerightpainter;
	delete bottomrightpainter;
}

int MapView::getScrollX()
{
	return contentX;
}

int MapView::getScrollY()
{
	return contentY;
}

void MapView::setScrollX(int x)
{
	qDebug() << "X:" << x;
	contentX = x;
	m_mapX += x;
	lon -= ((double)x) / 10000.0;
	changed = true;
	this->update();
}

void MapView::setScrollY(int y)
{
	qDebug() << "Y:" << y;
	m_mapY += y;
	contentY = y;
	lat -= ((double)y)/10000.0;
	changed = true;
	this->update();
}
void MapView::scrollFinished()
{
	//drawMap();
	this->update();
}

void MapView::paint(QPainter *painter, const QStyleOptionGraphicsItem *style , QWidget * w)
{

	//painter->drawPixmap(0,0,800,600,*m_bgPixMap);
	if (!topleft || !middleleft || !bottomleft || !topcenter || !middlecenter || !bottomcenter || !topright || !middleright || !bottomright)
	{
		return;
	}
	painter->drawPixmap(m_mapX,m_mapY,800,600,*topleft);
	painter->drawPixmap(m_mapX,m_mapY+600,800,600,*middleleft);
	painter->drawPixmap(m_mapX,m_mapY+1200,800,600,*bottomleft);

	painter->drawPixmap(m_mapX+800,m_mapY,800,600,*topcenter);
	painter->drawPixmap(m_mapX+800,m_mapY+600,800,600,*middlecenter);
	painter->drawPixmap(m_mapX+800,m_mapY+1200,800,600,*bottomcenter);

	painter->drawPixmap(m_mapX+1600,m_mapY,800,600,*topright);
	painter->drawPixmap(m_mapX+1600,m_mapY+600,800,600,*middleright);
	painter->drawPixmap(m_mapX+1600,m_mapY+1200,800,600,*bottomright);

	painter->drawRect(0,0,800,600);
	painter->drawRect(0,600,800,600);
	painter->drawRect(0,1200,800,600);
	painter->drawRect(800,0,800,600);
	painter->drawRect(800,600,800,600);
	painter->drawRect(800,1200,800,600);
	painter->drawRect(1600,0,800,600);
	painter->drawRect(1600,600,800,600);
	painter->drawRect(1600,1200,800,600);
	//if (changed)
	//{
		//drawMap(painter);
	//	changed = false;
	//}
	//p//ainter->drawImage();
}
void MapView::geometryChanged (const QRectF &newgeometry,const QRectF &oldgeometry)
{

}
