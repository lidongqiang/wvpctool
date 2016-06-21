#include "stdio.h"
#include "stdint.h"
#include "FileIO.h"
#ifdef  WIN32

FIOH FILEW_open(const TCHAR *file_name, FILE_open_mode_enum open_mode)
{
    DWORD  dwAccessMode     = 0;
    DWORD  dwOpenMode       = 0; 
    if(file_name && *file_name) {
        if(file_name) {
            switch(open_mode) {
                case FILE_open_mode_read:
                    dwAccessMode = GENERIC_READ;
                    dwOpenMode   = OPEN_ALWAYS;
                    break;
                case FILE_open_mode_write_append:
                    dwAccessMode = GENERIC_WRITE;
                    dwOpenMode   = OPEN_ALWAYS;
                    break;
                case FILE_open_mode_write_replace:
                    dwAccessMode = GENERIC_WRITE;
                    dwOpenMode   = OPEN_ALWAYS;
                    break;
                case FILE_open_mode_write_create:
                    dwAccessMode = GENERIC_WRITE|GENERIC_READ;
                    dwOpenMode   = CREATE_ALWAYS;
                    break;
            }
        }
    }
    HANDLE in = CreateFile(file_name,dwAccessMode,0,NULL,dwOpenMode,FILE_ATTRIBUTE_NORMAL,NULL);
    if(INVALID_HANDLE_VALUE == in) return NULL;
    return in;

}


FILE_status_enum  FILEW_seek(FIOH hFile, uint32_t offset)
{
    FILE_status_enum    result  = FILE_status_ok;
    DWORD dwNewPos = SetFilePointer(hFile,offset,NULL,FILE_BEGIN);
    if((INVALID_SET_FILE_POINTER == dwNewPos)&&GetLastError() != NO_ERROR) {
        result = FILE_status_EOF;
    }
    return result;
}

FILE_status_enum  FILEW_write(FIOH hFile, uint8_t *mem_pointer, uint32_t length)
{
    FILE_status_enum    result                  = FILE_status_ok;
    BOOL                bRet                    = FALSE;
    DWORD               dwNumberOfBytesWritten  = 0;
    if(hFile) {
        bRet = WriteFile(hFile,(LPCVOID)mem_pointer,length,&dwNumberOfBytesWritten,NULL);
    }
    if((!bRet)||(dwNumberOfBytesWritten != length)) {
        result = FILE_status_write_access_error;
    }
    return result;
}
int32_t FILEW_pos(FIOH hFile)
{
#if 0
    if(handle)
        return ftell(handle);
#endif
    return 0;
}
FILE_status_enum  FILEW_read(FIOH hFile, uint8_t *mem_pointer, uint32_t length)
{
    FILE_status_enum    result              = FILE_status_ok;
    BOOL                bRet                = FALSE;
    DWORD               dwNumberOfBytesRead = 0;
    if(hFile && mem_pointer) {
        bRet = ReadFile(hFile,(LPVOID)mem_pointer,length,&dwNumberOfBytesRead,NULL);
    }
    if((!bRet)||(length != dwNumberOfBytesRead )) {
        result = FILE_status_EOF;
    }
    return result;
}


uint8_t FILEW_EOF(FIOH hFile)
{
#if 0
    if(in)
        return (feof(in) != 0);
    return true;
#else 
    return 0;
#endif
}
uint8_t FILEW_exists(const TCHAR *file_name)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFind;
    if(!file_name) return false;
    hFind = FindFirstFile(file_name, &FindFileData);
    if (INVALID_HANDLE_VALUE == hFind) {
        return false;
    }  else  {
        FindClose(hFind);
    }
    return true;
}
FILE_status_enum FILEW_close(FIOH hFile)
{
    FILE_status_enum  result = FILE_status_EOF;
    if(hFile && CloseHandle(hFile) != 0)
        result = FILE_status_ok;
    return result;
}

FILE_status_enum FILEW_flush(FIOH hFile)
{
    if(FlushFileBuffers(hFile)) 
        return FILE_status_ok;
    return FILE_status_EOF;
}
uint32_t FILEW_size(FIOH hFile)
{
    uint32_t length=0;
    if(hFile) {
        length = GetFileSize(hFile,NULL);
    } else {
        length = 0;
    }
  return length;
}
#endif
