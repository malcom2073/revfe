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

#include "Sirius.h"

#include <QTimer>
#include <QFile>
#include <QtDebug>

//Megasuirt gets ttyUSB0 so use ttyUSB1
//XXX TODO Poll for right port
#ifdef MEGA_SQUIRT
static const QString PORT_NAME = "/dev/ttyUSB1";
#else
static const QString PORT_NAME = "/dev/ttyUSB0";
#endif

static const QByteArray StartBits = QByteArray::fromHex("a40300");

Sirius::Sirius(QObject* parent) :
   QObject(parent),
   m_currentStation(21),
   m_currentGain(0),
   m_currentMute(false),
   m_currentSignal(0),
   m_currentPower(false),
   m_sequenceNumber(0),
   m_lastAck(255),
   m_lastSeq(255),
   m_currentVerbosity(false),
   m_signalOverall(0),
   m_signalSat(0),
   m_signalTower(0),
   m_lastGottenChannel(0)
 {
   m_stationModel = new SiriusStationModel(this);
    
   //Setup Data Structs
   encodeComands();
   setupStations();
   
   //XXX FIXME
   //Load last radio state (channel mostly)
   //Use QSettings
   

   //MLC - This has been disabled and the functions made public
   //Open Connection to SCC1 Sirius Brain
   //openSerial();
   //initSCC1();
   
}

Sirius::~Sirius()
{
   cleanupSCC1();
   closeSerial();
 #ifdef SCC1
   delete m_port;
 #endif
}
   
bool Sirius::setCurrentStation(quint8 station)
{
   qDebug() << "setCurrentStation" << station;
   
   m_currentStation = station;
   //Offset is always 0
   QString offset = "00";
   QString stationStr = QString::number(station, 16);
   //Pad Station
   if(stationStr.size() < 2)
   {
      stationStr.prepend("0");
   }
   QString cmd;
   cmd += encodedCommand(ChannelPrefix);
   cmd += stationStr;
   cmd += offset;
   cmd += encodedCommand(ChannelSuffix);
   sendCommand(cmd);
   
   emit currentStationChanged(station);
   return true;
}

quint8 Sirius::currentStation() const
{
   return m_currentStation;
}

bool Sirius::setVerbosity(bool updatesForAllChannels)
{
   m_currentVerbosity = updatesForAllChannels;
   if(true == updatesForAllChannels)
      return sendCommand(encodedCommand(Verbosity) +"1f00");
   else
      return sendCommand(encodedCommand(Verbosity) +"0000");
}

bool Sirius::verbosity() const
{
   return m_currentVerbosity;
}
   
bool Sirius::setGain(quint8 gain)
{
   m_currentGain = gain;
   
   QString gainStr = QString::number(gain,16);
   if(gainStr.size() < 2)
      gainStr.prepend("0");
   return sendCommand(encodedCommand(Gain)+gainStr);
}

bool Sirius::gain() const
{
   return m_currentGain;
}
   
bool Sirius::setMute(bool muteOn)
{
   m_currentMute = muteOn;
   QString muteStr = QString::number(0,16);
   
   if(muteOn)
      muteStr = QString::number(1,16);
   //Need to pad to a whole byte
   muteStr.prepend("0");
   
   return sendCommand(encodedCommand(Mute)+muteStr);
}

bool Sirius::mute() const
{
   return m_currentMute;
}
   
bool Sirius::refreshSignal()
{
   return sendCommand(encodedCommand(RequestSignal));
}

quint8 Sirius::signal() const
{
   return m_currentSignal;
}

bool Sirius::refreshSid()
{
   return sendCommand(encodedCommand(RequestSid));
}

QString Sirius::sid() const
{
   return m_sid;
}

bool Sirius::setPower(bool powerOn)
{
   m_currentPower = powerOn;
   
   if(powerOn == true)
   {
      return sendCommand(encodedCommand(PowerOn));
   }
   else
   {
      return sendCommand(encodedCommand(PowerOff));
   }
}

bool Sirius::power() const
{
   return m_currentPower;
}
   
bool Sirius::reset()
{
   return sendCommand(encodedCommand(Reset));
}

QString Sirius::channelName(int channel) const
{
   return m_stationModel->station(channel).longName();
}

