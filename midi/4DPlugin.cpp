/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : MIDI
 #	author : miyako
 #	2016/03/21
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

#pragma mark -

namespace MIDI
{
#if VERSIONWIN
typedef struct
{
	HANDLE hThread;
	HANDLE hProcess;
	HANDLE hStdInput;
	HANDLE hStdOutput;
	HANDLE hStdError;
}context;
#endif
#if VERSIONWIN
	HMODULE pluginBundle = NULL;
	std::map<DWORD, MIDI::context *> tasks;
#else
	NSBundle *pluginBundle = nil;
	std::map<int, NSTask*> tasks;
#endif
	
#if VERSIONWIN
	void taskDeleteAll()
	{
		for (auto it = MIDI::tasks.begin(); it != MIDI::tasks.end(); ++it)
		{
			MIDI::context *c = it->second;
			DWORD exitCode;
			if (GetExitCodeProcess(c->hProcess, &exitCode))
			{
				if(exitCode == STILL_ACTIVE)
				{
					TerminateProcess(c->hProcess, 1);
				}
			}
			//pi
			CloseHandle(c->hProcess);
			CloseHandle(c->hThread);
			//si
			CloseHandle(c->hStdInput);
			CloseHandle(c->hStdOutput);
			CloseHandle(c->hStdError);
			delete c;
		}
		MIDI::tasks.clear();
	}
#else
	void taskDeleteAll()
	{
		for (auto it = MIDI::tasks.begin(); it != MIDI::tasks.end(); ++it)
		{
			NSTask *task = it->second;
			
			if([task isRunning])
				[task terminate];
		}
		MIDI::tasks.clear();
	}
#endif
	
#if VERSIONWIN
	HANDLE taskGet(C_LONGINT &index)
	{
		HANDLE task = NULL;
		std::map<DWORD, MIDI::context *>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			MIDI::context *c = pos->second;
			task = c->hProcess;
		}
		return task;
	}
#else
	NSTask *taskGet(C_LONGINT &index)
	{
		NSTask *task = NULL;
		std::map<int, NSTask*>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			task = pos->second;
		}
		return task;
	}
#endif

	void taskSuspend(C_LONGINT &index)
	{
#if VERSIONWIN
		std::map<DWORD, MIDI::context *>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			MIDI::context *c = pos->second;
			SuspendThread(c->hThread);
		}
#else
	NSTask *task = MIDI::taskGet(index);
	if(task)
		[task suspend];
#endif
	}
	
	void taskResume(C_LONGINT &index)
	{
#if VERSIONWIN
		std::map<DWORD, MIDI::context *>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			MIDI::context *c = pos->second;
			ResumeThread(c->hThread);
		}
#else
	NSTask *task = MIDI::taskGet(index);
	if(task)
		[task resume];
#endif
	}

	bool taskIsRunning(C_LONGINT &index)
	{
		bool isRunning = false;
#if VERSIONWIN
		DWORD exitCode;
		HANDLE h = MIDI::taskGet(index);
		if (GetExitCodeProcess(h, &exitCode))
		{
			isRunning = (exitCode == STILL_ACTIVE);
		};
#else
		NSTask *task = MIDI::taskGet(index);
		if(task)
		{
			isRunning = [task isRunning];
		}
#endif
		return isRunning;
	}
	
#if VERSIONWIN
	void taskSet(PROCESS_INFORMATION *pi, STARTUPINFO *si)
	{
		MIDI::context *c = new MIDI::context;

		c->hThread = pi->hThread;
		c->hProcess = pi->hProcess;
		c->hStdInput = si->hStdInput;
		c->hStdOutput = si->hStdOutput;
		c->hStdError = si->hStdError;
		
		MIDI::tasks.insert(std::map<DWORD, MIDI::context *>::value_type(pi->dwProcessId, c));
	}
#else
	NSTask * taskCreate()
	{
		NSTask *task = [[NSTask alloc]init];
		MIDI::tasks.insert(std::map<int, NSTask*>::value_type([task processIdentifier], task));
		return task;
	}
