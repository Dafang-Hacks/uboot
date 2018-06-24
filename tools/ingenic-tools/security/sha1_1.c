#include<stdio.h>
#include<string.h>

//#include "splsrc28k.h"
//#include "splsrc14k.h"
//#include "splsrc16k.h"

#define FILE_NUM 1
#define FILE_MAX_LEN 38672


int do_sha1_1(int *pbuf, int plen)
{
  unsigned int i, r, j;
  unsigned int length;
  char file_name[32];
  unsigned int RAND_SEED = 0 ;
  srand( RAND_SEED );
  char string;
  
  for( i=0; i<FILE_NUM; i++)
  {
        if ( (i%100) == 0 ){
                RAND_SEED ++;
                //printf("RAND_SEED = %d\n",RAND_SEED);
                srand ( RAND_SEED ) ;
        }
	length = plen;
	//printf("length %d\n", length);
        sprintf( file_name, "./file%04d.hex", i);
        //printf ("file_name: %s\n", file_name);

        FILE *fp;
        fp=fopen (file_name, "w");

#if 0
        /*      random int to string    */
        for (j=0; j<length; j++)
        {
                r = rand() & 0xff;
                //printf ("r=%d\n", r);
                string = (char) r;
		fprintf(fp,"%c",string);
        }
#else
	for (j = 0; j < length/4; j++){
		unsigned int *p = pbuf;
		r = p[j] & 0xFF;
		string = (char)r;
		fprintf(fp,"%c",string);

		r = (p[j]>>8) & 0xFF;
		string = (char)r;
		fprintf(fp,"%c",string);

		r = (p[j]>>16) & 0xFF;
		string = (char)r;
		fprintf(fp,"%c",string);

		r = (p[j]>>24) & 0xFF;
		string = (char)r;
		fprintf(fp,"%c",string);
	}
#endif
        //printf("string_length(byte)=%d\n",length);
	fclose(fp);
   }
   return 0;
}