QString Sirius::channelGenre(int channel) const
{
   return m_stationModel->station(channel).longCategory();
}

QString Sirius::channelIcon(int channel) const
{
   return m_stationModel->station(channel).icon();
}

QString Sirius::channelArtist(int channel) const
{
   return m_stationModel->station(channel).artist();
}

QString Sirius::channelSong(int channel) const
{
   return m_stationModel->station(channel).song();
}
   
QString Sirius::encodedCommand(CommandType cmd)
{
   return m_encodedCommands.value(cmd);
}

char Sirius::checksum(const QByteArray& data) const
{
   char sum = 0;
   for(int i=0; i<data.size(); i++)
   {
      sum += data.at(i);
   }
   
   if( (sum % 0x100) == 0 )
   {
      return 0;
   }
   //Else
   char checksum = 0x100 - (sum % 0x100);
   return checksum;
}

QByteArray Sirius::filterEscapes(const QByteArray& data) const
{
   //Change 1b1b to 1b
   static QByteArray before = QByteArray::fromHex("1b1b");
   static QByteArray after = QByteArray::fromHex("1b");
   QByteArray tmp = data;
   return tmp.replace(before, after);
}

QByteArray Sirius::addEscapes(const QByteArray& data) const
{
   //Change 1b to 1b1b
   static QByteArray before = QByteArray::fromHex("1b");
   static QByteArray after = QByteArray::fromHex("1b1b");
   QByteArray tmp = data;
   return tmp.replace(before, after);
}

bool Sirius::sendCommand(const QString& command)
{
#ifdef SCC1
   QByteArray hexCommand = QByteArray::fromHex(command.toLatin1());
   quint8 seq = m_sequenceNumber;

   QByteArray rawData;
   rawData.append(StartBits);
   rawData.append(seq);
   qint8 type = Command;
   rawData.append(type);
   rawData.append((quint8)hexCommand.size());
   rawData.append(hexCommand);

   char chksum = checksum(rawData);
   
   //Escapes don't count as length.
   QByteArray data;
   data.append(StartBits);
   data.append(seq);
   data.append(type);
   data.append((quint8)hexCommand.size());
   data.append(hexCommand);
   data.append(chksum);

   // Filter escapes
   data = addEscapes(data);

   qDebug() << "\t\tSending" << data.toHex();
   
   //Try to send a message for 2seconds before giving up
   //Most messages go on the 1st 3 tries. PowerOn/Off tend to take forever
   int attempts=0;
   for(attempts=0; attempts<27; attempts++)
   {
      qDebug() << "Trying" << attempts+1;
      m_port->write(data);
      readData();
      if(m_lastAck == seq)
      {
         qDebug() << "\t\tGot ack after" << attempts+1 << "attempts";
         break;
      }
      SiriusThread::msleep(75);
      qDebug() << "Trying again";
   }

   m_sequenceNumber++;
   //Give up. Print error
   if( attempts >= 27 &&
       m_lastAck == seq )
   {
      qDebug() << "\t\tCommand not acknowledged by tuner after 10 attempts (2.0sec)";
      return false;
   }
#endif //SCC1
   return true;
}

bool Sirius::sendAck(quint8 sequenceNum)
{
 #ifdef SCC1
   qDebug () << "sendAck" << sequenceNum << m_port->bytesAvailable();
   
   QByteArray ackData;
   ackData.append(StartBits);
   ackData.append(sequenceNum);
   quint8 type = Ack;
   ackData.append(type);
   ackData.append((char)0);
   ackData.append(checksum(ackData));
   int bytes = m_port->write(ackData);
   m_port->waitForBytesWritten(500);
   //m_port->flush();
   bool allWritten = (bytes == ackData.size());
   
   if(!allWritten)
   {
      qDebug() << "Ack write failed" << bytes << ackData.size();
   }
   
   return allWritten;
#else
   return false;
#endif //SCC1
}

void Sirius::handleReadyRead()
{
   readData();
}

