/*
*
*      Desgined for AES Encryption
*
*
*      
*/


/*
       General Desription:

	   The users can use these as follows:

	   Encrypt Data by Aes;

	   Deccrypt Data by Aes;

	   Print Data;


*/
#include <stdio.h>
#include <stdlib.h>
#define FILE_NUM 1
//#define RAND_SEED 0
#define BLOCK_MAX_NUM 20
#define KEY_LEN 128
//#define ECB_CBC_SEL 0	//	0:ECB, 1:CBC
#define DISPLAY_ON	//	information display on
#define BYTE_NUM_ADDR 	 0x00000000
#define ECB_CBC_SEL_ADDR 0x00000004
#define KEY_LEN_ADDR     0x00000006
#define IV_ADDR  	 0x00000008
#define KEY_ADDR         0x00000018
#define O_ADDR		 0x01000000
#define E_ADDR		 0x04000000

//#include "nku.h"
//#include "nku1.h"
//#include "nku2.h"

#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b)) 
#define Multiply(x,y) (((y & 1) * x) ^ ((y>>1 & 1) * xtime(x)) ^ ((y>>2 & 1) * xtime(xtime(x))) ^ ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^ ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))

unsigned int ECB_CBC_SEL ;

unsigned int size = KEY_LEN/32;
unsigned char  RoundKey[120][8]; 
unsigned char  Rc_h[14]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,0x6c,0xd8,0xab,0x4d};  
unsigned char S_Box[0x10][0x10]=   
{   
	   //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F   
    	    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76}, //0   
	    {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0}, //1   
	    {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15}, //2   
	    {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75}, //3   
	    {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84}, //4   
	    {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf}, //5   
	    {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8}, //6   
	    {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2}, //7   
	    {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73}, //8   
	    {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb}, //9   
	    {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79}, //A   
	    {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08}, //B   
	    {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a}, //C   
	    {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e}, //D   
	    {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf}, //E   
	    {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}, //F
   
};   

unsigned char S_InvBox[0x10][0x10]=   
{   
	     //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F   
            {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb}, //0   
	    {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb}, //1   
            {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e}, //2   
	    {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25}, //3   
	    {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92}, //4   
	    {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84}, //5   
	    {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06}, //6   
	    {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b}, //7   
	    {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73}, //8   
	    {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e}, //9   
	    {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b}, //A   
	    {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4}, //B   
	    {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f}, //C   
	    {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef}, //D   
	    {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61}, //E   
	    {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}, //F   
};   			

unsigned char MixColumn_Matrix[0x04][0x04]={ 0x02,0x03,0x01,0x01,   
					     0x01,0x02,0x03,0x01,   
					     0x01,0x01,0x02,0x03,   
                                             0x03,0x01,0x01,0x02 };

unsigned char InvMixColumn_Matrix[0x04][0x04]={   
                                             0x0e,0x0b,0x0d,0x09,   
                                             0x09,0x0e,0x0b,0x0d,   
                                             0x0d,0x09,0x0e,0x0b,   
                                             0x0b,0x0d,0x09,0x0e}; 

void KeyExp128(unsigned char OriKey[4][4]);
void KeyExp192(unsigned char OriKey[4][6]);
void KeyExp256(unsigned char OriKey[4][8]);
void ByteSub(unsigned char state[4][4]);
void ShiftRow(unsigned char state[4][4]);
void MixColumn(unsigned char state[4][4]);
void InvByteSub(unsigned char state[4][4]);
void InvShiftRow(unsigned char state[4][4]);
void InvMixColumn(unsigned char state[4][4]);
void AddRoundKey(unsigned char State[4][4],int Nr);     
void Encrypt128(unsigned char State[4][4]);            
void Encrypt192(unsigned char State[4][4]);            
void Encrypt256(unsigned char State[4][4]);            
void Decrypt(unsigned char State[4][4]);           

