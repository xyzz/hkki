#include "compress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef unsigned char byte;


int decompress_block(byte* buf, FILE* f, byte mask)
{
    int i, j, c=0;

    for(i=7; i >= 0; --i){
	uint16_t packedref = 0, offset = 0;
	byte num = 0;
	byte iscomp = (mask>>i)&1;

	if( !iscomp ){
	    fread(&buf[c++], 1, 1, f);
	    continue;
	}

	if( 0 == fread(&packedref, 2, 1, f) )
	    return 0;
	if( 0 == (packedref & 0xf0) ){
	    num = 16 * ((packedref&0xff)+1);
	    fseek(f, -1, SEEK_CUR);
	    fread(&packedref, 2, 1, f);
	}

	num    += ((packedref >> 4) & 0xF) + 1;
	offset = ( ((packedref & 0xf) <<8) | (packedref >> 8)  ) + 1;

	/*
	printf("fpos: 0x%X packedref: 0x%04X num %d, offset: 0x%04X, mask: 0x%02X\n", 
	       ftell(f), packedref, num, offset, mask);
	*/
	for(j=0; j<num; j++, c++)
	    buf[c] = buf[ c-offset ];

    }

    return c;
}

/*** XXX
     - check header for correctness
     - check for compression type
     - generally more checks
***/
int decompress(char* from, char* to)
{
    FILE* tf;
    FILE* ff = fopen(from, "rb");
    int decompsize = 0;
    int count = 8;
    byte* buf = NULL;

    if( NULL == ff )
	return 1;

    fseek(ff, 0x1, SEEK_SET); /* skip the header */
    fread(&decompsize, 4, 1, ff);
    buf = (byte*)malloc( decompsize+1 );
    if( NULL == buf ){
	fclose(ff);
	return 1;
    }
    memset(buf, 0, decompsize);
    
    fread(buf, 8, 1, ff);
    
    while( count < decompsize ){
	 byte mask;
	 int blockc;
	 fread(&mask, 1, 1, ff);
	 blockc = decompress_block(&buf[count], ff, mask); 
	 if( 0 == blockc )
	     break;

	 count += blockc;
	 //	printf("count: 0x%X\n", count);
     }
     //    printf("count: 0x%X, decompsize: 0x%X\n", count, decompsize);
     fclose(ff);


     tf = fopen(to, "wb");
     if( NULL == tf )
	 return 1;
     fwrite(buf, 1, decompsize, tf);
     free(buf);
     fclose(tf);
     //printf("Written decompressed file to %s\n", to);

     return 0;
 }


 int find_in_window(byte* str, byte* dict, int end, 
		    int* len, int* offset)
 {
     static const int threshold = 3;
     *len = 0;
     *offset = 0;

     int i = end-1;
     while( i >= 0 && (end-i) < 0xfff ){
	 int j = 0;
	 while( str[j] == dict[i+j] )
	     j += 1;

	 if( j >= threshold && ((j-1)/16) < 16 && j > *len){
	     *len = j - 1;
	     *offset = end - i - 1;
	 }

	 i -= 1;
     }

     if( *len )
	 return 0;

     return 1;
 }



int compress_block(byte* buf, int* bufsize, int* count, 
		    byte* src, int* srcpos)
{
    byte mask = 0;
    int maskpos = (*count)++;
    int i;
    
    for(i=0; i<8; ++i){
	int len = 0, offset = 0;
	byte* tmp = &src[ *srcpos ];
	
	if( !find_in_window(tmp, src, *srcpos, &len, &offset) ){
	    uint16_t packedref = 0;
	    packedref |= (offset & 0xf00) >> 8;
	    packedref |= (offset & 0xff ) << 8;

	    *srcpos += len+1;

	    if( len >= 16 ){
		byte multiplier = (len / 16)-1;
		len = len % 16;
		buf[ (*count)++ ] = multiplier;
	    }	    
	    packedref |= len << 4;
	    *((uint16_t*)&buf[ *count ]) = packedref;
	    *count += 2;

	    mask |= 1 << (7-i);
	}else{
	    buf[ (*count)++ ] = *tmp;
	    *srcpos += 1;
	}
    }
    
    buf[ maskpos ] = mask;

    return 0;
}


int compress(char* from, char* to)
{
    static const byte header[] = 
	{0x11};
    static const int max_block_size = 8*3 + 1;
    static const int alloc_size = 0x200;

    FILE* tf = NULL;
    FILE* ff = fopen(from, "rb");
    int count = 0, bufsize = 0;
    int frompos = 0, fromlen = 0;
    byte* buf = NULL;
    byte* frombuf;
    
    if( NULL == ff ){
	free(buf);
	return 1;
    }

    fseek(ff, 0, SEEK_END);
    fromlen = ftell(ff);
    fseek(ff, 0, SEEK_SET);
    frombuf = (byte*)malloc(fromlen);
    if( NULL == frombuf ){
	fclose(ff);
	free(buf);
	return 1;
    }
    fread(frombuf, 1, fromlen, ff);
    fclose(ff);

    do{
	if( bufsize - count < max_block_size ){
	    bufsize += alloc_size;
	    buf = (byte*)realloc(buf, bufsize);
	    if( NULL == buf ){
		free(frombuf);
		free(buf);
		return 1;
	    }
	}

	compress_block(buf, &bufsize, &count, 
		       frombuf, &frompos);
    }while( frompos < fromlen );
    free(frombuf);

    tf = fopen(to, "wb");
    if( NULL == tf ){
	free(buf);
	return 1;
    }
    fwrite(header, 1, sizeof(header), tf);
    fwrite(&fromlen, 4, 1, tf);
    fwrite(&buf[1], 1, count-1, tf);
    fclose(tf);

    return 0;
}