void Sirius::readData()
{
 #ifdef SCC1
   qDebug() << "readData()";
   
   //Bail if no data
   if(m_port->bytesAvailable() == 0)
   {
      return;
   }
   
   QByteArray header;
   //Keep reading headers/data while data is available
   while (1)
   { 
      header += m_port->read(6 - header.size());
      
      //Look for hex a40300
      int indxStart = header.indexOf(StartBits,0);
 
      if(-1 == indxStart)
      {
         //Header doesn't include header data, bail
         //Probably middle of some packet
         qDebug() << "Not header data! Clearing buffer.";
         header.clear();
         if(m_port->bytesAvailable() > 0)
            continue;
         else
            break;
      }
      
      if(0 != indxStart)
      {
         //Remove BS data so StartBits is 1st
         header.remove(0, indxStart);
         qDebug() << "Partial header waiting for more.";
         continue;
      }
      
      if (header.size() < 6)
      {
         qDebug() << "Partial header waiting for more.";
         continue;
      }
      
      // Remove escapes 0x1b1b (replace with single escape 0x1b)
      // Re-read for size of removed escapes. Unescaped header should be 6 bytes
      header = filterEscapes(header);
      int headerReReadSize = 6 - header.size();
      if(0 > headerReReadSize)
      {
         header += m_port->read(headerReReadSize);
      }

      QByteArray start = header.left(3);
      quint8 seq = header[3];
      quint8 type = header[4];
      quint8 length = header[5]; 

      //Special case where length == 1b (escape character)
      //Read 1 to flush extra bit. The length in this case is 1b1b
      //So we don't bother reading again. That bit will also be 1b
      if(length == 0x1b)
      {
         qDebug() << "Flushing 1 char on length == 0x1b";
         m_port->read(1);
      }
      
      //Read data and checksum (checksum not included in length)
      QByteArray data = m_port->read(length+1); //+1 for checksum bit
      //Force read all
      int tries =0;
      while(data.size() < length+1 &&
            tries < 3)
      {
         qDebug() << "Looking for" << length+1 << "have" << data.size() << tries;
         data += m_port->read(length+1 - data.size());
         tries++;
      }
      
      //Did we get stalled (shouldn't really happen since we try real hard above)
      //For now let's bail, but I think this could be fixed with another loop...
      if(data.size() < length+1)
      {
         qDebug() << "Timeout on data read. Bailing.";
         sendAck(seq); //Ack anyway for now? XXX FIXME
         break;
      }

      //Remove escapes before checksum
      //The re-read. XXX MAY NEED RE-RE-READ! LOOP!
      //XXX FIXME
      data = filterEscapes(data);
      int dataReReadSize = length - data.size();
      if(dataReReadSize > 0)
      {
         qDebug() << "Want" << dataReReadSize << "more";
         while(data.size() < length+1)
         {
            qDebug() << "Getting more data";
            data += m_port->read(dataReReadSize);
         } 
      }
      
      //Try again...? Need loop!!! XXX FIXME
      data = filterEscapes(data);
      dataReReadSize = length - data.size();
      if(dataReReadSize > 0)
      {
         qDebug() << "Want" << dataReReadSize << "more";
         while(data.size() < length+1)
         {
            qDebug() << "Getting more data";
            data += m_port->read(dataReReadSize);
         } 
      }
      
      //Time to get the checksum
      char dataChksum = data.at(data.size()-1);
      //Remove checksum
      data = data.remove(data.size()-1,1);
      char calcChksum = checksum(header+data);//Checksum both header and data portion
      
      //Check the sums 
      if(calcChksum != dataChksum)
      {
         //Send back bad chksum message XXX FIXME
         qDebug() << "Bad Checksum" << (quint8)calcChksum << (quint8)dataChksum;
      }
      
      //Break out on ack. No real data.
      if(type == Ack)
      {
         qDebug() << "Found ack" << seq;
         m_lastAck = seq;
         break;
      }
      
      //Ack back anything other packet
      sendAck(seq);
     
      if(type == Command)
      {
         //Skip repeats
         if(seq == m_lastSeq)
         {
            qDebug() << "\t\tDUPLICATE PACKET Skipping...";
            header.clear();
            continue;
         }
         m_lastSeq = seq;
         
         bool ok;
         //XXX FIXME Switch to casting bits rather than ascii-hex-int conversions
         quint16 updateType = data.left(2).toHex().toInt(&ok,16);
         //Remove command from data to make processing easier
         data.remove(0,2);
         //Call the correct update handler
         switch(updateType)
         {
         case PowerUpdate:
            handleUpdatePower(data);
            break;
         case VolumeUpdate:
            handleUpdateVolume(data);
            break;
         case MuteUpdate:
            handleUpdateMute(data);
            break;
         case ChannelUpdate:
            handleUpdateChannelUpdate(data);
            break;
         case VerbosityUpdate:
            handleUpdateVerbosity(data);
            break;
          case NewChannelUpdate:
            handleNewChannel(data);
            break;
         case SidUpdate:
            handleUpdateSid(data);
            break;
         case UnknlUpdate:
            handleUpdateUnknl(data);
            break;
         case SignalUpdate:
            handleUpdateSignal(data);
            break;
         case ChannelInfoUpdate:
            handleUpdateChannelInfo(data);
            break;
         case PidUpdate:
            handleUpdatePid(data);
            break;
         case TimeUpdate:
            handleUpdateTime(data);
            break;
         case TunerUpdate:
            handleUpdateTuner(data);
            break;
         case SignalInfoUpdate:
            handleUpdateSignalInfo(data);
            break;
         default:
            qDebug() << "\t\tUNKNOWN UPDATE." << QString::number(updateType, 16) <<"hex. Skipping...";
            break;
         }
      }
      else if(type == ErrorBusy)
      {
         qDebug() << "\t\t BUSY! TRY AGAIN.";
      }
      else if(type == ErrorChecksum)
      {
         qDebug() << "\t\t BAD CHECKSUM! TRY AGAIN.";
      }
      else
      {
         qDebug() << "\t\tNOT COMMAND" << QString::number(type, 16) << "hex. Skipping...";
      }
      
      //Should we go another round?
      if(6 < m_port->bytesAvailable())
      {
         header.clear();
         continue;
      }
      else
      {
         break;
      }
   }
   
   //Should be 5 or less bytes on the port
   //when we break out
#endif //SCC1
}

