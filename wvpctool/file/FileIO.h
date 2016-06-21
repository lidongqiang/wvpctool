#ifndef DLL_FileIOH
#define DLL_FileIOH
typedef enum
{
  FILE_status_ok = 0,
  FILE_status_file_not_found,
  FILE_status_open_error,
  FILE_status_read_access_error,
  FILE_status_write_access_error,
  FILE_status_EOF
} FILE_status_enum;

typedef enum
{
  FILE_open_mode_read = 0,
  FILE_open_mode_write_append,
  FILE_open_mode_write_replace,
  FILE_open_mode_write_create  /* If file exists it will be reset to size 0 byte */
} FILE_open_mode_enum;

#ifdef __cplusplus
  extern "C" {
#endif
  extern          uint8_t   FILE_exists (const char *file_name);
  extern             FILE  *FILE_open   (const char *file_name, FILE_open_mode_enum open_mode);
  extern FILE_status_enum   FILE_close  (FILE *handle);
  extern         uint32_t   FILE_size   (FILE *handle);
  extern          int32_t   FILE_pos    (FILE *handle);
  extern          uint8_t   FILE_EOF    (FILE *in);
  extern FILE_status_enum   FILE_read   (FILE *handle, uint8_t *mem_pointer, uint32_t length);
  extern FILE_status_enum   FILE_write  (FILE *handle, uint8_t *mem_pointer, uint32_t length);
  extern FILE_status_enum   FILE_seek   (FILE *handle, uint32_t offset);
  extern             char  *FILE_ExtractFileName(char *filename);
  extern             char  *FILE_ExtractFileExt(char *filename);
  extern          uint8_t   FILE_ReadLine(FILE *in, char *dest, uint32_t BufferSize);
  extern             void   FILE_WriteLine(FILE *out, char *string);
  extern             char  *FILE_get_last_error(void);
  extern FILE_status_enum   FILE_flush(FILE *handle);
#ifdef __cplusplus
}
#endif

#ifdef  WIN32
#include <windows.h>
#include <TCHAR.H>
#define FIOH void*
#ifdef __cplusplus
  extern "C" {
#endif


extern          uint8_t FILEW_exists(const TCHAR *file_name);
extern             FIOH FILEW_open  (const TCHAR *file_name, FILE_open_mode_enum open_mode);
extern FILE_status_enum FILEW_seek  (FIOH hFile, uint32_t offset);
extern FILE_status_enum FILEW_write (FIOH hFile, uint8_t *mem_pointer, uint32_t length);
extern FILE_status_enum FILEW_read  (FIOH hFile, uint8_t *mem_pointer, uint32_t length);
extern          uint8_t FILEW_EOF   (FIOH hFile );
extern          int32_t FILEW_pos   (FIOH hFile );
extern FILE_status_enum FILEW_close (FIOH hFile );
extern FILE_status_enum FILEW_flush (FIOH hFile );
extern uint32_t         FILEW_size  (FIOH hFile);




#ifdef __cplusplus
}
#endif
#endif



#endif
