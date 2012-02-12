
Global/Core:
appfiles - list of qml skin files (read from Skin.qml)
window global instance of the main window. Functions:
window.passMessage(IPCMessage)
window.saveSetting(name,key,value,type,id) - Save a setting.

MediaLibrary:
This plugin handles all media storage related things, such as Playlist generation, id3 tag reading,
playlist editing, and loading/saving of playlists. This is one of the core plugins, without which
RevFE simply ceases to functon properly, so this plugin, or an equivalant one is required for normal
operation. In this header, I will outline the basic process in which MediaLibrary operates.
MediaLibrary maintains an internal list of all media currently accounted for by the system. This is a list
of MediaFileClass objects. Each object has within it the full file path of the file, as well as an ID3Tag
structure with the pertenant information about the file. MediaLibrary can either generate this information
on the fly, or save and load it from a sqlite3 .db file. Once a folder is scanned for media, each file is added
to the MediaList with an invalid ID3 tag. At this point, a scanning loop is entered, and each tag is
populated with the proper data as the files are read. This is a slow process, hence saving this information
to a sqlite3 database. Once all media is scanned for id3 tag information, you can generate folder based,
or "album" tag based playlists. These playlists contain references to the media files that are contained
by them, to save memory, rather than copies. Once the initial playlist creation is complete, playlists
can be created, deleted, and edited completly independant of the origional playlist structure. The
initial generation is only intended to be a starting point, assuming you have enough media to warrant
multiple playlists.

Once all the media is loaded, you can then tell MediaLibrary to set the current playlist for playing.
This is done via the following command:
"MediaLibrary:playlist set !title!"

This command causes MediaLibrary to make a copy of the selected list, set its name as "Current Playlist",
and throw event:playlistloaded as well as setting the qml property list currentPlaylistModel.


MediaLibrary properties available for QML skin files:

Properties available via propertyMap:
Media_Scanner_Progress - Progress text for media scanning and tag reading

Properties available via listmodels:
currentPlaylistModel
modelData.text = "artist - title"

MEDIA_EDIT_CURRENT_LIST
modelData.text = full path of file

MEDIA_EDIT_FILELIST
modelData.text = list of files in "current" directory

MEDIA_EDIT_DIRLIST
modelData.text = directory of dirs in "current" directory

playlistListModel
modelData.text = playlist name

Properties available via complex listmodels:
none

Events fired in-code:
event:newmedialist
Thrown when the main media list has been updated, so plugins should re-request the list
event:mediascanfinished
Thrown when the media scanner has finished scanning the current set of media.
event:playlistloaded title (optional)autostart (optional+1)pos (optional+2)song
Throw when a new playlist has been loaded. The option parameters are included if media library wishes the MediaEngine to automatically start playback
event:newmediadir
Fired when a new media dir is set. This event is no longer in use


ObdMate:
Properties available via propertyMap:
Any OBD2 pid such as: OBD_010D
These can be added to the "request" list on the settings page



MediaEngine:
This plugin handles the actual playing and audio routing of music files. It will eventually also handle
video playback, but at the moment it only does audio. This is also a core plugin, without which RevFE just
sits idle. MediaEngine has several key features, which tightly integrate it with the MediaLibrary as well
as the GUI. When MediaEngine see's "event:playlistloaded" fired from MediaLibrary, it requests the current
list of songs, via
"MediaLibrary:playlist getsongfilenames !title!"
MediaLibrary replies back with:
"Sender:playlist songlist !title! !basestring! !path1! !album1! !artist1! !title1! !track1! !pathN! !albumN! !artistN! !titleN! !trackN!"
MediaEngine takes this list, and creates its own Playlist of files, including the id3 information.
This allows MediaEngine to have the all the relevant current file information without having to query
MediaLibrary unless a new playlist is loaded.
MediaEngine has a number of useful commands for the handling of media:

MediaEngine:media play !index!
MediaEngine:media pause
MediaEngine:media prev
MediaEngine:media next
MediaEngine:media stop
MediaEngine:media mute
MediaEngine:media setpos !position(as a percentage)!
MediaEngine:media setvolume !volume(as a percentage)!
MediaEngine:media volup
MediaEngine:media voldown

MediaEngine also fires off a few events for other plugins to listen to:
event:mediachange !filename! !index!
event:mediaengine_volumechanged !volume(as a percentage)!
event:mediastate !state!
State can be: "media:playing" or "media:stopped"


MediaEngine properties available for QML skin files:

Properties available via propertyMap:
sys_volume (0-100)
currentSongText (generally tag->artist and tag->title)
currentSongIndex index based on the currentPlaylistModel of the currently playing song
totalTimeText
elapsedTimeText
remainingTimeText


Properties available via listmodels:
none

Properties available via complex listmodels:
none

FuelPrices
This plugin has the ability to retreive a list of fuel stations nearby by either zip code or by
gps coordinates, allowing you to select the stations with the cheapest fuel. This is NOT a core plugin,
but adds extra functionality.

- FuelPrices:fuel get-zip !ZIPCODE!
	Gets list of fuel prices by zip code
- FuelPrices:fuel get-gps
	Gets list of fuel prices by gps. You must have an active GPS fix or this function will fail



Properties available via propertyMap:
none
Properties available via listmodels:
none
Properties available via complex listmodels:
FuelPricesList
modelData.Station_Name
modelData.Station_Address
modelData.Regular
modelData.Midgrade
modelData.Premium

GPSReader:
Properties available via propertyMap:
GPS_LONGITUDE
GPS_LATITUDE
GPS_TIME
GPS_FIXTYPE
GPS_NUMSATS
GPS_ALTITUDE
Properties available via listmodels:
none
Properties available via complex listmodels:
GPS_SAT_INFO
modelData.SAT_PRN
modelData.SAT_ELEVATION
modelData.SAT_AZIMUTH
modelData.SAT_SNR

RadioPal:
Properties available via propertyMap:
RADIOPAL_CURRENTFREQ
RADIOPAL_PRESETX radio presets, indeterminate number from 0-255ish
Properties available via listmodels:
none
Properties available via complex listmodels:
none


Note about settings:
RevFE expects there to be a dynamic settings page configuration in the skin. It will call "addSettingsValue" and "addSettingsPage" to add these dynamic settings.
These functions should create pages, and settings appropriatly.
TODO here, there should be a way to define dynamic settings for a skin, to allow for simpler skin creation, at the expense of usability. This is in theory possible,
however impractical and not yet attempted.



