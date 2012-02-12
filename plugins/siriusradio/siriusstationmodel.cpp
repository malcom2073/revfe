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
*/

#include "SiriusStationModel.h"

SiriusStationModel::SiriusStationModel(QObject* parent) :
   QAbstractListModel(parent)
{
   QHash<int,QByteArray> roleMap;
   roleMap.insert(LongName, "longName");
   roleMap.insert(ShortName, "shortName");
   roleMap.insert(Number, "number");
   roleMap.insert(Artist, "artist");
   roleMap.insert(Song, "song");
   roleMap.insert(Category, "category");
   roleMap.insert(Icon, "icon");
   setRoleNames(roleMap);
}
   
void SiriusStationModel::updateStation(const SiriusStation& station)
{
   //Check for insert
   if(false == this->station(station.number()).isValid())
   {
      //XXX Be dumb for now.
      //Probably an append search backwards
      QList<int> keys = m_stationsMap.keys();
      int insertionRow = keys.size();
      while(insertionRow != 0)
      {
         if(keys.value(insertionRow-1) < station.number())
         {
            break;
         }
         insertionRow--;
      }
      beginInsertRows(QModelIndex(),insertionRow,insertionRow);
      m_stationsMap.insert(station.number(), station);
      endInsertRows();
   }
   else //Simple update
   {
      m_stationsMap.insert(station.number(), station);
      int row = m_stationsMap.keys().indexOf(station.number());
      QModelIndex idx = index(row,0,QModelIndex());
      emit dataChanged(idx,idx);
   }
}

SiriusStation SiriusStationModel::station(int number)
{
   return m_stationsMap.value(number);
}

//QAIM API
int SiriusStationModel::rowCount(const QModelIndex& parent) const
{
   if(parent.isValid())
   {
      return 0;
   }
   else
   {
      return m_stationsMap.keys().size();
   }
   
}

QVariant SiriusStationModel::data(const QModelIndex& index, int role) const
{
   //XXX This may suck
   //Depends on how QMap stores it's keys
   int station = m_stationsMap.keys().value(index.row());
   
   switch(role)
   {
   case LongName:
      return m_stationsMap.value(station).longName();
   case ShortName:
      return m_stationsMap.value(station).shortName();
   case Number:
      return m_stationsMap.value(station).number();
   case Artist:
      return m_stationsMap.value(station).artist();
   case Song:
      return m_stationsMap.value(station).song();
   case Category:
      return m_stationsMap.value(station).longCategory();
   case Icon:
      return m_stationsMap.value(station).icon();
   default:
      return QVariant();
   }
   
   return QVariant();
}
