//---------------------------------------------------------------------------

#include <windows.h>
#include <iostream>
#include <tlhelp32.h>

#pragma hdrstop
#pragma argsused

#ifdef _WIN32
#include <tchar.h>
#else
  typedef char _TCHAR;
  #define _tmain main
#endif

//---------------------------------------------------------------------------
#include "UnitMain.h"
//---------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "XTerminator" << std::endl << std::endl;

	if(argc != 2)
	{
		std::cout << "Command Line Parameter Missed" << std::endl << std::endl;
		return -1;
	}

	std::string strApplication = argv[1];

	std::string strMsg = strApplication;
	strMsg.append(" will be Terminated");
	int ExitCode = MessageBox(NULL, "Do you want to proceed?", strMsg.c_str(), MB_YESNO|MB_ICONWARNING);
	if(ExitCode == IDNO)
		return -1;

	std::cout << "Termination in progress..." << std::endl << std::endl;

	int intTerminated = 0;
	int intNotTerminated = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if(Process32First(snapshot, &entry) == TRUE)
	{
		int intConsecutiveNumber = 0;

		while(Process32Next(snapshot, &entry) == TRUE)
		{
			if(stricmp(entry.szExeFile, strApplication.c_str()) == 0)
			{
				intConsecutiveNumber++;
				std::cout << "#" << intConsecutiveNumber << " termination...";

				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				TerminateProcess(hProcess, 0);
				DWORD dwResult = WaitForSingleObject(hProcess, INT_WAIT_FOR_TERMINATION);
				if(dwResult == WAIT_OBJECT_0)
				{
					intTerminated++;
					std::cout << " completed" << std::endl << std::endl;
				}
				else
				{
					intNotTerminated++;
					std::cout << " not completed in time" << std::endl << std::endl;
				}

				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);

	std::cout << "Terminated/Not terminated: " << intTerminated << "/" << intNotTerminated << std::endl;

	system("pause");
	return 0;
}
//---------------------------------------------------------------------------
//#pragma package(smart_init)
