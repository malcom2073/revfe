/***************************************************************************
*   Copyright (C) 2009 by Michael Carpenter (malcom2073)                  *
*   mcarpenter@interforcesystems.com                                      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#pragma once
#include <string>
//DOT NET MANAGER
#ifdef MANAGEDWRAPPER_EXPORTS
#define DLLAPI  __declspec(dllexport)
#else
#define DLLAPI  __declspec(dllimport)
#endif
#pragma comment (lib, "DotNetInterface.lib") // if importing, link also

void* funcPtr;
class DLLAPI IWrapperClass
{
public:
	static void *init();
	static void passPluginMessage(void *classptr, std::string sender,std::string message);
	static void passCallbackPtr(void *func,void *classptr);
	static void Show(std::string message);
	static void LoadPlugins(std::string path);
	static void UnloadPlugin(std::string name);
	static std::string GetPluginNames();
	static std::string GetPluginFileNames();
};