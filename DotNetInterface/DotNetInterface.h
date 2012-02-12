// DotNetInterface.h
#pragma once

#define MANAGEDWRAPPER_EXPORTS
#include "IWrapperClass.h"



using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace InterfaceClasses; //This comes from Class.cs in in the InterfaceClass directory

void (*passMsgPtr)(void *,std::string,std::string) = NULL;  
void *classPtr;


namespace ManagedPlugins
{
	public interface class IPlugin
	{
		System::String^ Name();
		System::String^ Author();
		System::String^ Description();
		System::String^ Version();

		void Initialize();
		void passPluginMessage(System::String ^sender, System::String ^message);
	};
	public ref class PluginClass
	{
	public:
		PluginClass(System::Object^ plugin);
		System::Collections::Generic::Dictionary<String^,System::Reflection::MethodInfo^> methodList;
		System::Object^ pluginObject;
		System::String^ fileName;

	};
	public ref class ManagedPluginLoader : public IPluginHost
	{
	public:
		void LoadPlugins(System::String ^path,System::Collections::Generic::Dictionary<String^,PluginClass^> ^pluginlist);
		virtual void passCoreMessage(System::String ^sender, System::String ^message);
	private:
		
	};
	//void ManagedPluginLoader::passCoreMessage(System::String ^sender,System::String ^message)
	//{
	//}
	public ref class ManagedPluginInfo
	{
	public:
		ManagedPluginInfo()
		{
			loopThread = true;
			messageList = gcnew List<String^>();
		}
		Thread ^pluginThread;
		List<String^> ^messageList;
		AutoResetEvent ^autoEvent;
		AutoResetEvent ^reverseAutoEvent;
		AutoResetEvent ^closingAutoEvent;
		PluginClass ^plugin;
		bool loopThread;
	};
	public ref class ManagedPluginManager
	{
	public:
		ManagedPluginManager();
		void passPluginMessage(String ^sender, String ^message);
		void LoadPlugins(String ^path);
		void UnloadPlugin(String ^name);
		void ReloadPlugin(String ^name);
		std::string GetPluginNames();
		std::string GetPluginFileNames();
	private:
		Dictionary<String^,ManagedPluginInfo^> ^threadList;
		Dictionary<int,String^> ^threadNameList;
		void OnClickFunc(System::Object ^sender, System::EventArgs ^e);
		ManagedPluginLoader ^pluginLoader;
		void ThreadLoop();
		int i;
		System::Collections::Generic::Dictionary<String^,PluginClass^> ^pluginList;
	};
	static ref class ManagedHolder
	{
	public:
		ManagedHolder()
		{
			myList = gcnew System::Collections::Generic::List<ManagedPluginManager^>();
		}
		static void push(ManagedPluginManager ^plugin)
		{
			myList->Add(plugin);
		}
		static System::Collections::Generic::List<ManagedPluginManager^> ^myList = gcnew System::Collections::Generic::List<ManagedPluginManager^>();
	}; 

}

