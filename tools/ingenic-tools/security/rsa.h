#ifndef _SCROM_RSA_H_
#define _SCROM_RSA_H_

// max length in 32-bit word
// MAX length in bit ML_BIT / 32 * 2 + 1, ML_BIT = 1984
//#define BI_MAXBIT 1984
// 1984 / 32 * 2 + 1 = 62*2 + 1 = 125
#define BI_MAXLEN 125//能够表示的最大的位数由这个决定
// OTP MAXLEN for nn+ku in word = 2048/32 - 1
#define OTP_MAXLEN 63
// MIN N in bit
#define MIN_N_BIT 17
// MIN K in bit
#define MIN_K_BIT 17
// MIN DATA in bit
#define MIN_D_BIT 8

#define BIT_SHIFT 32
#define BIT_BASE ((unsigned long long)0x1 << BIT_SHIFT) 

int bi_lt (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data);
//int bi_add (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data,
//	    unsigned long *c_data);
int bi_sftadd (int a_len, unsigned long *a_data, 
	       int b_len, unsigned long *b_data, int s_len);
int bi_sftsub (int a_len, unsigned long *a_data, 
	       int b_len, unsigned long *b_data, int s_len);
int bi_mul (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data,
	    unsigned long *c_data);
int bi_mul1 (int a_len, unsigned long *a_data, unsigned long b_d,
	     unsigned long *c_data);
int bi_mod (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data);
int bi_blen (int len, unsigned long *data);
int bi_len (int len, unsigned long *data);
int bi_mon (int a_len, unsigned long *a_data, int k_len, unsigned long *k_data, 
	    int b_len, unsigned long *b_data, unsigned long *c_data);
int bi_eq (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data);

extern int nn_len;
extern unsigned long nn[BI_MAXLEN/2];
extern int ku_len;
extern unsigned long ku[BI_MAXLEN/2];

int invalid_nk (int, unsigned long *, int, unsigned long *);
int invalid_d (int, unsigned long *, int, unsigned long *);
int rsa_eq (int src_len, unsigned long *src, int dst_len, unsigned long *dst);
int rsa_md5 (int src_len, unsigned long *src, unsigned long *sig);

#ifndef SCROM_RSA
/**************************************************************/
/****** the following functions are not needed in SC-ROM ******/
/**************************************************************/
int bi_add1 (int a_len, unsigned long *a_data, unsigned long b_d);
int bi_str2bi (char *str, int str_len, unsigned long *a_data, 
	       const unsigned int system);
unsigned long bi_mod1 (int a_len, unsigned long *a_data, unsigned long b_d);
int bi_div1 (int a_len, unsigned long *a_data, unsigned long b_d);
int bi_bi2str (char *str, int a_len, unsigned long *a_data, 
	       const unsigned int system);
void bi_print (char * name, int a_len, unsigned long *a_data, 
	       const unsigned int system);

#ifdef COUNT_MUL
extern int mul_num;
extern int num1, num2;
extern long long sig[32];
#endif
#endif

#endif /* _SCROM_RSA_H_ */

