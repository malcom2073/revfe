// This is the main DLL file.

#include "stdafx.h"

#include "DotNetInterface.h"


namespace ManagedPlugins
{
	PluginClass::PluginClass(System::Object^ plugin)
	{
		pluginObject = plugin;
	}

	void ManagedPluginLoader::LoadPlugins(System::String ^path,System::Collections::Generic::Dictionary<String^,PluginClass^> ^pluginlist)
	{
		//System::Windows::Forms::MessageBox::Show("Weee1" + path);
		try
		{
			//System::IO::Directory ^pathDir = gcnew System::IO::Directory();
			cli::array<System::String^,1> ^files = System::IO::Directory::GetFiles(path);
			int i =0;
			for each (System::String^ file in files)
			{
				if (file->EndsWith(".dll"))
				{
				try
				{
					//System::Windows::Forms::MessageBox::Show("Files: " + file);
					i=1;
					System::Reflection::Assembly ^pluginAssembly = System::Reflection::Assembly::LoadFile(file);
					i=2;
					Object ^o = pluginAssembly->CreateInstance("Plugin");
					i=3;
					PluginClass ^tmpClass = gcnew PluginClass(o);
					tmpClass->fileName = file->Replace("\\","/");;
					i=4;
					System::Type ^tmpType = pluginAssembly->GetType("Plugin");
					i=5;
					System::Reflection::MethodInfo ^methodInfo = pluginAssembly->GetType("Plugin")->GetMethod("passPluginMessage", gcnew array<Type^> { String::typeid, String::typeid });
					i=6;
					System::Reflection::MethodInfo ^initMethod = pluginAssembly->GetType("Plugin")->GetMethod("Initialize");
					i=10;
					System::Reflection::MethodInfo ^nameMethod = pluginAssembly->GetType("Plugin")->GetMethod("Name");
					i=11;
					tmpClass->methodList.Add("passPluginMessage",methodInfo);
					i=12;
					tmpClass->methodList.Add("Initialize",initMethod);
					i=13;
					IPluginHost^ tmpHost = safe_cast<IPluginHost^>(this);
					i=14;
					System::String ^retVar = safe_cast<String^>(nameMethod->Invoke(o,gcnew cli::array<Object^,1>(0)));
					i=15;
					initMethod->Invoke(o,gcnew array<IPluginHost^> {tmpHost} );
					i=16;
					//methodInfo->Invoke(o,gcnew array<System::String^> {gcnew System::String("DotNetInterface"),gcnew System::String("Let's see a message!")});
					//System::Windows::Forms::MessageBox::Show("Works!");
					pluginlist->Add(retVar,tmpClass);
					i=17;
				}
				catch (System::Exception ^ex)
				{
					if (i >= 2)
					{
						System::Windows::Forms::MessageBox::Show(file + "::" + ex->Message + "::" + i.ToString());
					}
				}
				}
			}
		}
		catch (System::Exception ^ex)
		{
			System::Windows::Forms::MessageBox::Show(ex->Message);
		}
	}
	void ManagedPluginLoader::passCoreMessage(System::String ^sender, System::String ^message)
	{
		//System::Windows::Forms::MessageBox::Show(message);
		try
		{
		char* senderChar = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(sender);
		char* messageChar = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(message);
		std::string senderStr = std::string(senderChar);
		std::string messageStr = std::string(messageChar);
		passMsgPtr(classPtr,senderStr,messageStr);
		System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)senderChar);
		System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)messageChar);
		
		}
		catch (Exception ^ex)
		{
			System::Windows::Forms::MessageBox::Show(ex->Message);
		}
	}
	ManagedPluginManager::ManagedPluginManager()
	{
		//Need to load plugins here
		threadList = gcnew Dictionary<String^,ManagedPluginInfo^>();
		threadNameList = gcnew Dictionary<int,String^>();
		pluginList = gcnew System::Collections::Generic::Dictionary<String^,PluginClass^>();
		pluginLoader = gcnew ManagedPluginLoader();

		
	}
	std::string ManagedPluginManager::GetPluginNames()
	{
		std::string retVar = "";
		for each (System::Collections::Generic::KeyValuePair<String^,PluginClass^> kvp in pluginList)
		{
			std::string tmpVar = "";
			char* str = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(kvp.Key);
			tmpVar = tmpVar.append(std::string(str));
			retVar = retVar.append(tmpVar).append(":");
			System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)str);
		}
		if (retVar.length() > 0)
		{
			retVar = retVar.substr(0,retVar.length()-1);
		}
			return retVar;
	}
	std::string ManagedPluginManager::GetPluginFileNames()
	{
		std::string retVar = "";
		for each (System::Collections::Generic::KeyValuePair<String^,PluginClass^> kvp in pluginList)
		{
			std::string tmpVar = "";
			char* str = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(kvp.Value->fileName);
			tmpVar = tmpVar.append(std::string(str));
			retVar = retVar.append(tmpVar).append("|");
			System::Runtime::InteropServices::Marshal::FreeHGlobal((System::IntPtr)str);
		}
		if (retVar.length() > 0)
		{
			retVar = retVar.substr(0,retVar.length()-1);
		}
		return retVar;
	}
	void ManagedPluginManager::LoadPlugins(String ^path)
	{
		pluginLoader->LoadPlugins(path,pluginList);
		for each (KeyValuePair<String^,PluginClass^> kvp in pluginList)
		{
			Thread ^tmpThread = gcnew Thread(gcnew ThreadStart(this,&ManagedPluginManager::ThreadLoop));
			threadNameList->Add(tmpThread->ManagedThreadId,kvp.Key);
			ManagedPluginInfo ^tmpInfo = gcnew ManagedPluginInfo();
			tmpInfo->pluginThread = tmpThread;
			tmpInfo->autoEvent = gcnew AutoResetEvent(false);
			tmpInfo->reverseAutoEvent = gcnew AutoResetEvent(false);
			tmpInfo->closingAutoEvent = gcnew AutoResetEvent(false);
			tmpInfo->plugin = kvp.Value;
			tmpInfo->loopThread = true;
			threadList->Add(kvp.Key,tmpInfo);
			tmpThread->Start();
			//threadList.
		}
	}
	void ManagedPluginManager::ReloadPlugin(String ^name)
	{
		if (threadList->ContainsKey(name))
		{
			if (threadList[name]->loopThread == false)
			{
				
			}
		}
	}
	void ManagedPluginManager::UnloadPlugin(String ^name)
	{
		if (threadList->ContainsKey(name))
		{
			if (threadList[name]->loopThread == true)
			{
				//threadList[name]->pluginThread->Abort();
				//threadList[name]->messageList->Add(sender + ":" + message);
				threadList[name]->messageList->Add("core:event initclose");
				threadList[name]->loopThread = false;
				threadList[name]->autoEvent->Set();
				threadList[name]->closingAutoEvent->WaitOne();
				return;
			}
/*			}
			else if ((message->Contains("initstarted")) || (message->Contains("initcomplete")))
			{
				threadList[name]->messageList->Add(sender + ":" + message);
				threadList[name]->autoEvent->Set();
				threadList[name]->reverseAutoEvent->WaitOne();
			}
			else
			{
				threadList[name]->messageList->Add(sender + ":" + message);
				threadList[name]->autoEvent->Set();
			}
			*/
		}
		
	}

	void ManagedPluginManager::ThreadLoop()
	{
		String ^name = threadNameList[Thread::CurrentThread->ManagedThreadId];
		bool loop = true;
		while (threadList[name]->loopThread)
		{
			try
			{
				while(threadList[name]->messageList->Count > 0)
				{
					String^ message = threadList[name]->messageList[0];
					String ^sender = message->Substring(0,message->IndexOf(":"));
					message = message->Substring(message->IndexOf(":")+1);
					threadList[name]->plugin->methodList["passPluginMessage"]->Invoke(threadList[name]->plugin->pluginObject,gcnew cli::array<String^,1> {sender,message});
					threadList[name]->messageList->RemoveAt(0);
					if (message->Contains("initstarted") || message->Contains("initcomplete"))
					{
						threadList[name]->reverseAutoEvent->Set();
						//Thread::CurrentThread->Abort();
						//System::Windows::Forms::MessageBox::Show("Plugin closing");
						
					}
				}
			}
			catch (Exception ^ex)
			{
				System::Windows::Forms::MessageBox::Show(ex->Message);
			}
			threadList[name]->autoEvent->WaitOne();
		}
		threadList[name]->closingAutoEvent->Set();
		//System::Windows::Forms::MessageBox::Show("Plugin Closed");
	}
	void ManagedPluginManager::OnClickFunc(System::Object ^sender, System::EventArgs ^e)
	{
		//System::Windows::Forms::MessageBox::Show("Weee" + i.ToString());
		
	}
	void ManagedPluginManager::passPluginMessage(String ^sender, String ^message)
	{
		if (message->IndexOf(":") != -1)
		{
			String ^name = message->Substring(0,message->IndexOf(":"));
			if (threadList->ContainsKey(name))
			{
				if (threadList[name]->loopThread)
				{
					if (message->Contains("initclose"))
					{
						//threadList[name]->pluginThread->Abort();
						threadList[name]->messageList->Add(sender + ":" + message);
						threadList[name]->loopThread = false;
						threadList[name]->autoEvent->Set();
						threadList[name]->closingAutoEvent->WaitOne();
						return;
					}
					else if ((message->Contains("initstarted")) || (message->Contains("initcomplete")))
					{
						threadList[name]->messageList->Add(sender + ":" + message);
						threadList[name]->autoEvent->Set();
						threadList[name]->reverseAutoEvent->WaitOne();
					}
					else
					{
						threadList[name]->messageList->Add(sender + ":" + message);
						threadList[name]->autoEvent->Set();
					}
				}
			}
			return;
			//System::Windows::Forms::MessageBox::Show("NAME!!: " +name);
			for each (System::Collections::Generic::KeyValuePair<String^,PluginClass^> kvp in pluginList)
			{
				if (kvp.Key == name)
				{
					kvp.Value->methodList["passPluginMessage"]->Invoke(kvp.Value->pluginObject,gcnew cli::array<String^,1> { sender,message});
					if (!message->Contains("initstarted"))
					{
						if (!message->Contains("initcomplete"))
						{
							//System::Windows::Forms::MessageBox::Show("Target Found!");
						}
					}
					return;
				}
			}
			System::Windows::Forms::MessageBox::Show("No key found!");
			
			//for (int i=0;i<pluginList->cou
		}
		System::Windows::Forms::MessageBox::Show("No good message");
		//if (message->StartsWith("Load "))
		//{
		//	pluginLoader->LoadPlugins(message->Substring(5));
		//}
	}

}

