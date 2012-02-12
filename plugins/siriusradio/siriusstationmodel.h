
#ifndef SIRIUSSTATIONMODEL_H
#define SIRIUSSTATIONMODEL_H
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

#include <QAbstractListModel>
#include <QMap>

#include "SiriusStation.h"

class SiriusStationModel : public QAbstractListModel
{
   Q_OBJECT
public:
   enum UserRoles {
                     LongName=Qt::UserRole,
                     ShortName,
                     Number,
                     Artist,
                     Song,
                     Category,
                     Icon
                  };
   
   SiriusStationModel(QObject* parent=0);
   
   //Inserts if it doesn't exist yet
   void updateStation(const SiriusStation& station);
   
   SiriusStation station(int number);
   
   //QAIM API
   int rowCount(const QModelIndex& parent=QModelIndex()) const;
   QVariant data(const QModelIndex& index, int role) const;
   
private:
   QMap<int,SiriusStation> m_stationsMap;
   
};

#endif //SIRIUSSTATIONMODEL_H

