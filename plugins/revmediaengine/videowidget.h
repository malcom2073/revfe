#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
#include <QWidget>
#include <QMutex>
#include <QTimer>
#include <QDeclarativeItem>
#include "vlc/vlc.h"

class VideoWidget : public QDeclarativeItem
{
	Q_OBJECT
public:
	VideoWidget(QDeclarativeItem *parent=0);
	Q_INVOKABLE void load(QString file);
	void unload();
	Q_INVOKABLE void play();
	Q_INVOKABLE void left();
	void pause();
	void stop();
	QString GetTitle(QString file);
	QImage *pix;
	QMutex pixmapMutex;
signals:
	void clicked();
protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
private slots:
	void timerTick();
private:
	bool mediaLoaded;
	QTimer *timer;
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;
};

#endif // VIDEOWIDGET_H
