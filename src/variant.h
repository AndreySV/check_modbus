#ifndef _VARIANT_H_
#define _VARIANT_H_

#include <stdint.h>

typedef struct 
{
    union
    {
        uint8_t     bytes[8];
        uint16_t    words[4];

        uint8_t     byte;
        int8_t      sbyte;

        uint16_t    word;
        int16_t     sword;

        uint32_t    dword;
        int32_t     sdword;

        uint64_t    qword;
        int64_t     sqword;

        float       real;
        double      long_real;
    } val;
    int8_t      format;
} data_t;


enum
{
    FORMAT_MIN_SUPPORTED= 0,
    FORMAT_SIGNED_WORD,
    FORMAT_UNSIGNED_WORD,
    FORMAT_SIGNED_DWORD,
    FORMAT_UNSIGNED_DWORD,
    FORMAT_SIGNED_QWORD,
    FORMAT_UNSIGNED_QWORD,
    FORMAT_FLOAT,
    FORMAT_DOUBLE,
    FORMAT_MAX_SUPPORTED
};



int     sizeof_data_t(data_t* data);    /* returns size in words */
void    printf_data_t(data_t* data);
double  value_data_t(data_t*  data);
void    init_data_t(data_t* data, int8_t format);
void    clear_data_t(data_t* data);
void    reorder_data_t(data_t* data, int swap_bytes, int inverse_words);

#endif

