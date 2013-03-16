#include <stdint.h>
#include <stdio.h>
#include "variant.h"

/* returns size in words */
int     sizeof_data_t(data_t* data)
{
    int size = 0;
    switch( data->format )
    {
        case FORMAT_DUMP_BIN:
        case FORMAT_DUMP_HEX:
        case FORMAT_DUMP_DEC:
            size = data->arr_size/2;
            break;
        case FORMAT_SIGNED_WORD:
        case FORMAT_UNSIGNED_WORD:
            size = 1;
            break;
        case FORMAT_FLOAT:
        case FORMAT_SIGNED_DWORD:
        case FORMAT_UNSIGNED_DWORD:
            size = 2;
            break;
        case FORMAT_DOUBLE:
        case FORMAT_SIGNED_QWORD:
        case FORMAT_UNSIGNED_QWORD:
            size = 4;
            break;
        default:
            printf("Sizeof_data_t(): Unsupported format (%d)\n", data->format);
            break;
    }
    return size;
}



void    clear_data_t(data_t* data)
{
    int i;
    for(i=0;i<sizeof(data->val);i++)
    {
        data->val.bytes[i] = 0;
    }
}



void    init_data_t(data_t* data, int8_t format,uint8_t size)
{
    clear_data_t( data );
    data->format = format;
    data->arr_size = size;
}



double  value_data_t(data_t*  data)
{
    double tmp;
    switch( data->format )
    {
        case FORMAT_SIGNED_WORD:
            tmp = data->val.sword;
            break;
        case FORMAT_UNSIGNED_WORD:
            tmp = data->val.word;
            break;
        case FORMAT_SIGNED_DWORD:
            tmp = data->val.sdword;
            break;
        case FORMAT_UNSIGNED_DWORD:
            tmp = data->val.dword;
            break;
        case FORMAT_SIGNED_QWORD:
            tmp = data->val.sqword;
            break;
        case FORMAT_UNSIGNED_QWORD:
            tmp = data->val.qword;
            break;
        case FORMAT_FLOAT:
            tmp = data->val.real;
            break;
        default:
            tmp = 0;
    }
    return tmp;
}

void    printf_data_t(data_t* data)
{
    int size = 0;
    int i;
    switch( data->format )
    {
        case FORMAT_SIGNED_WORD:
            printf("%d", data->val.word);
            break;
        case FORMAT_UNSIGNED_WORD:
            printf("%u", data->val.word);
            break;
        case FORMAT_SIGNED_DWORD:
            printf("%ld", data->val.sdword);
            break;
        case FORMAT_UNSIGNED_DWORD:
            printf("%lu", data->val.dword);
            break;
        case FORMAT_SIGNED_QWORD:
            printf("%lld", data->val.sqword);
            break;
        case FORMAT_UNSIGNED_QWORD:
            printf("%llu", data->val.qword);
            break;
        case FORMAT_FLOAT:
            printf("%f", data->val.real);
            break;
        case FORMAT_DOUBLE:
            printf("%Lf", data->val.long_real);
            break;
        case FORMAT_DUMP_BIN:
            fwrite( data->val.bytes, 1, data->arr_size, stdout);
            break;
        case FORMAT_DUMP_HEX:
            for( i=0; i<data->arr_size;)
            {
                printf("%X ", data->val.bytes[i++]);
                if ( (i%16) == 0) printf("\n");
            }
            printf("\n");
            break;
        case FORMAT_DUMP_DEC:
            for( i=0; i<data->arr_size;)
            {
                printf("%d ", data->val.bytes[i++]);
                if ( (i%16) == 0) printf("\n");
            }
            printf("\n");
            break;

        default:
            printf("Printf_data_t(): Unsupported format (%d)\n", data->format);
    }
}

uint16_t    swap_bytes(uint16_t word)
{
    return ((word & 0xFF00)>>8) | ((word & 0x00FF)<<8);
}


void    reorder_data_t(data_t* data, int swap, int inverse_words)
{
    int         size = sizeof_data_t( data );
    int         i, j;
    uint16_t    word;
    data_t      tmp;

    tmp  = *data;
    for( i = 0; i<size; i++)
    {
        j = inverse_words ? size-i-1 : i;
        word = tmp.val.words[j];
        data->val.words[i] = swap ? swap_bytes(word) : word;
    }

}

int     equal_data_t(data_t* data1,data_t* data2)
{
    int         size = sizeof_data_t( data1 );
    int         i;

    if ( data1->format != data2->format ) return 0;

    for( i=0; i<size; i++)
        if (data1->val.bytes[i] != data2->val.bytes[i]) return 0;
    
    return 1;
}
