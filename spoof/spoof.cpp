// spoof.cpp : This file contains the 'main' function. Program execution begins and ends there.
// A slightly adjusted version of https://blog.didierstevens.com/2009/11/22/quickpost-selectmyparent-or-playing-with-the-windows-process-tree/

#include "pch.h"
#include <windows.h>
#include <iostream>

BOOL CurrentProcessAdjustToken(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES sTP;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sTP.Privileges[0].Luid))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		sTP.PrivilegeCount = 1;
		sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		CloseHandle(hToken);
		return TRUE;
	}
	return FALSE;
}


int main(int argc, const char * argv[])
{


	STARTUPINFOEXA sie = { sizeof(sie) };
	PROCESS_INFORMATION pi;
	SIZE_T cbAttributeListSize = 0;
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = NULL;
	HANDLE hParentProcess = NULL;
	DWORD dwPid = 0;


	if (argc != 4)
		std::cout << "usage: SelectMyParent program pid showWindow\n";
	else
	{
		dwPid = atoi(argv[2]);
		if (0 == dwPid)
		{
			std::cout << "Invalid pid\n";
			return 1;
		}
		InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize);
		pAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, cbAttributeListSize);
		if (NULL == pAttributeList)
		{
			std::wcout << "HeapAlloc error\n";
			return 2;
		}
		if (!InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize))
		{
			std::wcout << "InitializeProcThreatAttributeList error\n";
			return 3;
		}
		CurrentProcessAdjustToken();
		hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (NULL == hParentProcess)
		{
			std::wcout << "OpenProcess error\n";
			return 4;
		}
		if (!UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProcess, sizeof(HANDLE), NULL, NULL))
		{
			std::wcout << "UpdateProcThreadAttribute error\n";
			return 5;
		}
		sie.lpAttributeList = pAttributeList;
		sie.StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		sie.StartupInfo.wShowWindow = atoi(argv[3]);
		if (!CreateProcessA(NULL, (LPSTR)argv[1], NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT | CREATE_NEW_CONSOLE, NULL, NULL, &sie.StartupInfo, &pi))
		{
			std::wcout << "CreateProcess error\n";
			return 6;
		}
		printf("Process created: %d\n", pi.dwProcessId);

		//WaitForSingleObject(pi.hProcess, INFINITE);
		
		DeleteProcThreadAttributeList(pAttributeList);
		CloseHandle(hParentProcess);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
