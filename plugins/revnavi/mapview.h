#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QDeclarativeItem>
#include <osmscout/Database.h>
#include <osmscout/MapPainterQt.h>
#include <osmscout/StyleConfigLoader.h>
#include <osmscout/Util.h>


class MapView : public QDeclarativeItem
{
	Q_OBJECT
public:
	Q_PROPERTY(int contentX READ getScrollX WRITE setScrollX);
	Q_PROPERTY(int contentY READ getScrollY WRITE setScrollY);
	Q_INVOKABLE void scrollFinished();
	MapView(QDeclarativeItem *parent = 0);
	int getScrollX();
	int getScrollY();
	void setScrollX(int x);
	void setScrollY(int y);
	void drawMap();
private:
	bool changed;
	osmscout::DatabaseParameter databaseParameter;
	osmscout::Database          *database;
	osmscout::StyleConfig *styleConfig;
	osmscout::MercatorProjection  projection;
	osmscout::MapParameter        drawParameter;
	osmscout::AreaSearchParameter searchParameter;
	osmscout::MapData             data;
	osmscout::MapPainterQt        mapPainter;
	double lat;
	double lon;
	double zoom;
	int width;
	int height;
	int contentX;
	int contentY;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *style , QWidget * w);
	void geometryChanged (const QRectF &newgeometry,const QRectF &oldgeometry);
	QPixmap *m_bgPixMap;
	int m_mapX;
	int m_mapY;
	QPixmap *topleft;
	QPixmap *middleleft;
	QPixmap *bottomleft;
	QPixmap *topcenter;
	QPixmap *middlecenter;
	QPixmap *bottomcenter;
	QPixmap *topright;
	QPixmap *middleright;
	QPixmap *bottomright;
signals:

public slots:

};

#endif // MAPVIEW_H
