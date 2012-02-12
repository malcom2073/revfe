#include "mediaplayer.h"
#include <QDebug>

void callback(const struct libvlc_event_t *evt,void *usr)
{
	/*
	libvlc_MediaPlayerNothingSpecial
	libvlc_MediaPlayerOpening
	libvlc_MediaPlayerBuffering
	libvlc_MediaPlayerPlaying
	libvlc_MediaPlayerPaused
	libvlc_MediaPlayerStopped
	libvlc_MediaPlayerForward
	libvlc_MediaPlayerBackward
	libvlc_MediaPlayerEndReached
	libvlc_MediaPlayerEncounteredError
	libvlc_MediaPlayerTimeChanged
	libvlc_MediaPlayerPositionChanged
	libvlc_MediaPlayerSeekableChanged
	libvlc_MediaPlayerPausableChanged
	libvlc_MediaPlayerTitleChanged
	libvlc_MediaPlayerSnapshotTaken
	libvlc_MediaPlayerLengthChanged */
	//qDebug() << evt->type;
	if (evt->type == libvlc_MediaPlayerOpening)
	{
		//qDebug() << "Media Opening song";
	}
	else if (evt->type == libvlc_MediaPlayerPlaying)
	{
		MediaPlayer *plr = qobject_cast<MediaPlayer*>((QObject*)usr);
		plr->_evt_playing();
		//qDebug() << "Media playing";
	}
	else if (evt->type == libvlc_MediaPlayerStopped)
	{
		//qDebug() << "Media stopped";
	}
	else if (evt->type == libvlc_MediaPlayerEndReached)
	{
		MediaPlayer *plr = qobject_cast<MediaPlayer*>((QObject*)usr);
		plr->_evt_finished();
		//qDebug() << "Media End reached";
	}
	else if (evt->type == libvlc_MediaPlayerForward)
	{
		//qDebug() << "Media Forward";
	}
	else if (evt->type == libvlc_MediaPlayerPositionChanged) //This is disabled atm.
	{
		MediaPlayer *plr = qobject_cast<MediaPlayer*>((QObject*)usr);
		plr->_evt_positionchanged(evt->u.media_player_position_changed.new_position);
	}
	else if (evt->type == libvlc_MediaPlayerTimeChanged)
	{
		MediaPlayer *plr = qobject_cast<MediaPlayer*>((QObject*)usr);
		plr->_evt_timechanged(evt->u.media_player_time_changed.new_time);
	}
	else if (evt->type == libvlc_MediaPlayerTitleChanged)
	{
		//qDebug() << evt->u.media_player_title_changed.new_title;
	}
}
void MediaPlayer::_evt_finished()
{
	emit finished();
}

void MediaPlayer::_evt_timechanged(int current)
{
	//Need to see how expensive libvlc_media_get_duration actually is, and if we should tone down how
	//often we call it.
	emit timeChanged(current,libvlc_media_get_duration(vlcMedia));
}

void MediaPlayer::_evt_positionchanged(double current)
{
	//This is a percentage, from 0.0-1.0. I do not currently use it
}

void MediaPlayer::_evt_playing()
{
	if (seeknext)
	{
		qDebug() << "Seek:" << (float)libvlc_media_get_duration(vlcMedia) / seekloc << (float)libvlc_media_get_duration(vlcMedia) << seekloc;
		seeknext = false;
		libvlc_media_player_set_position(vlcMediaPlayer,(float)seekloc / libvlc_media_get_duration(vlcMedia));
		seekloc = 0;
	}
	emit playing();
	//qDebug() << "New Time:" << libvlc_media_player_get_length(vlcMediaPlayer);
	//qDebug() << "New New time:" << libvlc_media_get_duration(vlcMedia);
}

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent)
{
	seeknext = false;
	vlcInstance = libvlc_new(0,NULL);
	vlcMediaPlayer = 0;
	vlcMedia = 0;
}
MediaPlayer::~MediaPlayer()
{

}

void MediaPlayer::play()
{
	if (vlcMediaPlayer)
	{
		libvlc_media_player_play(vlcMediaPlayer);
	}
}

