/*
 * Adithya Bhat <bhat24@purdue.edu>
 * 2019
 * */

#include "log.h"
#include <stdio.h>

void
PrintBuffer(unsigned char* buffer, size_t size)
{
	printf("0x");
	for ( size_t i = 0 ; i < size ; i++ ) {
		printf ( "%02x" , buffer [ i ] );
	}
	printf( "\n" );
}