void Sirius::encodeComands()
{
   m_encodedCommands.insert(PowerOff, "000800");
   m_encodedCommands.insert(Reset, "0009");
   m_encodedCommands.insert(PowerOn, "000803");
   m_encodedCommands.insert(Gain, "0002");
   m_encodedCommands.insert(Mute, "0003");
   m_encodedCommands.insert(ChannelPrefix, "000a");
   m_encodedCommands.insert(ChannelSuffix, "000b");
   m_encodedCommands.insert(RequestSignal, "4018");
   m_encodedCommands.insert(RequestUnkl, "4017"); //XXX What does this do!
   m_encodedCommands.insert(RequestSid, "4011");
   m_encodedCommands.insert(Verbosity, "000d000000");
   m_encodedCommands.insert(RequestChannelInfo, "4008");
}

void Sirius::setupStations()
{
   //Read in last saved CSV
   QFile csvFile("Sirius/SiriusStations.csv");
   if(true == csvFile.open(QIODevice::ReadOnly))
   {
      QString csvFileString = csvFile.readAll();
      QStringList csvLines = csvFileString.split("\n");
      foreach(const QString& line, csvLines)
      {
         QStringList stationInfo = line.split(",");
         
         SiriusStation station(stationInfo[0].toInt());
         station.setLongName(stationInfo[1]);
         station.setShortName(stationInfo[2]);
         station.setLongCategory(stationInfo[3]);
         station.setIcon(stationInfo[4]);
         m_stationModel->updateStation(station);
         
      }
      
      csvFile.close();
   }
   
   //XXX TODO We ought to write the file after we get the channel lineup from SiriusStation
   //Some of the short names/categories are guessed in the inital file.
}

QAbstractItemModel* Sirius::stationModel()
{
   return m_stationModel;
}