void MediaPlayer::stop()
{
	if (vlcMediaPlayer)
	{
		libvlc_media_player_stop(vlcMediaPlayer);
	}
}

void MediaPlayer::pause()
{
	libvlc_media_player_pause(vlcMediaPlayer);
}
void MediaPlayer::seek(int loc)
{
	seeknext = true;
	seekloc = loc;
}

void MediaPlayer::setMedia(QString file)
{
	//qDebug() << "MediaPlayer::setMedia" << file;
	if (vlcMediaPlayer)
	{
		//libvlc_event_attach (libvlc_event_manager_t *p_event_manager, libvlc_event_type_t i_event_type, libvlc_callback_t f_callback, void *user_data)
		libvlc_event_manager_t *manager = libvlc_media_player_event_manager(vlcMediaPlayer);
		libvlc_event_manager_t *mediamanager = libvlc_media_event_manager(vlcMedia);
		//libvlc_event_detach(mediamanager,libvlc_MediaStateChanged,&callback,this);
		/*
		libvlc_MediaPlayerNothingSpecial
		libvlc_MediaPlayerOpening
		libvlc_MediaPlayerBuffering
		libvlc_MediaPlayerPlaying
		libvlc_MediaPlayerPaused
		libvlc_MediaPlayerStopped
		libvlc_MediaPlayerForward
		libvlc_MediaPlayerBackward
		libvlc_MediaPlayerEndReached
		libvlc_MediaPlayerEncounteredError
		libvlc_MediaPlayerTimeChanged
		libvlc_MediaPlayerPositionChanged
		libvlc_MediaPlayerSeekableChanged
		libvlc_MediaPlayerPausableChanged
		libvlc_MediaPlayerTitleChanged
		libvlc_MediaPlayerSnapshotTaken
		libvlc_MediaPlayerLengthChanged */
		libvlc_event_detach(manager,libvlc_MediaPlayerOpening,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerBuffering,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerPlaying,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerStopped,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerPaused,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerForward,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerBackward,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerEndReached,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerEncounteredError,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerTimeChanged,&callback,this);
		//libvlc_event_detach(manager,libvlc_MediaPlayerPositionChanged,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerSeekableChanged,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerPausableChanged,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerTitleChanged,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerSnapshotTaken,&callback,this);
		libvlc_event_detach(manager,libvlc_MediaPlayerLengthChanged,&callback,this);

		libvlc_media_release(vlcMedia);
		libvlc_media_player_stop(vlcMediaPlayer);
		libvlc_media_player_release(vlcMediaPlayer);
		vlcMediaPlayer = 0;
		vlcMedia = 0;
	}

	//This little bs code is brought to you by the dolts from vlc and Qt. Qt's QDir dosen't return
	//native path seperators nicely, and VLC won't accept anything but native.
#ifdef Q_OS_WIN32
	vlcMedia = libvlc_media_new_path(vlcInstance,file.replace("/","\\").toAscii());
#else
#ifdef Q_OS_LINUX
	vlcMedia = libvlc_media_new_path(vlcInstance,file.toAscii());
#endif
#endif
	vlcMediaPlayer = libvlc_media_player_new_from_media(vlcMedia);
	libvlc_media_parse(vlcMedia);

	libvlc_event_manager_t *manager = libvlc_media_player_event_manager(vlcMediaPlayer);
	libvlc_event_attach(manager,libvlc_MediaPlayerOpening,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerBuffering,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerPlaying,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerStopped,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerPaused,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerForward,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerBackward,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerEndReached,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerEncounteredError,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerTimeChanged,&callback,this);
	//libvlc_event_attach(manager,libvlc_MediaPlayerPositionChanged,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerSeekableChanged,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerPausableChanged,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerTitleChanged,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerSnapshotTaken,&callback,this);
	libvlc_event_attach(manager,libvlc_MediaPlayerLengthChanged,&callback,this);
	//libvlc_event_manager_t *mediamanager = libvlc_media_event_manager(vlcMedia);
	//libvlc_event_attach(mediamanager,libvlc_MediaStateChanged,&callback,this);
	emit mediaChanged();
}
