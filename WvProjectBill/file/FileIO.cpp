#include "stdio.h"
#include "stdint.h"
#include "FileIO.h"
FILE  *FILE_open(const char *file_name, FILE_open_mode_enum open_mode)
{
    FILE *in = NULL;
    if(file_name && *file_name) {
        if(file_name) {
            switch(open_mode) {
                case FILE_open_mode_read:
                    in = fopen(file_name, "rb");
                    break;
                case FILE_open_mode_write_append:
                    in = fopen(file_name, "a+b");
                    break;
                case FILE_open_mode_write_replace:
                    in = fopen(file_name, "w+b");
                    break;
                case FILE_open_mode_write_create:
                    if(FILE_exists(file_name))
                    (void)remove(file_name);
                    in = fopen(file_name, "wb");
                    break;
            }
        }
    }
    return in;
}

FILE_status_enum  FILE_seek(FILE *handle, uint32_t offset)
{
    FILE_status_enum    result  = FILE_status_EOF;
    int                 res     = 0;
    if(handle) {
        res &= fseek(handle, offset, SEEK_SET);
        if(handle && (res == 0)) {
            result = FILE_status_ok;
        }
    }
    return result;
}

FILE_status_enum  FILE_write(FILE *handle, uint8_t *mem_pointer, uint32_t length)
{
    FILE_status_enum    result  = FILE_status_write_access_error;
    uint32_t            n       = 0;
    if(handle) {
        n = fwrite(mem_pointer, 1, length, handle);
    }
    if(n == length)
        result = FILE_status_ok;
    return result;
}
int32_t FILE_pos(FILE *handle)
{
    if(handle)
        return ftell(handle);
    return 0;
}
FILE_status_enum  FILE_read(FILE *handle, uint8_t *mem_pointer, uint32_t length)
{
    FILE_status_enum    result  = FILE_status_EOF;
    uint32_t            n       = 0;
    if(handle && mem_pointer) {
        n = fread(mem_pointer, 1, length, handle);
    }
    if(n == length)
        result = FILE_status_ok;
    return result;
}
uint8_t FILE_EOF(FILE *in)
{
    if(in)
        return (feof(in) != 0);
    return true;
}
uint8_t FILE_exists(const char *file_name)
{
    uint8_t result = false;
    FILE *in;
    if(file_name && ((in = fopen(file_name, "rb")) != NULL)) {
        result = true;
        fclose(in);
    }
    return result;
}
FILE_status_enum FILE_close(FILE *handle)
{
    FILE_status_enum  result = FILE_status_EOF;
    if(handle && fclose(handle) == 0)
        result = FILE_status_ok;
    return result;
}
uint32_t FILE_size(FILE *handle)
{
    uint32_t curpos;
    uint32_t length=0;
    if(handle) {
        curpos = ftell(handle);
        fseek(handle, 0L, SEEK_END);
        length = ftell(handle);
        fseek(handle, curpos, SEEK_SET);
    } else {
        length = 0;
    }
  return length;
}

