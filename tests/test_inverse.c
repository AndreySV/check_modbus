#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "../src/variant.h"

int test_inverse(int format)
{
	data_t before, after;
	int size_words;
	int i;

	before.format = format;
	after.format  = format;
	
	size_words  = sizeof_data_t( &before ); /* sizeof_data_t returs size in words */
	for(i=0; i<size_words; i++ )
	{
		before.val.words[i] = i;
		after.val.words[size_words-i-1] = i;
	}

	reorder_data_t( &before, 0, 1 );
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
		if (!test_inverse( formats[i] )) return -1;
	}
	return 0;
}
