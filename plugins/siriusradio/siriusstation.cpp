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

#include "SiriusStation.h"

SiriusStation::SiriusStation() :
   m_isValid(false)
{
   
}

SiriusStation::SiriusStation(int number) :
   m_number(number),
   m_isValid(true)
{
   //Figure out hexNumber
   m_hexNumber = QByteArray::number(m_number, 16);
   if(m_hexNumber.size() < 2)
   {
      m_hexNumber.prepend("0");
   }
}

void SiriusStation::setNumber(int channel)
{
   if(m_number != channel)
   {
      m_number = channel;
      //Figure out hexNumber
      m_hexNumber = QByteArray::number(m_number, 16);
      if(m_hexNumber.size() < 2)
      {
         m_hexNumber.prepend("0");
      }
   }
}

int SiriusStation::number() const
{
   return m_number;
}

QByteArray SiriusStation::hexNumber()
{
   return m_hexNumber;
}

void SiriusStation::setShortName(const QString& name)
{
   if(m_shortName != name)
   {
      m_shortName = name;
   }
}

QString SiriusStation::shortName() const
{
   return m_shortName;
}

void SiriusStation::setLongName(const QString& name)
{
   if(m_longName != name)
   {
      m_longName = name;
   }
}

QString SiriusStation::longName() const
{
   return m_longName;
}

void SiriusStation::setIcon(const QString& icon)
{
   m_icon = icon;
}

QString SiriusStation::icon() const
{
   return m_icon;
}

void SiriusStation::setShortCategory(const QString& category)
{
   if(m_shortCategory != category)
   {
      m_shortCategory = category;
   }
}

QString SiriusStation::shortCategory() const
{
   return m_shortCategory;
}

void SiriusStation::setLongCategory(const QString& category)
{
   if(m_longCategory != category)
   {
      m_longCategory = category;
   }
}

QString SiriusStation::longCategory() const
{
   return m_longCategory;
}

void SiriusStation::setArtist(const QString& artist)
{
   if(m_artist != artist)
   {
      m_artist = artist;
   }
}

void SiriusStation::setCategoryNumber(int num)
{
   if(m_categoryNumber != num)
   {
      m_categoryNumber = num;
   }
}

int SiriusStation::categoryNumber() const
{
   return m_categoryNumber;
}

QString SiriusStation::artist() const
{
   return m_artist;
}

void SiriusStation::setComposer(const QString& composer)
{
   if(m_composer != composer)
   {
      m_composer = composer;
   }
}

QString SiriusStation::composer() const
{
   return m_composer;
}

void SiriusStation::setSong(const QString& song)
{
   if(m_song != song)
   {
      m_song = song;
   }
}

QString SiriusStation::song() const
{
   return m_song;
}

void SiriusStation::setPid(const QString& pid)
{
   if(m_pid != pid)
   {
      m_pid = pid;
   }
}

QString SiriusStation::pid() const
{
   return m_pid;
}

bool SiriusStation::isValid() const
{
   return m_isValid;
}