#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <vlc/vlc.h>
class MediaPlayer : public QObject
{
	Q_OBJECT
public:
	MediaPlayer(QObject *parent = 0);
	~MediaPlayer();
	void play();
	void stop();
	void pause();
	void setMedia(QString file);
	void _evt_playing();
	void _evt_positionchanged(double current);
	void _evt_timechanged(int current);
	void _evt_finished();
	void seek(int loc);
private:
	bool seeknext;
	int seekloc;
	libvlc_instance_t *vlcInstance;
	libvlc_media_player_t *vlcMediaPlayer;
	libvlc_media_t *vlcMedia;

signals:
	void mediaChanged();
	void timeChanged(unsigned long current,unsigned long total);
	void finished();
	void stopped();
	void playing();
public slots:

};

#endif // MEDIAPLAYER_H