bool Sirius::openSerial(const QString &port)
{
#ifdef SCC1
   QxtSerialDevice::PortSettings settings;
   settings |= QxtSerialDevice::Bit8;
   settings |= QxtSerialDevice::FlowOff;
   settings |= QxtSerialDevice::ParityNone;
   settings |= QxtSerialDevice::Stop1;

   m_port = new QxtSerialDevice(port.toStdString().c_str());
   m_port->setBaud(QxtSerialDevice::Baud57600);
   m_port->setPortSettings(settings);
   connect(m_port, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));

   //Open comm to Sirius SCC1
   if(m_port->open(QIODevice::ReadWrite))
   {
      qDebug() << "Opened comm with SCC1 on" << PORT_NAME;
      return true;
   }
   else
   {
      qDebug() << "Cannot open comm with SCC1 on" << PORT_NAME;
      return false;
   }
#endif
   //Shouldn't get this far
   return false;
}

void Sirius::closeSerial()
{
 #ifdef SCC1
   if(true == m_port->isOpen())
      m_port->close();
#endif
}

bool Sirius::initSCC1()
{

   //Reset
   reset();
   setPower(false);
   setPower(true);
   setGain(0);
   refreshSignal();
   refreshSid();
   setMute(false);
   setVerbosity(true);
   
   //Set station
   setCurrentStation(m_currentStation);
   
   //Get Channel List
   //Probably want to save to a file and note fetch each time
   //Only on special occasion or force refresh
   refreshChannelList();
   
   return true;
 
}

void Sirius::cleanupSCC1()
{
   
}

void Sirius::handleUpdatePower(const QByteArray& data)
{
   qDebug() << "\t\thandleUpdatePower" << data.toHex();
   
}

void Sirius::handleUpdateVolume(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateVolume" << data.toHex();
}

void Sirius::handleUpdateMute(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateMute" << data.toHex();
}

void Sirius::handleUpdateChannelUpdate(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateChannelUpdate" << data.toHex();
   
   QByteArray myData = data;
   bool ok;
   //XXX FIXME Switch to casting bits rather than ascii-hex-int conversions
   int number = myData.left(1).toHex().toInt(&ok,16);
   myData.remove(0,2); //And skip a byte.
   
   //int categoryNum =  myData.left(1).toHex().toInt(&ok,16);
   myData.remove(0,3); //Skip two bytesAvailable
   
   int shortNameStrLen = myData.left(1).toHex().toInt(&ok,16);
   QString shortName = myData.mid(1,shortNameStrLen);
   myData.remove(0,shortNameStrLen+1); //Plus 1 is for the length bit
   
   int longNameStrLen = myData.left(1).toHex().toInt(&ok,16);
   QString longName = myData.mid(1,longNameStrLen);
   myData.remove(0,longNameStrLen+1); //Plus 1 is for the length bit
   
   int shortCatStrLen = myData.left(1).toHex().toInt(&ok,16);
   QString shortCat = myData.mid(1,shortCatStrLen);
   myData.remove(0,shortCatStrLen+1); //Plus 1 is for the length bit
   
   int longCatStrLen = myData.left(1).toHex().toInt(&ok,16);
   QString longCat = myData.mid(1,longCatStrLen);
   myData.remove(0,longCatStrLen+1); //Plus 1 is for the length bit
   
   qDebug() << "\t\tGot station" << number << shortName << longName << shortCat << longCat;
   
   //Comes with song/artist too :)
   handleChannelItemUpdate(number, myData);
}

void Sirius::handleUpdateVerbosity(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateVerbosity" << data.toHex();
}

void Sirius::handleUpdateSid(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateSid" << data.toHex();
}

void Sirius::handleUpdateUnknl(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateUknl" << data.toHex();
}

void Sirius::handleUpdateSignal(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateSignal" << data.toHex();
}

void Sirius::handleUpdateChannelInfo(const QByteArray& data)
{
   qDebug() << "\t\thandleUpdateChannelInfo" << data.toHex();
 
   QByteArray myData = data;
 
   bool ok;
   //XXX FIXME Switch to casting bits rather than ascii-hex-int conversions
   int channel = myData.left(1).toHex().toInt(&ok,16);
   qDebug() << "\t\tChannel" << myData.left(1).toHex() << channel;
   myData.remove(0,1);
   handleChannelItemUpdate(channel, myData);
}

void Sirius::handleUpdatePid(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdatePid" << data.toHex();
}

void Sirius::handleUpdateTime(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateTime" << data.toHex();
}

void Sirius::handleUpdateTuner(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateTuner" << data.toHex();
}