void ByteSub(unsigned char state[4][4]) 
{ 
  int x=0,y=0; 
  for(y=0;y<4;y++) 
      for(x=0;x<4;x++) 
      { 
            state[y][x]=S_Box[state[y][x]>>4][state[y][x]&0x0f]; 
      } 
 
} 

void InvByteSub(unsigned char state[4][4]) 
{ 
  int x=0,y=0; 
  for(y=0;y<4;y++) 
      for(x=0;x<4;x++) 
      { 
            state[y][x]=S_InvBox[state[y][x]>>4][state[y][x]&0x0f]; 
      } 
 
} 

void ShiftRow(unsigned char state[4][4])
{
 int x,y;
 int n;
 int temp[4];
 
   for(y=1;y<4;y++) 
  { 
      for(x=y,n=0;n<4;x++,n++) 
      { 
        temp[n]=state[y][x%4];     
      } 
     for(x=0;x<4;x++) 
         state[y][x]=temp[x]; 
  } 
} 

void InvShiftRow(unsigned char state[4][4])
{
 int temp[4];
 
             // Rotate first row 1 columns to right	
             temp[0]=state[1][3];
			 state[1][3]=state[1][2];
			 state[1][2]=state[1][1];
			 state[1][1]=state[1][0];
			 state[1][0]=temp[0];

             // Rotate first row 2 columns to right	
			 temp[0]=state[2][3];
			 temp[1]=state[2][2];
			 state[2][3]=state[2][1];
			 state[2][2]=state[2][0];
			 state[2][1]=temp[0];
			 state[2][0]=temp[1];

             // Rotate first row 3 columns to right	
			 temp[0]=state[3][0];
			state[3][0]=state[3][1];
			state[3][1]=state[3][2];
			state[3][2]=state[3][3];
			state[3][3]=temp[0];
} 

void MixColumn(unsigned char state[4][4]) 
{ 
   unsigned char col_temp[4]; 
   int i,j,n; 
   for(j=0;j<4;j++) 
   { 
       for (i=0;i<4;i++ )
       {
         
		 col_temp[i]=Multiply(state[0][j],MixColumn_Matrix[i][0])^Multiply(state[1][j],MixColumn_Matrix[i][1])^Multiply(state[2][j],MixColumn_Matrix[i][2])^Multiply(state[3][j],MixColumn_Matrix[i][3]);

       }
	   for (n=0;n<4;n++ )
	   {

		  state[n][j]=col_temp[n];
	   }
   }  
 
}

void InvMixColumn(unsigned char state[4][4]) 
{ 
   unsigned char col_temp[4]; 
   int i,j,n; 
   for(j=0;j<4;j++) 
   { 
       for (i=0;i<4;i++ )
       {
         
		 col_temp[i]=Multiply(state[0][j],InvMixColumn_Matrix[i][0])^Multiply(state[1][j],InvMixColumn_Matrix[i][1])^Multiply(state[2][j],InvMixColumn_Matrix[i][2])^Multiply(state[3][j],InvMixColumn_Matrix[i][3]);

       }
	   for (n=0;n<4;n++ )
	   {

		  state[n][j]=col_temp[n];
	   }
   }  
 
}