#endif

#if VERSIONWIN
	void taskDelete(C_LONGINT &index)
	{
		HANDLE task = NULL;
		std::map<DWORD, MIDI::context *>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			MIDI::context *c = pos->second;
			
			DWORD exitCode;
			if (GetExitCodeProcess(c->hProcess, &exitCode))
			{
				if(exitCode == STILL_ACTIVE)
				{
					TerminateProcess(c->hProcess, 1);
				}
			};
			//pi
			CloseHandle(c->hProcess);
			CloseHandle(c->hThread);
			//si
			CloseHandle(c->hStdInput);
			CloseHandle(c->hStdOutput);
			CloseHandle(c->hStdError);
			delete c;
			MIDI::tasks.erase(pos);
		}
	}
#else
	void taskDelete(C_LONGINT &index)
	{
		NSTask *task = NULL;
		std::map<int, NSTask*>::iterator pos = MIDI::tasks.find(index.getIntValue());
		if(pos != MIDI::tasks.end())
		{
			task = pos->second;
			
			if([task isRunning])
				[task terminate];
			
			[task release];
			
			MIDI::tasks.erase(pos);
		}
	}
#endif
}

bool IsProcessOnExit()
{
    C_TEXT name;
    PA_long32 state, time;
    PA_GetProcessInfo(PA_GetCurrentProcessNumber(), name, &state, &time);
    CUTF16String procName(name.getUTF16StringPtr());
    CUTF16String exitProcName((PA_Unichar *)"$\0x\0x\0\0\0");
    return (!procName.compare(exitProcName));
}

void OnStartup()
{
#if VERSIONWIN
	MIDI::pluginBundle = GetModuleHandleW(L"MIDI.4DX");
#else
	MIDI::pluginBundle = [NSBundle bundleWithIdentifier:@"com.4D.4DPlugin.midi"];
#endif
}

void OnCloseProcess()
{
    if(IsProcessOnExit())
		{
			MIDI::taskDeleteAll();
    }
}

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
        case kInitPlugin :
        case kServerInitPlugin :            
            OnStartup();
            break;    

        case kCloseProcess :            
            OnCloseProcess();
            break;
			
// --- MIDI

		case 1 :
			MIDI_Play(pResult, pParams);
			break;

		case 2 :
			MIDI_ABORT(pResult, pParams);
			break;

		case 3 :
			MIDI_SUSPEND(pResult, pParams);
			break;

		case 4 :
			MIDI_RESUME(pResult, pParams);
			break;

		case 5 :
			MIDI_Is_running(pResult, pParams);
			break;
			
	}
}

// ------------------------------------- MIDI -------------------------------------

void MIDI_Is_running(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;
	C_LONGINT returnValue;

	Param1.fromParamAtIndex(pParams, 1);

	returnValue.setIntValue(MIDI::taskIsRunning(Param1));
	returnValue.setReturn(pResult);
}