void *IWrapperClass::init()
{
	try
	{
		ManagedPlugins::ManagedPluginManager ^pluginManager = gcnew ManagedPlugins::ManagedPluginManager();
		ManagedPlugins::ManagedHolder::push(pluginManager);
		return 0;
	}
	catch (System::Exception ^ex)
	{
		System::Windows::Forms::MessageBox::Show(ex->Message);
		return 0;
	}

}
void IWrapperClass::passPluginMessage(void *classptr, std::string sender,std::string message)
{
	try
	{
		String ^m_sender = gcnew String(sender.c_str());
		String ^m_message = gcnew String(message.c_str());
		ManagedPlugins::ManagedHolder::myList[0]->passPluginMessage(m_sender,m_message);
	}
	catch (System::Exception ^ex)
	{
		System::Windows::Forms::MessageBox::Show(ex->Message);
	}
}
void IWrapperClass::passCallbackPtr(void* func,void *classptr)
{
	try
	{
		passMsgPtr = ((void(__cdecl*)(void*,std::string,std::string))func);
		classPtr = classptr;
	}
	catch (System::Exception ^ex)
	{
		System::Windows::Forms::MessageBox::Show(ex->Message);
	}
}
void IWrapperClass::LoadPlugins(std::string path)
{
	try
	{
		//ManagedPlugins::ManagedHolder::myList
		ManagedPlugins::ManagedHolder::myList[0]->LoadPlugins(gcnew String(path.c_str()));
	}
	catch (System::Exception ^ex)
	{

	}
}
std::string IWrapperClass::GetPluginNames()
{
	try
	{
		if (ManagedPlugins::ManagedHolder::myList->Count > 0)
		{
			return ManagedPlugins::ManagedHolder::myList[0]->GetPluginNames();
		}
		else
		{
			return "";
		}
	}
	catch (System::Exception ^ex)
	{
		return "";
	}
}
void IWrapperClass::UnloadPlugin(std::string name)
{
	try
	{
		String ^m_name= gcnew String(name.c_str());
		ManagedPlugins::ManagedHolder::myList[0]->UnloadPlugin(m_name);	
	}
	catch (System::Exception ^ex)
	{
		
	}
}
std::string IWrapperClass::GetPluginFileNames()
{
	try
	{
		if (ManagedPlugins::ManagedHolder::myList->Count > 0)
		{
			return ManagedPlugins::ManagedHolder::myList[0]->GetPluginFileNames();
		}
		else
		{
			return "";
		}
	}
	catch (System::Exception ^ex)
	{
		return "";
	}
}