void KeyExp128(unsigned char OriKey[4][4])
{
	int i,j;
	unsigned char col_temp[4],val_temp;
	for (i=0;i<4 ;i++ )
	{
		for (j=0;j<4 ;j++ )
		{
			RoundKey[j][i]=OriKey[i][j];
		}
	}
   for (i=4;i<48;i++ )
   {
	   if (i%4==0)
	   {
		   val_temp=RoundKey[i-1][0];
                   col_temp[0]=RoundKey[i-1][1];
		   col_temp[1]=RoundKey[i-1][2];
		   col_temp[2]=RoundKey[i-1][3];
		   col_temp[3]=val_temp;

		   col_temp[0]=S_Box[col_temp[0]>>4][col_temp[0]&0x0f];
		   col_temp[1]=S_Box[col_temp[1]>>4][col_temp[1]&0x0f];
		   col_temp[2]=S_Box[col_temp[2]>>4][col_temp[2]&0x0f];
		   col_temp[3]=S_Box[col_temp[3]>>4][col_temp[3]&0x0f];

                   col_temp[0]=RoundKey[i-4][0]^col_temp[0]^Rc_h[(i/4)-1];
		   col_temp[1]=RoundKey[i-4][1]^col_temp[1]^0x00;
		   col_temp[2]=RoundKey[i-4][2]^col_temp[2]^0x00;
		   col_temp[3]=RoundKey[i-4][3]^col_temp[3]^0x00;
           
		   RoundKey[i][0]=col_temp[0];
		   RoundKey[i][1]=col_temp[1];
		   RoundKey[i][2]=col_temp[2];
		   RoundKey[i][3]=col_temp[3];
	   }
	   else
	   {
           	   RoundKey[i][0]=RoundKey[i-4][0]^RoundKey[i-1][0];
		   RoundKey[i][1]=RoundKey[i-4][1]^RoundKey[i-1][1];
		   RoundKey[i][2]=RoundKey[i-4][2]^RoundKey[i-1][2];
		   RoundKey[i][3]=RoundKey[i-4][3]^RoundKey[i-1][3];
	   }
   }
}

void KeyExp192(unsigned char OriKey[4][6])
{
	int i,j;
	unsigned char col_temp[4],val_temp;
	for (i=0;i<6 ;i++ )
	{
		for (j=0;j<4 ;j++ )
		{
			RoundKey[i][j]=OriKey[j][i];
		}
	}
   for (i=6;i<58;i++ )
   {
	   if (i%6==0)
	   {
		   val_temp=RoundKey[i-1][0];
                   col_temp[0]=RoundKey[i-1][1];
		   col_temp[1]=RoundKey[i-1][2];
		   col_temp[2]=RoundKey[i-1][3];
		   col_temp[3]=val_temp;

		   col_temp[0]=S_Box[col_temp[0]>>4][col_temp[0]&0x0f];
		   col_temp[1]=S_Box[col_temp[1]>>4][col_temp[1]&0x0f];
		   col_temp[2]=S_Box[col_temp[2]>>4][col_temp[2]&0x0f];
		   col_temp[3]=S_Box[col_temp[3]>>4][col_temp[3]&0x0f];

                   col_temp[0]=RoundKey[i-6][0]^col_temp[0]^Rc_h[(i/6)-1];
		   col_temp[1]=RoundKey[i-6][1]^col_temp[1]^0x00;
		   col_temp[2]=RoundKey[i-6][2]^col_temp[2]^0x00;
		   col_temp[3]=RoundKey[i-6][3]^col_temp[3]^0x00;
           
		   RoundKey[i][0]=col_temp[0];
		   RoundKey[i][1]=col_temp[1];
		   RoundKey[i][2]=col_temp[2];
		   RoundKey[i][3]=col_temp[3];
	   }
	   else
	   {
           	   RoundKey[i][0]=RoundKey[i-6][0]^RoundKey[i-1][0];
		   RoundKey[i][1]=RoundKey[i-6][1]^RoundKey[i-1][1];
		   RoundKey[i][2]=RoundKey[i-6][2]^RoundKey[i-1][2];
		   RoundKey[i][3]=RoundKey[i-6][3]^RoundKey[i-1][3];
	   }
   }
}

