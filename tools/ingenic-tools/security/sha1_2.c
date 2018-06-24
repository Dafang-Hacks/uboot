/*	Secure Hash Algorithm	SHA-1		*/
/*	2013.7.23	Ryan			*/
/*	Max message length: 2^64-1 bits		*/
/*	Block length: 512 bits			*/
/*	Digest length: 160 bits			*/
/*      Round: 80                               */
/*	hex file making				*/
/*	File output: sha1_file0000.hex		*/

#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#define DISPLAY			//	information display
#define rol(n,x) ( ((x) << (n)) | ((x) >> (32-(n))) )	//	Rotate a 32 bit integer by n bits
#define fn 1
#define RAND_SEED 6
typedef struct
{
int index;
uint32_t len_low, len_high;
uint32_t h0,h1,h2,h3,h4;
unsigned int mes[64];
}message;

char ADDR_LENGTH[] = "@00000000";
char ADDR_PADDING[] = "@00000004";
char ADDR_SHA1[] = "@08000000";

void sha1_transform( message * );
void sha1_init( message * );
void sha1_input( char *,char *, message * );
void sha1_pad( char *,message * );
void sha1_core( char *, char * , int *);

unsigned int K[4] = {
0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
};

void sha1_init( message *hd ) 	//	initial 
{
int i;
hd->index = 0;
hd->len_low = 0;
hd->len_high = 0; 
hd->h0 = 0x67452301; 
hd->h1 = 0xefcdab89; 
hd->h2 = 0x98badcfe; 
hd->h3 = 0x10325476; 
hd->h4 = 0xc3d2e1f0;
for(i=0;i<64;i++)
	hd -> mes[i] = 0; 
} 

void sha1_input( char infilename[],char *outfilename,  message *hd )	//	message input
{
FILE *infp;
FILE *outfp;
infp = fopen(infilename,"rb+");
outfp = fopen(outfilename,"w");
int i,j=0,k,m;
	fprintf(outfp,"%s\n",ADDR_PADDING);
	fseek(infp,0,2);
	k=ftell(infp);
	//printf("length=%d\n",k);
	rewind(infp);
	for(m=0;m<k;m++){
	//while( !feof(infp) ){
		fread( &hd->mes[ hd->index ], 1, 1, infp);
		//printf("mes=%02x, index=%d\n",hd->mes[ hd->index ],hd->index);
/*
		if( hd->mes[ hd->index ] == 0x0a){
			if(hd->index)
                		hd->mes[ hd->index ] = 0;
			hd->index = hd->index - 1;
			}

		else
                */hd->len_low += 8;
		hd->index ++;
		if( hd->len_low == 0)
			hd->len_high ++;
		if( (hd->index) == 64 ){
			#ifdef DISPLAYn
			for( i=0; i<16; i++){
				printf("0x");
                		for( j=0; j<4; j++)
                        		printf("%02x",hd->mes[i*4+3-j]);
                		printf(",\n");
			}
			#endif
			for(i=0;i<64;i++)
				fprintf(outfp,"%02x\n",hd->mes[i]);
			sha1_transform( hd );
		}
		//printf("  mes=%02x, index=%d\n",hd->mes[ hd->index ],hd->index);
	}
	/* #ifdef DISPLAY */
	/* for( i=0; i<16; i++){ */
        /*                         for( j=0; j<4; j++) */
        /*                                 printf("%02x",hd->mes[i*4+j]); */
        /*                         printf("\n"); */
        /*                 } */
	/* #endif */
	fclose(infp);
	fclose(outfp);
	//hd->len_low = hd->len_low-16;
}


