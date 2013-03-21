#include <stdio.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "command_line.h"


FILE* open_file(void)
{
    const  int str_len = 50;

    FILE*  fd;
    struct tm* tm_time;
    time_t t;
    pid_t  pid;
    char   file_name[str_len];
    
    t=time(NULL);
    tm_time =  localtime( &t );

    pid = getpid();

    snprintf( file_name, str_len,                                        \
              "/tmp/%d_%d_%d-%d_%d_%d-%d-debug.log",                     \
              tm_time->tm_year,                                          \
              tm_time->tm_mon,                                           \
              tm_time->tm_mday,                                          \
              tm_time->tm_hour,                                          \
              tm_time->tm_min,                                           \
              tm_time->tm_sec,                                           \
              pid                                                        \
        );

    fd = fopen(file_name, "wb");
    if (!fd)
    {
        fprintf( stderr, "Can't create file with debug information\n");
    }
    return fd;
}

void save_dump_file(FILE* fd, FILE* dump)
{
    int rc;
    int cnt;
    uint8_t byte;
    size_t  read_bytes;
    
    if (!dump)
    {
        fprintf( stderr, "dump file is not opened\n");
        return;
    }
    
    fprintf( fd, "---------------------------\n");

    clearerr( dump );
    rc = fseek( dump, 0, SEEK_SET );
    if (rc)
    {
        fprintf( fd, "fseek() failed %d, errno: %d (%s)\n", rc , errno, strerror( errno ) );
        return;
    }

    for( cnt=0; ; cnt++ )
    {
        read_bytes = fread( &byte, 1, 1, dump);
        if (read_bytes) fwrite( &byte, 1, 1, fd);
        else
        {
            if (rc = feof( dump ))
            {
                fprintf( fd, "\n----------------------------\n");
                fprintf( fd, "End of dump file (%d)\n", rc);
                break;
            }
               
            if ( rc = ferror( dump) )
            {
                fprintf( fd, "\n----------------------------\n");
                fprintf( fd, "Error (%d)\n", rc );
                fprintf( fd, "errno: %d (%s)\n", errno, strerror( errno ) );
                break;
            }
        }
    }

    fprintf( fd, "Read %d bytes from dump file\n", cnt);
    fprintf( fd, "\n");
    
    return; 
}


void get_pids_of_other_processes(FILE* fd)
{
}

        
void save_debug_information(modbus_params_t* params,FILE* infile, int read, int size)
{
    FILE* fd;

    fd = open_file();
    if (!fd) return;

    print_settings( fd, params );

    /* save information about dump file */
    fprintf( fd, "---------------------------\n");
    fprintf( fd, "read: %d\n", read);
    fprintf( fd, "written: %d\n", size);
    fprintf( fd, "FILE pointer: %p\n", infile);
    fprintf( fd, "current file position: %ld\n", ftell( infile ) );
             

    save_dump_file( fd, infile );
    
    get_pids_of_other_processes();
    
    fprintf( fd, "---------------------------\n");
    fclose(fd);
    return;
}