void KeyExp256(unsigned char OriKey[4][8])
{
	int i,j;
	unsigned char col_temp[4],val_temp;
	for (i=0;i<8 ;i++ )
	{
		for (j=0;j<4 ;j++ )
		{
			RoundKey[i][j]=OriKey[j][i];
		}
	}
   for (i=8;i<68;i++ )
   {
	   if (i%8==0)
	   {
		   val_temp=RoundKey[i-1][0];
                   col_temp[0]=RoundKey[i-1][1];
		   col_temp[1]=RoundKey[i-1][2];
		   col_temp[2]=RoundKey[i-1][3];
		   col_temp[3]=val_temp;

		   col_temp[0]=S_Box[col_temp[0]>>4][col_temp[0]&0x0f];
		   col_temp[1]=S_Box[col_temp[1]>>4][col_temp[1]&0x0f];
		   col_temp[2]=S_Box[col_temp[2]>>4][col_temp[2]&0x0f];
		   col_temp[3]=S_Box[col_temp[3]>>4][col_temp[3]&0x0f];

                   col_temp[0]=RoundKey[i-8][0]^col_temp[0]^Rc_h[(i/8)-1];
		   col_temp[1]=RoundKey[i-8][1]^col_temp[1]^0x00;
		   col_temp[2]=RoundKey[i-8][2]^col_temp[2]^0x00;
		   col_temp[3]=RoundKey[i-8][3]^col_temp[3]^0x00;
           
		   RoundKey[i][0]=col_temp[0];
		   RoundKey[i][1]=col_temp[1];
		   RoundKey[i][2]=col_temp[2];
		   RoundKey[i][3]=col_temp[3];
	   }
	   else
	   {
           	   RoundKey[i][0]=RoundKey[i-8][0]^RoundKey[i-1][0];
		   RoundKey[i][1]=RoundKey[i-8][1]^RoundKey[i-1][1];
		   RoundKey[i][2]=RoundKey[i-8][2]^RoundKey[i-1][2];
		   RoundKey[i][3]=RoundKey[i-8][3]^RoundKey[i-1][3];
	   }
   }
}

void AddRoundKey(unsigned char State[4][4],int Nr)
{
    int i,j;
	  for (i=0;i<4;i++ )
	  {
		  for (j=0;j<4 ;j++ )
		  {
			  State[j][i]=State[j][i]^RoundKey[i+(Nr*size)][j];

		  }

	  }
}

void Encrypt128(unsigned char State[4][4])
{
	int i;
        AddRoundKey(State,0);

	for (i=1;i<=9 ;i++)
	{
	
	ByteSub(State);
	ShiftRow(State);
	MixColumn(State);
	AddRoundKey(State,i);
	}

	ByteSub(State);
	ShiftRow(State);
	AddRoundKey(State,10);
}

void Encrypt192(unsigned char State[4][4])
{
        int i;
        AddRoundKey(State,0);

        for (i=1;i<=11 ;i++)
        {

        ByteSub(State);
        ShiftRow(State);
        MixColumn(State);
        AddRoundKey(State,i);
        }

        ByteSub(State);
        ShiftRow(State);
        AddRoundKey(State,12);
}
void Encrypt256(unsigned char State[4][4])
{
        int i;
        AddRoundKey(State,0);

        for (i=1;i<=13 ;i++)
        {

        ByteSub(State);
        ShiftRow(State);
        MixColumn(State);
        AddRoundKey(State,i);
        }

        ByteSub(State);
        ShiftRow(State);
        AddRoundKey(State,14);
}

void Decrypt(unsigned char State[4][4])
{
	int i,j;
        AddRoundKey(State,10);
	InvShiftRow(State);
	InvByteSub(State);
	for (i=9;i>0 ;i--)
	{
	AddRoundKey(State,i);
	InvMixColumn(State);
	InvShiftRow(State);
	InvByteSub(State);
	}
	AddRoundKey(State,0);
}


void set_rand( unsigned char p_data[4][4] )
{
    unsigned int i, j ;
    for(i=0;i<4;i++)
		for(j=0;j<4;j++)
        	p_data[j][i] = rand()&0xff ;
}


void set_randkey128( unsigned char p_data[4][4] )
{
    unsigned int i, j ;
    for(i=0;i<4;i++)
                for(j=0;j<4;j++)
                p_data[j][i] = rand()&0xff ;
}
void set_randkey192( unsigned char p_data[4][6] )
{
    unsigned int i, j ;
    for(i=0;i<6;i++)
                for(j=0;j<4;j++)
                p_data[j][i] = rand()&0xff ;
}
void set_randkey256( unsigned char p_data[4][8] )
{
    unsigned int i, j ;
    for(i=0;i<8;i++)
                for(j=0;j<4;j++)
                p_data[j][i] = rand()&0xff ;
}


