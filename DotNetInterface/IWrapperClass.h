#pragma once
#include <string>
//DOT NET INTERFACE
#ifdef MANAGEDWRAPPER_EXPORTS
#define DLLAPI  __declspec(dllexport)
#else
#define DLLAPI  __declspec(dllimport)
#pragma comment (lib, "DotNetInterface.lib") // if importing, link also
#endif
void *funcPtr;
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