void MIDI_Play(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	ARRAY_TEXT Param2;
	C_LONGINT returnValue;

	Param1.fromParamAtIndex(pParams, 1);
	Param2.fromParamAtIndex(pParams, 2);

	if(MIDI::pluginBundle)
	{
			uint32_t countArgs = Param2.getSize();
		
#if VERSIONWIN
			wchar_t	fDrive[_MAX_DRIVE], fDir[_MAX_DIR], fName[_MAX_FNAME], fExt[_MAX_EXT];
			wchar_t thisPath[_MAX_PATH] = {0};
			GetModuleFileNameW(MIDI::pluginBundle, thisPath, _MAX_PATH);
			_wsplitpath_s(thisPath, fDrive, fDir, fName, fExt);
			std::wstring windowsPath = fDrive;
			windowsPath+= fDir;
			//remove delimiter to go one level up the hierarchy
			if(windowsPath.at(windowsPath.size() - 1) == L'\\')
				windowsPath = windowsPath.substr(0, windowsPath.size() - 1);
			_wsplitpath_s(windowsPath.c_str(), fDrive, fDir, fName, fExt);
			std::wstring resourcesPath = fDrive;
			resourcesPath += fDir;
		
			std::wstring currentDirectoryPath = resourcesPath + L"Resources\\timidity";
		
//			std::wstring configPath = resourcesPath + L"Resources\\timidity\\timidity.cfg";
			std::wstring configPath = L"timidity.cfg";
		
			std::wstring arguments = L"\"";
			arguments += windowsPath;
			arguments += L"\\timidity.exe\" ";
			arguments += L"\"";
			arguments += (wchar_t *)Param1.getUTF16StringPtr();
			//remove delimiter to avoid open-ended escape sequence
			if(arguments.at(arguments.size() - 1) == L'\\')
				arguments = arguments.substr(0, arguments.size() - 1);
			arguments += L"\"";
		
			arguments += L" -c ";
			arguments += configPath;
			std::vector<wchar_t> buf(32768);
			PA_Unichar *p = (PA_Unichar *)&buf[0];
			wchar_t *commandLine = (wchar_t *)p;
			p += arguments.copy((wchar_t *)p, arguments.size());

			CUTF16String spc((PA_Unichar *)L" ");

			for(uint32_t i = 0; i < countArgs; ++i)
			{
				CUTF16String v;
				Param2.copyUTF16StringAtIndex(&v, i);
				if(v.size())
				{
					p += spc.copy(p, 1);
					p += v.copy(p, v.size());
				}
			}
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESTDHANDLES | STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		ZeroMemory(&pi, sizeof(pi));
		if (CreateProcess(
				NULL,
				commandLine,
				NULL,
				NULL,
				FALSE,
				NULL,	//CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT
				NULL,	//pointer to the environment block for the new process
				currentDirectoryPath.c_str(),
				&si,
				&pi
				))
				{
					MIDI::taskSet(&pi, &si);
					returnValue.setIntValue(pi.dwProcessId);
				}
#else
			NSURL *bundleURL = [MIDI::pluginBundle bundleURL];
/*
			NSString *configPath = [[[[[bundleURL
			URLByAppendingPathComponent:@"Contents"]
			URLByAppendingPathComponent:@"Resources"]
			URLByAppendingPathComponent:@"timidity"]
			URLByAppendingPathComponent:@"timidity.cfg"]path];
 */
 			NSString *configPath = @"timidity.cfg";
		
 			NSString *currentDirectoryPath = [[[[bundleURL
			URLByAppendingPathComponent:@"Contents"]
			URLByAppendingPathComponent:@"Resources"]
			URLByAppendingPathComponent:@"timidity"]path];
		
			NSString *launchPath = [[[[bundleURL
			URLByAppendingPathComponent:@"Contents"]
			URLByAppendingPathComponent:@"MacOS"]
			URLByAppendingPathComponent:@"timidity"]path];

			NSString *path = Param1.copyPath();
			NSMutableArray *arguments = [[NSMutableArray alloc]initWithObjects:@"-c", configPath, path, nil];
		
			for(uint32_t i = 0; i < countArgs; ++i)
			{
				NSString *v = Param2.copyUTF16StringAtIndex(i);
				if([v length])
					[arguments addObject:v];
				[v release];
			}
		
			NSTask *task = MIDI::taskCreate();
		
			[task setCurrentDirectoryPath:currentDirectoryPath];
		
			[task setArguments: arguments];
		
			[arguments release];
		
			[task setLaunchPath:launchPath];
			[task launch];
#endif
	}

	returnValue.setReturn(pResult);
}

void MIDI_ABORT(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;

	Param1.fromParamAtIndex(pParams, 1);

	MIDI::taskDelete(Param1);
}

void MIDI_SUSPEND(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;

	Param1.fromParamAtIndex(pParams, 1);
	
	MIDI::taskSuspend(Param1);
}

void MIDI_RESUME(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;

	Param1.fromParamAtIndex(pParams, 1);

	MIDI::taskResume(Param1);
}