void print_char44 ( unsigned char p_data[4][4] )
{
    unsigned int i , j;
    for(i=0;i<4;i++){
	    printf("0x");
		for(j=0;j<4;j++){
        	printf("%02x",p_data[j][i]) ;
			//if(j==3) printf("\n");
		}
		printf(", ");
	if(i==3) printf("\n");
    }
}

void print_char46 ( unsigned char p_data[4][6] )
{
    unsigned int i , j;
    for(i=0;i<4;i++)
                for(j=0;j<6;j++){
                printf("%02x",p_data[i][j]) ;
                        if(j==5) printf("\n");
                }
}

void print_char48 ( unsigned char p_data[4][8] )
{
    unsigned int i , j;
    for(i=0;i<4;i++)
                for(j=0;j<8;j++){
                printf("%02x",p_data[i][j]) ;
                        if(j==7) printf("\n");
                }
}


void reset( unsigned char p_data[4][4] )
{
    unsigned int i, j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            p_data[j][i] = 0;
}

void set_data( unsigned char p_data[4][4] )
{
    unsigned int i, j ,ii=0;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++,ii++)
            p_data[j][i] = (ii<<4)|ii ;
}

void char_xor ( unsigned char p_data[4][4], unsigned char q_data[4][4] )
{
    unsigned int i, j;
    for(i=0;i<4;i++){
           for(j=0;j<4;j++)
                p_data[i][j]=p_data[i][j]^q_data[i][j];
	   }
}

void char_cpy ( unsigned char p_data[4][4], unsigned char q_data[4][4] )
{
    unsigned int i, j;
    for(i=0;i<4;i++){
           for(j=0;j<4;j++)
                p_data[i][j]=q_data[i][j];
           }
}

void w_key_len ( FILE *fp )		//	write key_lem to file
{
    unsigned int key_len;
    switch ( KEY_LEN )                    //      only AES128 is functional here, so far
    {
         case 128: key_len = 0x0000; break;
         case 192: key_len = 0x01; break;
         case 256: key_len = 0x02; break;
         default:  key_len = 0xffff;
    }
         fprintf (fp, "@%08x\n", KEY_LEN_ADDR);
         fprintf (fp, "%02x\n", (key_len)&0xff);
         fprintf (fp, "%02x\n", (key_len)>>16);

}

void w_byte_num ( FILE *fp, unsigned int block_num )		//	write byte_num to file
{
    unsigned int byte_num;
    byte_num = 16 * block_num;
    fprintf (fp, "@%08x\n", BYTE_NUM_ADDR);
    fprintf (fp, "%02x\n", (byte_num<<24)>>24);
    fprintf (fp, "%02x\n", (byte_num<<16)>>24);
    fprintf (fp, "%02x\n", (byte_num<<8)>>24);
    fprintf (fp, "%02x\n", byte_num>>24);
}

void w_ecb_cbc ( FILE *fp )	//	write ECB_CBC_SEL to file
{
    unsigned int ecb_cbc_sel = ECB_CBC_SEL ;
    fprintf (fp, "@%08x\n", ECB_CBC_SEL_ADDR);
    fprintf (fp, "%02x\n", (ecb_cbc_sel)&0xff);
    fprintf (fp, "%02x\n", (ecb_cbc_sel)>>16);
}

