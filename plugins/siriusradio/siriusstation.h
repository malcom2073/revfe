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

#ifndef SIRIUSSTATION_H
#define SIRIUSSTATION_H

#include <QObject>
#include <QByteArray>

class SiriusStation
{
public:
   SiriusStation();
   
   SiriusStation(int number);
   
   void setNumber(int channel);
   int number() const;
   
   QByteArray hexNumber();
   
   void setShortName(const QString& name);
   QString shortName() const;
   
   void setLongName(const QString& name);
   QString longName() const;
   
   void setIcon(const QString& fileName);
   QString icon() const;
   
   void setShortCategory(const QString& category);
   QString shortCategory() const;
   
   void setLongCategory(const QString& category);
   QString longCategory() const;
   
   void setCategoryNumber(int num);
   int categoryNumber() const;
   
   void setArtist(const QString& artist);
   QString artist() const;
   
   void setComposer(const QString& composer);
   QString composer() const;
   
   void setSong(const QString& song);
   QString song() const;
   
   void setPid(const QString& pid);
   QString pid() const;
   
   bool isValid() const;
   
private:
   int m_number;
   QByteArray m_hexNumber;
   QString m_shortName;
   QString m_longName;
   QString m_icon;
   QString m_shortCategory;
   QString m_longCategory;
   int m_categoryNumber;
   QString m_artist;
   QString m_composer;
   QString m_song;
   QString m_pid;
   bool m_isValid;
   
};
   
#endif //SIRIUSSTATION_H