void Sirius::handleUpdateSignalInfo(const QByteArray& data)
{
    qDebug() << "\t\thandleUpdateSignalInfo" << data.toHex();
   
   bool ok;
   //XXX FIXME Switch to casting bits rather than ascii-hex-int conversions
   m_signalOverall = data.mid(0,1).toHex().toInt(&ok, 16) * 0.33;
   m_signalSat = data.mid(1,1).toHex().toInt(&ok, 16) * 0.33;
   m_signalTower = data.mid(2,1).toHex().toInt(&ok, 16) * 0.33;
   
   qDebug() << "Signal" << m_signalOverall << m_signalSat << m_signalTower;
   
   //Emit signal updates
   
}

void Sirius::handleChannelItemUpdate(int channel, const QByteArray& data)
{  
   bool ok;
   //XXX FIXME Switch to casting bits rather than ascii-hex-int conversions
   int numItems = data.mid(0,1).toHex().toInt(&ok, 16);
   qDebug() << "Processing" << numItems <<"Items for Channel" << channel;
   int offset=0;
   for(int i=0; i<numItems; i++)
   {
      qDebug() << "Item Type" << (quint8)data[0] << "Length" << (quint8)data[1];
      int itemType = data.mid(offset+1,1).toHex().toInt(&ok, 16);
      int itemStrLen = data.mid(offset+2,1).toHex().toInt(&ok, 16);
      QString itemStr = QString(data.mid(offset+3,itemStrLen));
      qDebug() << "Item" << itemStr;
      offset = offset + itemStrLen+2;
      
      //Create channel if it does not exist
      if(false == m_stationModel->station(channel).isValid())
      {
         m_stationModel->updateStation(SiriusStation(channel));
      }
      
      //Get current station info
      SiriusStation station = m_stationModel->station(channel);
      
      switch(itemType)
      {
      case Artist:
         station.setArtist(itemStr);
         break;
      case Title:
         station.setSong(itemStr);
         break;
      case Composer:
         station.setComposer(itemStr);
         break;
      case Pid:
         station.setPid(itemStr);
         break;
      default:
         qDebug() << "Unknown channel item" << QString::number(itemType,16);
      }
      
      //Update station with new merged data
      m_stationModel->updateStation(station);
   }
}

void Sirius::refreshChannelList()
{
   m_lastGottenChannel = 0;
   requestNextChannelInfoRec(true);
}

void Sirius::requestNextChannelInfoRec(bool acceptZero)
{
   //Method is indirectly recursive via queued invokeMethod

   //Check for last station
   //Break recursion
   if(false == acceptZero &&
      0 == m_lastGottenChannel)
   {
      return;
   }

   QString stationStr = QString::number(m_lastGottenChannel, 16);
   //Pad Station
   if(stationStr.size() < 2)
   {
      stationStr.prepend("0");
   }
   
   QString command = encodedCommand(RequestChannelInfo);
   command += stationStr;
   command += "01";
   command += "0000";
   sendCommand(command);
   
   //Setup next run with some cool off time  
   QTimer::singleShot(1000, this, SLOT(requestNextChannelInfoRec()));
   
}

void Sirius::handleNewChannel(const QByteArray& data)
{
   quint8 channel = (quint8)data[2];
   
   quint8 shortNameLen = (quint8)data[7];
   quint8 shortNameIdx = 8;
   QString shortName = data.mid(shortNameIdx,shortNameLen);
   
   quint8 longNameLen = (quint8)data[shortNameIdx+shortNameLen];
   quint8 longNameIdx = shortNameIdx+shortNameLen+1;
   QString longName = data.mid(longNameIdx,longNameLen);
   
   quint8 catLen = (quint8)data[longNameIdx+longNameLen];
   quint8 catIdx = longNameIdx+longNameLen+1;
   QString catName = data.mid(catIdx,catLen);
   
   //Some stations only ahve short names
   if(longName.isEmpty())
   {
      longName = shortName;
   }
   
   SiriusStation station(channel);
   station.setShortName(shortName);
   station.setLongName(longName);
   station.setLongCategory(catName);
   m_stationModel->updateStation(station);
   
   
   m_lastGottenChannel = channel;
}