void w_iv ( FILE *fp, unsigned char iv[4][4])	// 	write iv to file
{
        unsigned char buff[32];
        unsigned char tmp;
        int i, j;
        fprintf (fp, "@%08x\n", IV_ADDR);
        for(i=0;i<4;i++)
             for(j=0;j<4;j++)
             {
                 tmp = iv[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}
void w_key128 ( FILE *fp, unsigned char key[4][4])	//	write key to file
{
        int i, j;
        unsigned char tmp;
        unsigned char buff[32];
        fprintf(fp,"@%08x\n",KEY_ADDR);
        for(i=0;i<4;i++)
             for(j=0;j<4;j++)
             {
                 tmp = key[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
	for(i=0;i<4;i++)
             for(j=0;j<4;j++)
             {
                 tmp = 0;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}

void w_key192 ( FILE *fp, unsigned char key[4][6]) //      write key to file
{
        int i, j;
        unsigned char tmp;
        unsigned char buff[32];
        fprintf(fp,"@%08x\n",KEY_ADDR);
        for(i=0;i<6;i++)
             for(j=0;j<4;j++)
             {
                 tmp = key[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
        for(i=0;i<2;i++)
             for(j=0;j<4;j++)
             {
                 tmp = 0;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}
void w_key256 ( FILE *fp, unsigned char key[4][8]) //      write key to file
{
        int i, j;
        unsigned char tmp;
        unsigned char buff[32];
        fprintf(fp,"@%08x\n",KEY_ADDR);
        for(i=0;i<8;i++)
             for(j=0;j<4;j++)
             {
                 tmp = key[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}


void w_original_data ( FILE *fp, unsigned char data[4][4], unsigned int k )	//	write original data to file
{
        int i, j;
        unsigned char tmp;
        unsigned char buff[32];
        unsigned long int addr = O_ADDR + k*0x10;
        fprintf(fp,"@%08x\n",addr);
        for(i=0;i<4;i++)
             for(j=0;j<4;j++)
             {
             	 tmp = data[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}

void w_encrypt_data ( FILE *fp, unsigned char data[4][4], unsigned int k )       //      write encrypt data to file
{
        int i, j;
        unsigned char tmp;
        unsigned char buff[32];
        unsigned long int addr = E_ADDR + k*0x10;
        fprintf(fp,"@%08x\n",addr);
        for(i=0;i<4;i++)
             for(j=0;j<4;j++)
             {
                 tmp = data[3-j][i] ;
                 sprintf(buff,"%02x\n",tmp);
                 fwrite(buff,1,3,fp);
             }
}

/*void AES128test(){
	unsigned int key[4];
	unsigned int data[4];

	key[0] = 0x2b7e1516;
	key[1] = 0x28aed2a6;
	key[2] = 0xabf71588;
	key[3] = 0x09cf4f3c;

	data[0] = 0x3243f6a8;
	data[1] = 0x885a308d;
	data[2] = 0x313198a2;
	data[3] = 0xe0370734;

	set_randkey128 ( key );

	Encrypt128 ( data );
	
	printf("aes128test 0x%08x 0x%08x 0x%08x 0x%08x\n",
	       data[0], data[1], data[2], data[3]);
	       }*/
void AES128 (int *pkey, int *pinput, int plen)
{
    	unsigned int i , ii ;
        unsigned char data[4][4] ;
        unsigned char key[4][4] ;           //      key for AES128
        unsigned char iv[4][4] ;            //      initial vector for CBC
        FILE * fp ;
	FILE * fpsrc;
	FILE * fpdst;

        fp = fopen( "./aa", "w") ;
	fpsrc = fopen("./orign.h", "w+");
	fpdst = fopen("./result.h", "w+");

        w_byte_num ( fp, 1 );
        w_ecb_cbc ( fp );
        w_key_len ( fp );
        reset ( iv );
        if ( ECB_CBC_SEL==1 ){
            set_rand ( iv );
	    printf ( "<iv: " ) ;
	    print_char44 ( iv ) ;
	}

        w_iv ( fp, iv );
        //set_randkey128 ( key );
        set_randkey128 ( key );
	//0xdadadb31 0x3656ab44 0xe13e685e 0xde6aef98
	// root key
#if 1	
	for (i = 0; i < 4; i++){
		key[0][i] = (pkey[i]>>24)&0xFF;
		key[1][i] = (pkey[i]>>16)&0xFF;
		key[2][i] = (pkey[i]>>8)&0xFF;
		key[3][i] = (pkey[i])&0xFF;
	}
#endif

        //#ifdef DISPLAY_ON
        //printf ( "<key:" ) ;
        //print_char44 ( key ) ;
        //#endif
        w_key128 ( fp, key ) ;
        KeyExp128 ( key ) ;
        for ( ii=0; ii<plen/16; ii++ )
            {
              	set_rand ( data );
              	w_original_data ( fp, data, ii );
                //nkusig 0x9DAFD1B7 0x1F4AAAD6 0x8CB37B27 0x03983295
		//nku1sig[4] = {0x5ba4cf69, 0x8deba118, 0xa9fb626c, 0x831905da};
		//userkey[4] = {0x3c6ff675, 0x6f769390, 0x6b4a74e4, 0x3dd7a196};
		//data[0][0] = 0x5b;data[1][0] = 0xa4;data[2][0] = 0xcf;data[3][0] = 0x69;
		//data[0][1] = 0x8d;data[1][1] = 0xeb;data[2][1] = 0xa1;data[3][1] = 0x18;
		//data[0][2] = 0xa9;data[1][2] = 0xfb;data[2][2] = 0x62;data[3][2] = 0x6c;
		//data[0][3] = 0x83;data[1][3] = 0x19;data[2][3] = 0x05;data[3][3] = 0xda;
		
#if 1
		for (i = 0; i < 4; i++){
			data[0][i] = (pinput[ii*4+i]>>24)&0xFF;
			data[1][i] = (pinput[ii*4+i]>>16)&0xFF;
			data[2][i] = (pinput[ii*4+i]>>8)&0xFF;
			data[3][i] = (pinput[ii*4+i])&0xFF;
		}
#endif		
		
              	#ifdef DISPLAY_ON
              	//printf ( "\n<<original data%d:\n", ii );
              	//print_char44 ( data );
              	#endif
              	if ( ECB_CBC_SEL==1 )
              	    	{
              	    	char_xor ( data, iv );
              	    	#ifdef DISPLAY_ON
              	    	printf ( "\n<<iv\n" );
              	    	print_char44 ( iv );
              	    	printf ( "\n<<data after iv\n" );
              	    	print_char44 ( data );
              	    	#endif
              	    	}
              	Encrypt128 ( data );
              	char_cpy ( iv, data );
              	w_encrypt_data ( fp, data, ii );
              	#ifdef DISPLAY_ON
              	//printf ( "<<encrypt  data%d:\n", ii );
              	//print_char44 ( data );
		unsigned int *tdst = (unsigned int *)data;
		fprintf(fpdst, "0x%08x, ", tdst[0]);
		fprintf(fpdst, "0x%08x, ", tdst[1]);
		fprintf(fpdst, "0x%08x, ", tdst[2]);
		fprintf(fpdst, "0x%08x,\n", tdst[3]);
		for (i = 0; i < 4; i++){
			pinput[ii*4+i] = data[0][i]<<24 | data[1][i]<<16 | data[2][i]<<8 | data[3][i];
		}
		//print_char44 ( &pinput[ii*4] );

              	#endif
           }
           fclose ( fp );
	   fclose (fpsrc);
	   fclose (fpdst);
}

void AES192 ( char* name, unsigned int block_num )
{
    	unsigned int i , ii ;
        unsigned char data[4][4] ;
        unsigned char key[4][6] ;           //      key for AES128
        unsigned char iv[4][4] ;            //      initial vector for CBC
        FILE * fp ;
        fp = fopen( name, "w") ;
        w_byte_num ( fp, block_num );
        w_ecb_cbc ( fp );
        w_key_len ( fp );
        reset ( iv );
        if ( ECB_CBC_SEL==1 )
            set_rand ( iv );
        w_iv ( fp, iv );
        set_randkey192 ( key );
        #ifdef DISPLAY_ON
        printf ( "\n<key:\n" ) ;
        print_char46 ( key ) ;
        #endif
        w_key192 ( fp, key ) ;
        KeyExp192 ( key ) ;
        for ( ii=0; ii<block_num; ii++ )
            {
              	set_rand ( data );
              	w_original_data ( fp, data, ii );
              	#ifdef DISPLAY_ON
              	printf ( "\n<<original data%d:\n", ii );
              	print_char44 ( data );
              	#endif
              	if ( ECB_CBC_SEL==1 )
              	    	{
              	    	char_xor ( data, iv );
              	    	#ifdef DISPLAY_ON
              	    	printf ( "\n<<iv\n" );
              	    	print_char44 ( iv );
              	    	printf ( "\n<<data after iv\n" );
              	    	print_char44 ( data );
              	    	#endif
              	    	}
              	Encrypt192 ( data );
              	char_cpy ( iv, data );
              	w_encrypt_data ( fp, data, ii );
              	#ifdef DISPLAY_ON
              	printf ( "<<encrypt  data%d:\n", ii );
              	print_char44 ( data );
              	#endif
           }
           fclose ( fp );
}


void AES256 ( char* name, unsigned int block_num )
{
    	unsigned int i , ii ;
        unsigned char data[4][4] ;
        unsigned char key[4][8] ;           //      key for AES128
        unsigned char iv[4][4] ;            //      initial vector for CBC
        FILE * fp ;
        fp = fopen( name, "w") ;
        w_byte_num ( fp, block_num );
        w_ecb_cbc ( fp );
        w_key_len ( fp );
        reset ( iv );
        if ( ECB_CBC_SEL==1 )
            set_rand ( iv );
        w_iv ( fp, iv );
        set_randkey256 ( key );
        #ifdef DISPLAY_ON
        printf ( "\n<key:\n" ) ;
        print_char48 ( key ) ;
        #endif
        w_key256 ( fp, key ) ;
        KeyExp256 ( key ) ;
        for ( ii=0; ii<block_num; ii++ )
            {
              	set_rand ( data );
              	w_original_data ( fp, data, ii );
              	#ifdef DISPLAY_ON
              	//printf ( "\n<<original data%d:\n", ii );
              	print_char44 ( data );
              	#endif
              	if ( ECB_CBC_SEL==1 )
              	    	{
              	    	char_xor ( data, iv );
              	    	#ifdef DISPLAY_ON
              	    	printf ( "\n<<iv\n" );
              	    	print_char44 ( iv );
              	    	printf ( "\n<<data after iv\n" );
              	    	print_char44 ( data );
              	    	#endif
              	    	}
              	Encrypt256 ( data );
              	char_cpy ( iv, data );
              	w_encrypt_data ( fp, data, ii );
              	#ifdef DISPLAY_ON
              	//printf ( "<<encrypt  data%d:\n", ii );
              	//print_char44 ( data );
              	#endif
           }
           fclose ( fp );
}


int do_aes(int *key, int *input, int len)
{
        unsigned int filenum ;
        unsigned int block_num ;            //      block number, every block is 128 bits
	unsigned int RAND_SEED = 7553;
        char filename[32] ;
	srand ( RAND_SEED ) ;
	for ( filenum=0; filenum<FILE_NUM; filenum++)   //      make files
        {
		if ( (filenum%10) == 0 ){
			RAND_SEED ++;
			printf("RAND_SEED = %d\n",RAND_SEED);
			srand ( RAND_SEED ) ;
		}
           	sprintf( filename, "./hex/aes_file%04d.hex", filenum);
		block_num = len / 16;
		//ECB_CBC_SEL = rand()%2 ;
		ECB_CBC_SEL = 0;
		if ( O_ADDR + BLOCK_MAX_NUM*0x10 >= E_ADDR ){
			printf ( "Block_num overflow!\n" );
			return 0;
		}
           	//printf("file_name=%s, block_num=%d, ecb_cbc=%d\n",filename, block_num,ECB_CBC_SEL);
           	if ( KEY_LEN == 128 )
			AES128 (key, input, len);
		else if ( KEY_LEN == 192 )
			AES192 ( filename, block_num );
		else if ( KEY_LEN == 256 )
                        AES256 ( filename, block_num );
		else 
		{
			printf ( "Key_len error!\n" );
			return 0;
		}
        }
}