void sha1_pad( char *outfilename,message *hd )	//	message padding
{
FILE *outfp;
outfp = fopen(outfilename,"a");
int i,j;
	if( hd->index > 55 ){
		hd->mes[ hd->index++ ] = 128;
		while ( hd->index < 64 ){
			hd->mes[ hd->index++ ] = 0;
		}
		#ifdef DISPLAYn
		for( i=0; i<16; i++){
			printf("0x");
                	for( j=0; j<4; j++)
                        	printf("%02x",hd->mes[i*4+3-j]);
                        printf(",\n");
                }
		#endif
		for( i=0; i<64; i++ ){
                fprintf(outfp,"%02x\n",hd->mes[i ]);
        	}
		sha1_transform( hd );
	//	while( hd->index < 56 ){
	//		hd->mes[ hd->index++ ] = 0;
	//	}
	}
	else{
		hd->mes[ hd->index++ ] = 128;
		while( hd->index < 56 ){
			hd->mes[ hd->index++ ] = 0;
		}
	}
	//hd->mes[56] = ( hd->len_high >> 24 ) & 0xff;
        //hd->mes[57] = ( hd->len_high >> 16 ) & 0xff;
        //hd->mes[58] = ( hd->len_high >> 8 ) & 0xff;
        //hd->mes[59] = ( hd->len_high ) & 0xff;
	hd->mes[56] = 0x00;
        hd->mes[57] = 0x00;
        hd->mes[58] = 0x00;
        hd->mes[59] = 0x00;
	hd->mes[60] = ( hd->len_low >> 24 ) & 0xff;
        hd->mes[61] = ( hd->len_low >> 16 ) & 0xff;
        hd->mes[62] = ( hd->len_low >> 8 ) & 0xff;
        hd->mes[63] = ( hd->len_low ) & 0xff;	
	for( i=0; i<64; i++ ){
		fprintf(outfp,"%02x\n",hd->mes[i ]);
	}
	fclose(outfp);
	#ifdef DISPLAYn
	for( i=0; i<16; i++){
		printf("0x");
     		for( j=0; j<4; j++)
                     		printf("%02x",hd->mes[i*4+3-j]);
               		printf(",\n");
	}
	#endif
	sha1_transform( hd );
	
}

