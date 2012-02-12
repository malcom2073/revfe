#include "videowidget.h"
#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
VideoWidget::VideoWidget(QDeclarativeItem *parent) : QDeclarativeItem(parent)
{
	mediaLoaded = false;
	pix = new QImage(800,600,QImage::Format_RGB32);
	inst = libvlc_new (0, NULL);
	setFlag(QGraphicsItem::ItemHasNoContents,false);
	setFlag(QGraphicsItem::ItemIsFocusable,true);
	timer = new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(timerTick()));
	timer->start(20);
	return;
}
void VideoWidget::timerTick()
{
	//Fix for cpu usage of calling update inside unlock.
	update(boundingRect());
}
static void *lock(void *data, void **p_pixels)
{
	((VideoWidget*)data)->pixmapMutex.lock();
	*p_pixels = ((VideoWidget*)data)->pix->bits();
	return NULL;
}
void VideoWidget::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
	if (newGeometry == oldGeometry)
	{
		QDeclarativeItem::geometryChanged(newGeometry,oldGeometry);
		return;
	}
	if (mediaLoaded)
	{
		//qDebug() << "Geometry Change:" << newGeometry;
		pixmapMutex.lock();
		delete pix;
		pix = new QImage(newGeometry.width(),newGeometry.height(),QImage::Format_RGB32);
		//qDebug() << libvlc_media_get_state(pm);
		libvlc_video_set_format(mp, "RV32", newGeometry.width(), newGeometry.height(), newGeometry.width()*4);
		pixmapMutex.unlock();
	}
	else
	{
		//qDebug() << "Geometry Change:" << newGeometry;
		pixmapMutex.lock();
		delete pix;
		pix = new QImage(newGeometry.width(),newGeometry.height(),QImage::Format_RGB32);
		//qDebug() << libvlc_media_get_state(pm);
		pixmapMutex.unlock();

	}
	QDeclarativeItem::geometryChanged(newGeometry,oldGeometry);

}
void VideoWidget::left()
{
	//libvlc_media_player_navigate(mp,libvlc_navigate_left);
}
void VideoWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	pixmapMutex.lock();
	painter->drawImage(0,0,*pix);
	pixmapMutex.unlock();
}
static void unlock(void *data, void *id, void *const *p_pixels)
{
	((VideoWidget*)data)->pixmapMutex.unlock();
	//((VideoWidget*)data)->update(((VideoWidget*)data)->boundingRect());
}

static void display(void *data, void *id)
{
    (void) data;
}
void VideoWidget::load(QString file)
{
	qDebug() << "Loading media:" << file;

	m = libvlc_media_new_path(inst,file.toAscii());//libvlc_media_new_location (inst, file.toAscii());
	if (!m)
	{
		qDebug() << "Error loading media location:" << libvlc_errmsg();
	}
	mp = libvlc_media_player_new_from_media (m);
	if (!mp)
	{
		qDebug() << "Error loading media player" << libvlc_errmsg();
	}
	libvlc_media_release (m);
	//libvlc_media_player_set_xwindow(mp, this->winId()); //Old code for loading to a winId
	libvlc_video_set_callbacks(mp, lock, unlock, display, this);

	libvlc_video_set_format(mp, "RV32", pix->width(), pix->height(), pix->width()*4);
	mediaLoaded = true;
}
void VideoWidget::mouseReleaseEvent(QMouseEvent *event)
{
	emit clicked();
}
void VideoWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == 16777235)
	{
		//16777235 -- Up
		//16777234 --Left
		//16777237 -- Down
		//16777236 -- Right
		//16777220 -- Enter
	}
	qDebug() << event->key();
}
void VideoWidget::keyReleaseEvent(QKeyEvent *event)
{
}
void VideoWidget::mousePressEvent(QMouseEvent *event)
{
//libvlc_media_player_navigate(
//	libvlc_navigate_mode_t {
//  libvlc_navigate_activate = 0, libvlc_navigate_up, libvlc_navigate_down, libvlc_navigate_left,
//  libvlc_navigate_right
}
void VideoWidget::pause()
{
	libvlc_media_player_pause(mp);
}
void VideoWidget::unload()
{
	mediaLoaded = false;
	libvlc_media_player_stop (mp);
	libvlc_media_player_release (mp);
}
void VideoWidget::play()
{
	libvlc_media_player_play(mp);
	libvlc_audio_set_volume(mp,100);
}
void VideoWidget::stop()
{
	libvlc_media_player_stop(mp);
}

QString VideoWidget::GetTitle(QString file)
{
	libvlc_instance_t * pinst;
	libvlc_media_player_t *pmp;
	libvlc_media_t *pm;
	pinst = libvlc_new (0, NULL);
	pm = libvlc_media_new_path (pinst, file.toStdString().c_str());
	pmp = libvlc_media_player_new_from_media (pm);
	libvlc_media_player_play (pmp);
	while (libvlc_media_get_state(pm) != 3)
	{
		libvlc_media_get_state(pm);
	}
	char *meta = libvlc_media_get_meta (pm, libvlc_meta_Title);
	QString title = QString(meta);
	title.detach();
	libvlc_media_release (pm);
	libvlc_media_player_stop (pmp);
	libvlc_media_player_release (pmp);
	libvlc_release (pinst);
	return title;
}
