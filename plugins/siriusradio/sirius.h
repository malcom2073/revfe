/*   
    Copyright (C) 2010,2011  Integrated Computer Solutions (ICS) www.ics.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    This file has been modified by Michael Carpenter (malcom2073). Any
    and all modifications are released copyright free, as a part of the
    aforementioned license.
*/

#ifndef SIRIUS_H
#define SIRIUS_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QMap>
#include <QIcon>

#include "SiriusStation.h"
#include "SiriusStationModel.h"
#define SCC1
#ifdef SCC1
#include <QxtCore/qxtserialdevice.h>
#endif

class Sirius : public QObject
{
   Q_OBJECT
   Q_PROPERTY(quint8 currentStation READ currentStation WRITE setCurrentStation NOTIFY currentStationChanged)
   
public:
   Sirius(QObject* parent=0);
   ~Sirius();
   
   QAbstractItemModel* stationModel();
   
   quint8 currentStation() const;
   
   bool setVerbosity(bool updatesForAllChannels);
   bool verbosity() const;
   
   bool setGain(quint8 gain);
   bool gain() const;
   
   bool setMute(bool muteOn);
   bool mute() const;
   
   bool refreshSignal();
   quint8 signal() const;
   
   bool refreshSid();
   QString sid() const;
   
   bool setPower(bool powerOn);
   bool power() const;
   
   bool reset();
   
   void refreshChannelList();
   
   QString channelName(int channel) const;
   QString channelGenre(int channel) const;
   QString channelIcon(int channel) const;
   QString channelArtist(int channel) const;
   QString channelSong(int channel) const;

   //MLC - These have been changed to public to allow for finer control
   //Init functions
   void encodeComands();
   void setupStations();

   //MLC - We want to be able to specificy the port in code
   bool openSerial(const QString &port);
   void closeSerial();
   bool initSCC1();
   void cleanupSCC1();

public slots:
   bool setCurrentStation(quint8 station);
   void requestNextChannelInfoRec(bool acceptZero=false);
   
signals:
   void currentStationChanged(int channel);
   void channelNameChanged(int channel, const QString& newName);
   void channelGenreChanged(int channel, const QString& newGenre);
   void channelIconChanged(int channel, const QIcon& newIcon);
   void channelArtistChanged(int channel, const QString& newArtist);
   void channelSongChanged(int channel, const QString& newSong);
   
protected:
   enum CommandType {
                  PowerOff=0,
                  Reset,
                  PowerOn,
                  Gain,
                  Mute,
                  ChannelPrefix,
                  ChannelSuffix,
                  RequestSignal,
                  RequestUnkl,
                  RequestSid,
                  Verbosity,
                  RequestChannelInfo
                };

   //SIRIUS Radio code redacted. License Issues.             
   enum MessageType {
                     Command,
                     Ack,
                     ErrorBusy,
                     ErrorChecksum
                    };
                
   //SIRIUS Radio code redacted. License Issues.   
   enum UpdateType {
                     PowerUpdate,
                     VolumeUpdate,
                     MuteUpdate,
                     ChannelUpdate,
                     VerbosityUpdate,
                     NewChannelUpdate,
                     SidUpdate,
                     UnknlUpdate,
                     SignalUpdate,
                     ChannelInfoUpdate,
                     PidUpdate,
                     TimeUpdate,
                     TunerUpdate,
                     SignalInfoUpdate
   };
   
   //SIRIUS Radio code redacted. License Issues.   
   enum ChannelItemType {
                          Artist,
                          Title,
                          Composer,
                          Pid
                    };
   
   QString encodedCommand(CommandType cmd);
   char checksum(const QByteArray& data) const;
   QByteArray filterEscapes(const QByteArray& data) const;
   QByteArray addEscapes(const QByteArray& data) const;
   bool sendCommand(const QString& command);
   bool sendAck(quint8 sequenceNum);
   void readData();
   

   
protected slots:
   void handleReadyRead();
   
   void handleUpdatePower(const QByteArray& data);
   void handleUpdateVolume(const QByteArray& data);
   void handleUpdateMute(const QByteArray& data);
   void handleUpdateChannelUpdate(const QByteArray& data);
   void handleUpdateVerbosity(const QByteArray& data);
   void handleUpdateSid(const QByteArray& data);
   void handleUpdateUnknl(const QByteArray& data);
   void handleUpdateSignal(const QByteArray& data);
   void handleUpdateChannelInfo(const QByteArray& data);
   void handleUpdatePid(const QByteArray& data);
   void handleUpdateTime(const QByteArray& data);
   void handleUpdateTuner(const QByteArray& data);
   void handleUpdateSignalInfo(const QByteArray& data);
   void handleChannelItemUpdate(int channel, const QByteArray& data);
   void handleNewChannel(const QByteArray& data);
   
private:
   
   quint8 m_currentStation;
   quint8 m_currentGain;
   bool m_currentMute;
   quint8 m_currentSignal;
   bool m_currentPower;
   quint8 m_sequenceNumber;
   quint8 m_lastAck;
   quint8 m_lastSeq;
   QString m_sid;
   bool m_currentVerbosity;
   int m_signalOverall;
   int m_signalSat;
   int m_signalTower;
   SiriusStationModel* m_stationModel;
   QMap<int, QString> m_encodedCommands;
   quint8 m_lastGottenChannel;
#ifdef SCC1
   QxtSerialDevice* m_port;
#endif //SCC1

};

class SiriusThread : public QThread
{
   Q_OBJECT
public:
   static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
};


#endif //SIRIUS_H