void sha1_transform( message *hd ) //  SHA algorithm transform
{ 
uint32_t A, B, C, D, E, temp; 
uint32_t W[80];
int i, t;

A = hd->h0; 
B = hd->h1; 
C = hd->h2; 
D = hd->h3; 
E = hd->h4; 
//	Set 80 words start	//
for( t = 0; t < 16; t++ ){	
        W[t] = hd->mes[t * 4] << 24;
        W[t] |= hd->mes[t * 4 + 1] << 16;
        W[t] |= hd->mes[t * 4 + 2] << 8;
        W[t] |= hd->mes[t * 4 + 3];
        }
for( t = 16; t < 80; t++ ){
        W[t] = rol(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
        }
//	Set 80 words end	//
//	80 rounds begin		//
for( t = 0; t < 20; t++ ){
        temp = rol(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = rol(30,B);
        B = A;
        A = temp;
        }
for( t = 20; t < 40; t++ ){
        temp = rol(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = rol(30,B);
        B = A;
        A = temp;
        }
for( t = 40; t < 60; t++ ){
        temp = rol(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = rol(30,B);
        B = A;
        A = temp;
        }
for( t = 60; t < 80; t++ ){
        temp = rol(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = rol(30,B);
        B = A;
        A = temp;
        }
//	80 rounds end		//
hd->h0 = hd->h0 + A; 
hd->h1 = hd->h1 + B; 
hd->h2 = hd->h2 + C; 
hd->h3 = hd->h3 + D; 
hd->h4 = hd->h4 + E; 
hd->index = 0;
for( i=0; i<64; i++){
        hd->mes[i] = 0;
} 
}
void sha1_core( char infilename[], char outfilename[], int *pbuf)
{
int i,blk_num,byte_num;
message infor;
FILE *outfp;
	byte_num=0;
	blk_num=0;
	sha1_init( &infor );
	sha1_input( infilename, outfilename,&infor );
	sha1_pad( outfilename,&infor );
	//printf("length low=%d\n",infor.len_low);
        //printf("length high=%d\n",infor.len_high);
	//byte_num=infor.len_low/8+infor.len_high*536870912;
	byte_num=infor.len_low/8;
	//printf("byte_num=%d\n",byte_num);
	blk_num=(byte_num/64+(byte_num%64)/56+1)*64;
	//printf("blk_num=%x\n",blk_num);
	#ifdef DISPLAY
	//printf("Result :\n\t");
	//printf("%08X",infor.h0);
	//printf("%08X",infor.h1);
	//printf("%08X",infor.h2);
	//printf("%08X",infor.h3);
	//printf("%08X",infor.h4);
	//printf("\n");
	#endif
	outfp = fopen(outfilename,"a");
	fprintf(outfp,"%s\n",ADDR_SHA1);
	fprintf(outfp,"%02x\n",(infor.h0<<24)>>24);
	fprintf(outfp,"%02x\n",(infor.h0<<16)>>24);
	fprintf(outfp,"%02x\n",(infor.h0<<8)>>24);
	fprintf(outfp,"%02x\n",infor.h0>>24);

	//printf("SHA1 output:\n");
	pbuf[0] = infor.h0;
	pbuf[1] = infor.h1;
	pbuf[2] = infor.h2;
	pbuf[3] = infor.h3;
	pbuf[4] = infor.h4;
	//for (i = 0; i < 5; i++)
	//printf("sha %d 0x%08x\n", i, pbuf[i]);

	//printf("0x%02x",infor.h0>>24);
	//printf("%02x",(infor.h0<<8)>>24);
	//printf("%02x",(infor.h0<<16)>>24);
	//printf("%02x\n",(infor.h0<<24)>>24);

	fprintf(outfp,"%02x\n",(infor.h1<<24)>>24);
	fprintf(outfp,"%02x\n",(infor.h1<<16)>>24);
	fprintf(outfp,"%02x\n",(infor.h1<<8)>>24);
	fprintf(outfp,"%02x\n",infor.h1>>24);

	//printf("0x%02x",infor.h1>>24);
	//printf("%02x",(infor.h1<<8)>>24);
	//printf("%02x",(infor.h1<<16)>>24);
	//printf("%02x\n",(infor.h1<<24)>>24);
	
	fprintf(outfp,"%02x\n",(infor.h2<<24)>>24);
	fprintf(outfp,"%02x\n",(infor.h2<<16)>>24);
	fprintf(outfp,"%02x\n",(infor.h2<<8)>>24);
	fprintf(outfp,"%02x\n",infor.h2>>24);

	//printf("0x%02x",infor.h2>>24);
	//printf("%02x",(infor.h2<<8)>>24);
	//printf("%02x",(infor.h2<<16)>>24);
	//printf("%02x\n",(infor.h2<<24)>>24);
	
	fprintf(outfp,"%02x\n",(infor.h3<<24)>>24);
	fprintf(outfp,"%02x\n",(infor.h3<<16)>>24);
	fprintf(outfp,"%02x\n",(infor.h3<<8)>>24);
	fprintf(outfp,"%02x\n",infor.h3>>24);

	//printf("0x%02x",infor.h3>>24);
	//printf("%02x",(infor.h3<<8)>>24);
	//printf("%02x",(infor.h3<<16)>>24);
	//printf("%02x\n",(infor.h3<<24)>>24);

	fprintf(outfp,"%02x\n",(infor.h4<<24)>>24);
	fprintf(outfp,"%02x\n",(infor.h4<<16)>>24);
	fprintf(outfp,"%02x\n",(infor.h4<<8)>>24);
	fprintf(outfp,"%02x\n",infor.h4>>24);

	//printf("0x%02x",infor.h4>>24);
	//printf("%02x",(infor.h4<<8)>>24);
	//printf("%02x",(infor.h4<<16)>>24);
	//printf("%02x\n",(infor.h4<<24)>>24);
		
	fprintf(outfp,"%s\n",ADDR_LENGTH);
        fprintf(outfp,"%02x\n",(blk_num&0xff));
        fprintf(outfp,"%02x\n",(blk_num&0xff00)>>8);
        fprintf(outfp,"%02x\n",(blk_num&0xff0000)>>16);
        fprintf(outfp,"%02x\n",(blk_num&0xff000000)>>24);
	fclose(outfp);
}


int do_sha1_2(int *pbuf)
{
	int i;
	char infilename[30];
	char outfilename[30];
	srand(RAND_SEED);
	for (i=0;i<fn;i++) {
		sprintf(infilename,"./file%04d.hex",i);
		sprintf(outfilename,"./sha1_file%04d.hex",i);
		sha1_core(infilename,outfilename,pbuf);
	}
	return 0;
}


