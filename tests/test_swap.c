#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "../src/variant.h"

int test_swap(int format)
{
	data_t before, after;
	int size_bytes;
	int i;

	before.format = format;
	after.format  = format;
	
	size_bytes = 2*sizeof_data_t( &before ); /* sizeof_data_t returns size in words */
	for(i=0; i<size_bytes; i+=2 )
	{
		before.val.bytes[i] = i;
		before.val.bytes[i+1] = i+1;


		after.val.bytes[i] = i+1;
		after.val.bytes[i+1] = i;
	}

	reorder_data_t( &before, 1, 0);
	// check results
	return  equal_data_t( &before, &after );
}


int main(void)
{
	int rc;
	int i;
	int formats[]=
		{
			FORMAT_SIGNED_WORD,
			FORMAT_SIGNED_DWORD,
			FORMAT_SIGNED_QWORD
		};

	for(i=0; i<sizeof(formats)/sizeof(formats[0]); i++)
	{
		if (!test_swap( formats[i] )) return -1;
	}
	return 0;
}
