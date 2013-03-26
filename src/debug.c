#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

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
              tm_time->tm_year + 1900,                                   \
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

void save_dump_file_to_debug_log(FILE* fd, FILE* dump)
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
    
    fprintf( fd, "Dump of file:\n");
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



int  is_process_directory(struct dirent* d)
{
    char* p = NULL;
    long  id;
    
    if (d->d_type != DT_DIR )    return 0;

    id = strtol( d->d_name, &p, 10);

    if ( *p !='\0' ) return 0;
        
    return id;
}



int get_state_of_process(FILE* fout, int id)
{
    const char find_str[]="State:";
    char str[255];
    char* res;
    FILE* fd;
    
    if (!id) return 0;

    snprintf( str, sizeof(str), "/proc/%d/status", id);
    fd = fopen( str, "rt");
    if (!fd) return 0;

    do
    {
        res = fgets( str, sizeof(str), fd);
        if (res)
        {
            char* found;
            found = strstr( str, find_str );
            if (found)
            {
                fprintf( fout, "%s\n", str );
                break;
            }
        }
    }
    while( res );
    fclose( fd );
    
    fprintf( fout, "\n");
    
    return 0;
}


int is_check_modbus_process(FILE* fout, int id)
{
    char str[255];
    FILE* fd;
    char* res;
    int   ret;
    int   ch;

    ret = 0;
    if (!id) return ret;


    snprintf( str, sizeof(str), "/proc/%d/cmdline", id);
    fd = fopen( str, "rt");
    if (!fd) return ret;

    res = fgets( str, sizeof(str), fd);
    if (res)
    {
        const char ps_name[]="check_modbus";
        res = strstr( str, ps_name );
        if (res)
        {
            // check_modbus process is found
            fprintf( fout, "---------------------------\n");
            fprintf( fout, "Command: \t");

            fseek( fd, 0, SEEK_SET );
            do
            {
                ch = fgetc( fd );
                if (ch != EOF)
                {
                    if (!ch) ch=' ';
                    fprintf( fout, "%c", (char)ch );
                }
            }
            while( ch != EOF );

            fprintf( fout, "\n");
            fprintf( fout, "PID %d\n", id );

            get_state_of_process(fout, id);
                
            ret = 1;
        }
    }
    fclose(fd);
    return ret;
}

void get_pids_of_other_processes(FILE* fd)
{
    int  id;
    DIR* d;
    struct dirent* dir;

    d = opendir("/proc");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            id = is_process_directory( dir );
            
            is_check_modbus_process( fd, id );
        }
    }
    else
    {
        fprintf( fd, "Can't open /proc directory\n");
    }
}



        
void save_debug_information(modbus_params_t* params,FILE* infile, int read, int size)
{
    FILE* fd;

    fd = open_file();
    /* fd = stdout; */
    if (!fd) return;

    fprintf(fd, "Command line settings:\n");
    print_settings( fd, params );

    /* save information about dump file */
    fprintf( fd, "\n" );
    fprintf( fd, "\n" );
    fprintf( fd, "---------------------------\n");
    fprintf( fd, "read: %d\n", read);
    fprintf( fd, "need: %d\n", size);
    fprintf( fd, "FILE pointer: %p\n", infile);
    fprintf( fd, "current file position: %ld\n", ftell( infile ) );
    fprintf( fd, "\n");
    

    save_dump_file_to_debug_log( fd, infile );

    /* save information about pid of current process */
    fprintf( fd, "---------------------------\n");
    fprintf( fd, "pid of current process: %d\n", getpid() );

    
   /* save information about other running check_modbus processes  */
    get_pids_of_other_processes( fd );
    
    fprintf( fd, "---------------------------\n");
    fclose(fd);

    return;
}


