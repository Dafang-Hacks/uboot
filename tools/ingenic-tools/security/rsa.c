/*implement a high efficient big integer algorithm for 32-bit CPU*/

#include "rsa.h"   

#ifndef SCROM_RSA
#include <stdio.h>
#include <stdlib.h>

#ifdef COUNT_MUL
int mul_num;
int num1, num2;
long long sig[32];
#endif
#endif


/*struct bigint
{
  int len;
  //int sign; //0:positive, 1:negative
  unsigned long *data; //point to the LSW, which means little-endian word view
};
*/

// compare if A < B
int bi_lt (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data)
{
  int i;
  if (a_len != b_len)
  {
    return a_len < b_len;
  }

  for (i=a_len-1; i>=0; i--){
    if (a_data[i] != b_data[i]) 
	return a_data[i] < b_data[i];
  }	

  return 0;
}

/*
// add: C=A+B, return C's len
int bi_add (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data,
	    unsigned long *c_data)
{
  unsigned long carry, sum, *data;
  int i, len_min, len_max;

  if (a_len < b_len) {
    len_min = a_len;
    len_max = b_len;
    data = b_data;
  } else {
    len_min = b_len;
    len_max = a_len;
    data = a_data;
  }

  carry = 0;
  for (i=0; i<len_min; i++) {
    sum = a_data[i] + b_data[i] + carry;
    //need check both for case of b_data + carry = 2^32, so sum=a_data;
    carry = sum < a_data[i] || sum < b_data[i];
    c_data[i] = sum;
  }
  for (; i<len_max; i++) {
    sum = data[i] + carry;
    carry = sum < data[i];
    data[i] = sum;
  }
  data[len_max] = carry;
  return len_max+carry;
}
*/

// left-shift-add: A=A+(B<<(s_len*32)), return new A's len
int bi_sftadd (int a_len, unsigned long *a_data, 
		int b_len, unsigned long *b_data, int s_len)
{
	unsigned long carry;
	unsigned long long sum;
	int i, b_len_s, len_max;

	b_len_s = b_len + s_len;
	len_max = (a_len < b_len_s) ? b_len_s : a_len;

	for (i=a_len; i<len_max; i++) a_data[i] = 0;
	for (i=b_len; i<len_max-s_len; i++) b_data[i] = 0;
	carry = 0;
	for (i=s_len; i<len_max; i++) {
		sum = a_data[i];   
		sum = sum + b_data[i-s_len] + carry;
		a_data[i]= (unsigned long)(sum & (BIT_BASE - 1));
		carry = (unsigned long)(sum >> BIT_SHIFT);   
	}
	a_data[len_max] = carry;
	return len_max+carry;
}

// left-shift-sub: A=A-(B<<(s_len*32)), return new A's len
int bi_sftsub (int a_len, unsigned long *a_data, 
		int b_len, unsigned long *b_data, int s_len)
{
	unsigned long carry;
	unsigned long sum;
	int i, b_len_s;

	b_len_s = b_len + s_len;

	//printf("a_len=%d, b_len=%d, s_len=%d\n", a_len, b_len, s_len);

	for (i=b_len; i<a_len-s_len; i++) b_data[i] = 0;

	//for (i=a_len-1; i>=s_len; i--) 
	//printf("i=%d, a_data=%lu, b_data=%lu\n", i, a_data[i], b_data[i-s_len]);

	carry = 0;
	for (i=s_len; i<a_len; i++) {
		sum = a_data[i] - b_data[i-s_len] - carry;
		//need send check for case of b_data + carry(=1) = 2^32, so sum=a_data;
		carry = (sum > a_data[i]) || (carry && (sum == a_data[i]));
		a_data[i]= sum;
	}

	for (i=a_len-1; i>=0; i--)
		if (a_data[i] != 0) break;
	a_len = i + 1;

#ifndef SCROM_RSA
	if (carry || a_len <= 0) {
		printf("Error: unexpected carry %lu or len %d, quit\n", carry, a_len);
		exit (-1);
	}
#endif

	return a_len;
}

// C = A * B, return C's len. C cannot be A or B
int bi_mul (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data,
		unsigned long *c_data)
{
	unsigned long carry, data[BI_MAXLEN];
	unsigned long long mul;
	int i,j, len, c_len;

	c_len = 0;
	//for (j=a_len-1; j>=0; j--) printf("j=%d, a_data=%lu\n", j, a_data[j]);
	//for (j=b_len-1; j>=0; j--) printf("j=%d, b_data=%lu\n", j, b_data[j]);
	for (i=0; i<a_len; i++) {
		carry = 0;
		len = b_len;
		for (j=0; j<b_len; j++) {
			mul = b_data[j];   
			mul = mul * (unsigned long long)a_data[i] + carry;   
			data[j] = (unsigned long)(mul & (BIT_BASE-1));   
			carry = (unsigned long)(mul >> BIT_SHIFT);
#ifdef COUNT_MUL
			//mul_num++;
#endif
		}
		data[len] = carry;
		len += (carry != 0);
		//for (j=len-1; j>=0; j--) printf("i=%d,j=%d, %lu\n", i,j, data[j]);

		c_len = bi_sftadd (c_len, c_data, len, data, i);
	}

	return c_len;
}

// C = A * b, return C's len, A and C can be the same
int bi_mul1 (int a_len, unsigned long *a_data, unsigned long b_d,
		unsigned long *c_data)
{
	unsigned long carry;
	unsigned long long mul;
	int i, c_len;

	carry = 0;
	c_len = a_len;
	for (i=0; i<a_len; i++) {
		mul = a_data[i];
		mul = mul * (unsigned long long)b_d + carry;   
		c_data[i] = (unsigned long)(mul & (BIT_BASE-1));   
		carry = (unsigned long)(mul >> BIT_SHIFT);
#ifdef COUNT_MUL
		//mul_num++;
#endif
	}
	c_data[c_len] = carry;
	c_len += (carry != 0);

	return c_len;
}

// A = A % B, return A's new len
/* following variables are initialized in beginning of bi_mon
   n_w0 = b_data[len-1]
   n_w1 = b_data[len-2]
n_ffw: N's first-full-word
n_2fw_c: ~ of N's 2nd-full-word
n_ffw_p1: above + 1
n_hw1_p1: N's 1st half-word + 1
n_hw2_c14: N's complemental of 2nd half-word 14-m-s-bits
n_hw2_c15: N's complemental of 2nd half-word 15-m-s-bits
n_hw2_c16_b0: N's complemental of 2nd half-word lsb
div_nhw1p1: FFFFFFFF / n_hw1_p1
rem_nhw1p1_p1 : FFFFFFFF % n_hw1_p1 + 1
n_offbt_c[i]: ~ of N offset i+8 bits
 */
#ifndef SCROM_RSA
struct GLOBAL_VAR {
	unsigned long n_ffw, n_ffw_p1, n_w0, n_w1, n_hw1_p1, n_hw2_c15,
		      div_nhw1p1, rem_nhw1p1_p1, n_hw2_c16_b0, n_2fw_c;
	int lz_nw0, sig_nw0;
	unsigned long n_offbit_c[8];
} global_var;
#endif
//int DEBUG = 0;
//int DEBUG_N = 0;

int bi_mod (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data)
{
	unsigned long a_d, b_d, a_d_h, a_d_l, div, data[BI_MAXLEN];
	int i,len, s_len, small_b, a_le_b, a_eq_b;
#ifndef SCROM_RSA
	struct GLOBAL_VAR * global = &global_var;
#endif
	unsigned long n_hw1_p1 = global->n_hw1_p1;

#ifdef COUNT_MUL
	num1 += a_len - b_len + 1;
#endif
	while(1) {
		if (bi_lt(a_len, a_data, b_len, b_data)) return a_len;

		s_len = a_len - b_len;
#if 0//find a quot to mul alg 0 - the simplest one
		if (a_data[a_len-1] > global->n_w0) {
			div = a_data[a_len-1] / (global->n_w0 + 1);

		} else {

			if (s_len == 0 || a_len == 1) div = 1;
			//if (a_data[a_len-1] == b_data[b_len-1]) div = 1;
			else {

				if (global->n_w0 == (BIT_BASE-1)) {
					div = a_data[a_len-1];

				} else {
					num = a_data[a_len-1];
					num = (num << BIT_SHIFT) + a_data[a_len-2];
					div = num / (global->n_w0 + 1);
				}
				s_len--;
			}
		}
#endif

#if 1//find a quot to mul alg 2 - the most efficient
#define SMALL_A_B 15
		a_d = a_data[a_len-1];
		b_d = global->n_w0;
		int lz_num_a, lz_num_b, lz_num, a_lt_b, sig_a_b;
		unsigned long a_d1, b_d1;
		lz_num_b = global->lz_nw0;
#ifdef MIPS_INSN
		__asm__ __volatile__ ("clz\t%0,%1":"=r"(lz_num_a):"r"(a_d));
#else
		for(i=31; i >= 0; i--)
			if(a_d & (1 << i)) break;
		lz_num_a = 32 - (i + 1);
#endif
		a_eq_b = a_d == b_d;
		a_lt_b = a_eq_b ? (a_data[a_len-2] < global->n_w1) : a_d < b_d;
		lz_num = a_lt_b ? lz_num_a - 32 : lz_num_a;
		a_d1 = lz_num_a != 0 ?
			(a_d << lz_num_a) | (a_data[a_len-2] >> (32 - lz_num_a)) : a_d;
		sig_a_b = (32 - lz_num) - global->sig_nw0;
		// use l-div is enough: a_d1 / (b_d1+1), where sig(a_di) == 32
		// printf ("a_d=%lx, b_d=%lx, sig_a_b=%d\n", a_d, b_d,sig_a_b);
		if ((a_eq_b && !a_lt_b) || (sig_a_b <= SMALL_A_B)) {
			b_d1 = global->n_ffw >> sig_a_b;

			if (a_d1 == b_d1) {
				if (s_len == 0) div = 1;
				else {
					div = BIT_BASE-2;
					s_len--;
				}
			} else {
				unsigned long rem, b_d1_p1 = b_d1 + 1;
				div = a_d1 / b_d1_p1;
				rem = a_d1 % b_d1_p1;
				if ((div + rem) >= b_d1_p1 && sig_a_b >= 8) {
					/* try to get the "1" for div
					   div = (A - R) / (B + dB)
					   where A = a_d1, dB = (2**32 - b_d2) / 2**32, B+dB = b_d1+1
					   R is remainder rem
					   div
					   = A/B (1-R/A) / (1+dB/B)
					   = A/B (1-R/A) {1 - (dB/B)/[1+(dB/B)]}
					   = A/B {1 - R/A - (dB/B)/[1+(dB/B)] + (R/A)(dB/B)/[1+(dB/B)]}
					   div1  = A / B
					   div1 - div
					   = A/B {R/A + (dB/B)/[1+(dB/B)] - (R/A)(dB/B)/[1+(dB/B)]}
					   = 1/B {R + A*dB /(B+dB) - R*dB/(B+dB)}
					   = 1/B [R + dB * (A-R) /(B+dB)]
					   = (R + dB * div) / B
					   So if (R + dB * div) >= (b_d1 + 1), div++
					 */
					unsigned long dt_b;
					unsigned long long mul;
					/*b_d2 = a_lt_b ?
					  (n_w0 << lz_num_a) | (n_w1 >> (32 - lz_num_a)) :
					  (lz_num_a != 0 ? 
					  n_w1 << lz_num_a | b_data[b_len-3] >> (32 - lz_num_a) :
					  n_w1);
					  dt_b = ~b_d2;  //dt_b = (1 - b_d2)*2^32 - 1*/
					dt_b = global->n_offbit_c[sig_a_b-8];
					mul = (unsigned long long)dt_b * (unsigned long long)div;
					dt_b = mul >> BIT_SHIFT;
					div += (rem + dt_b) >= b_d1_p1;
#if 0
					if (sig_a_b == 16) {
						rem += dt_b;
						if (rem > b_d1) rem -= b_d1;
						div += rem > b_d1;
						if (rem > b_d1) rem -= b_d1;
						div += rem > b_d1;
						if (rem > b_d1) rem -= b_d1;
						div += rem > b_d1;
					}
#endif
				}
				s_len -= a_lt_b;
			}
			//if (div != 0 && a_lt_b) s_len--;
		} else {
			// use ll-div for more accurate div:
			// {a_d_h,a_d_l} / (n_ffw+1), where sig(n_ffw) == 32
			a_le_b = a_lt_b;
			small_b = lz_num_b != 0;
			a_d_h = small_b ?
				(a_le_b ? (a_d << lz_num_b) | (a_data[a_len-2] >> global->sig_nw0) :
				 a_d >> global->sig_nw0) : 
					(a_le_b ? a_d : 0);
			a_d_l = small_b ?
				(a_le_b ?
				 (a_data[a_len-2] << lz_num_b) | (a_data[a_len-3] >> global->sig_nw0) :
				 (a_d << lz_num_b) | (a_data[a_len-2] >> global->sig_nw0)) : 
					(a_le_b ? a_data[a_len-2] : a_d);
			if (global->n_ffw_p1 == 0) {
				div = a_d_h;
			} else {
#if 0//64b/32b-ll-div alg 0
				num = a_d_h;
				num = (num << BIT_SHIFT) + a_d_l;
				div = num / global->n_ffw_p1;
#else//64b/32b-ll-div alg 3
				/*if (DEBUG_N == 15) 
				  printf ("a_d_h=%lx, a_d_l=%lx, n_ffw+1=%lx, n_hw1+1=%lx, a_d1=%lx\n", 
				  a_d_h, a_d_l, global->n_ffw_p1, global->n_hw1_p1, a_d1);*/
				unsigned long div_d1, div_d2, div1, rem;
				unsigned long long mul, a_d_ll;
#ifdef COUNT_MUL
				//num1 += 1;
#endif
				div = a_d1 / n_hw1_p1;
				rem = a_d1 % n_hw1_p1;
				/* try to get the number after poing "." for div
				   div = (A - R) / (B + dB)
where: 
A = a_d1, dB = (2**16 - n_hw2) / 2**16, B+dB = n_hw1 + 1
R is remainder rem, B = n_hw1 + n_hw2 * 2**-16, 
n_hw2_ = (2**16 - n_hw2 - 1) >> 2, 
use 14 bit so mul will not exceed 32 bit
div1  = A / B
div1 - div
= (R + dB * div) / B

So: 
div1 = div + (R<<16 + div * n_hw2_) / (n_hw1+1) / 2^16
div1 = div + (R<<14 + div * n_hw2_) / (n_hw1+1) / 2^15
div1 = div + (R<<14 + div * n_hw2_) / (n_hw1+1) / 2^14
				 */
#if 1//16-bit n_hw2
				//unsigned long n_hw2_c16_b0;
				div_d2 = (rem << 16) + ((a_d_l << (32 - sig_a_b)) >> 16);
				div_d2 += global->n_hw2_c16_b0 ? div : 0;
				div_d2 >>= 1;
				div_d1 = div_d2 + div * global->n_hw2_c15;
				div1 = div_d1 / n_hw1_p1;
				rem = div_d1 % n_hw1_p1;
				if (div_d1 < div_d2) {
					div1 += global->div_nhw1p1;
					rem += global->rem_nhw1p1_p1;
					if (rem >= n_hw1_p1) {
						rem -= n_hw1_p1;
						div1 ++;
					}
				}
				div1 <<= 1;
				rem <<= 1;
				div1 += rem >= n_hw1_p1;
				div1 >>= (32 - sig_a_b);
#endif
#if 0//15-bit n_hw2
				div_d2 = (rem << 15) + ((a_d_l << (32 - sig_a_b)) >> 17);
				div_d1 = div_d2 + div * global->n_hw2_c15;
				div1 = div_d1 / n_hw1_p1;
				rem = div_d1 % n_hw1_p1;
				if (div_d1 < div_d2) {
					div1 += global->div_nhw1p1;
					rem +=  global->rem_nhw1p1_p1;
					if (rem >= n_hw1_p1) {
						rem -= n_hw1_p1;
						div1 ++;
					}
				}
				if (sig_a_b == 32) {
					div1 <<= 1;
					rem <<= 1;
					div1 += rem >= n_hw1_p1;
				} else {
					div1 >>= (31 - sig_a_b);
				}
#endif
#if 0//14-bit n_hw2
				div_d2 = (rem << 14) + ((a_d_l << (32 - sig_a_b)) >> 18);
				div_d1 = div_d2 + div * global->n_hw2_c14;
				div1 = div_d1 / n_hw1_p1;
				rem = div_d1 % n_hw1_p1;
				div1 = (sig_a_b > 30) ?
					(div1 << (sig_a_b - 30)) : (div1 >> (30 - sig_a_b));
				if (sig_a_b > 30) {
					rem <<= (sig_a_b - 30);
					div1 += rem >= n_hw1_p1;
				}
#endif
				div1 = (div << (sig_a_b - 16)) + div1;
				div = 0;
				while (1) {
#ifdef COUNT_MUL
					//num2++;
#endif
					mul = (unsigned long long)global->n_ffw_p1 * (unsigned long long)div1;
					a_d_ll = ((unsigned long long)a_d_h << BIT_SHIFT) + a_d_l;
#ifdef RSA_VERIFY
					if (a_d_ll < mul) {
						printf("Error: a_d_ll(%llx) < mul(%llx)\n", a_d_ll, mul);
						printf("\tn_ffw=%lx a_d_h=%lx a_d_l=%lx div1=%lx rem=%lx sig=%d n_hw2_c14=%lx a_d1=%lx\n",
								global->n_ffw, a_d_h, a_d_l, div1, rem, sig_a_b, 
								global->n_hw2_c15, a_d1);
						exit(-1);
					}
#endif
					a_d_ll -= mul;
					div += div1;
					a_d_h = (unsigned long)(a_d_ll >> BIT_SHIFT);
					a_d_l = (unsigned long)(a_d_ll & (BIT_BASE-1));
					//if (DEBUG_N == 15) 
					//printf ("a_d_h=%lx, a_d_l=%lx, div1=%lx\n", a_d_h, a_d_l, div1);
					if (a_d_h == 0) {
#ifdef COUNT_MUL
						sig[0]++;
#endif
						break;
					}
#ifdef MIPS_INSN
					__asm__ __volatile__ ("clz\t%0,%1":"=r"(lz_num_a):"r"(a_d_h));
#else
					for(i=31; i >= 0; i--)
						if(a_d_h & (1 << i)) break;
					lz_num_a = 32 - (i + 1);
#endif
					a_d1 = (a_d_h << lz_num_a) | (a_d_l >> (32 - lz_num_a));
					sig_a_b = 32 - lz_num_a;
#ifdef COUNT_MUL
					sig[sig_a_b]++;
#endif
					div1 = a_d1 / ((global->n_ffw >> sig_a_b) + 1);
				}
				div += a_d_l >= global->n_ffw_p1;
				rem = a_d_l >= global->n_ffw_p1 ? (a_d_l - global->n_ffw_p1) : a_d_l;
				// N's 2nd-word
				mul = (unsigned long long)global->n_2fw_c * (unsigned long long)div;
				div_d1 = mul >> BIT_SHIFT;
				/*if (DEBUG_N == 15) 
				  printf ("div=%lx, n_2fw_c=%lx, div_d1=%lx\n", 
				  div, global->n_2fw_c, div_d1);*/
				rem += div_d1;
				div += rem >= global->n_ffw_p1;
				div += rem < div_d1;//in case of 32-bits overflow when rem+=div_d1
#endif
			}
			s_len -= a_le_b;

		}
		//if (DEBUG_N == 15) printf ("div=%lx\n", div);
#endif

#if 0//find a quot to mul alg 1 - the best quot
		a_d = a_data[a_len-1];
		b_d = b_data[b_len-1];
		int lz_num;
#ifdef MIPS_INSN
		__asm__ __volatile__
			("clz\t%0,%1\#get leading zero num":"=r"(lz_num):"r"(b_d));
#else
		for(i=31; i >= 0; i--)
			if(b_d & (1 << i)) break;
		lz_num = 32 - (i + 1);
#endif
		small_b = lz_num != 0;
		a_le_b = a_d <= b_d;
		a_eq_b = a_d == b_d;
		b_d = small_b ?
			(b_d << lz_num) | (b_data[b_len-2] >> (32 - lz_num)) : b_d;
		a_d_h = small_b ?
			(a_le_b ? (a_d << lz_num) | (a_data[a_len-2] >> (32 - lz_num)) :
			 a_d >> (32 - lz_num)) : 
				(a_le_b ? a_d : 0);
		a_d_l = small_b ?
			(a_le_b ?
			 (a_data[a_len-2] << lz_num) | (a_data[a_len-3] >> (32 - lz_num)) :
			 (a_d << lz_num) | (a_data[a_len-2] >> (32 - lz_num))) : 
				(a_le_b ? a_data[a_len-2] : a_d);
		if (a_eq_b && a_d_h == b_d && s_len == 0) div = 1;
		else {
			if (a_eq_b && a_d_h > b_d) {
				a_d_l = a_d_h;
				a_d_h = 0;
			} else if (a_le_b) s_len--;

			if (b_d == (BIT_BASE-1)) {
				div = a_d_h;
			} else {
#if 1//64b/32b-ll-div alg 0
				num = a_d_h;
				num = (num << BIT_SHIFT) + a_d_l;
				div = num / (b_d + 1);
#endif
			}
		}
#endif
#ifndef SCROM_RSA
		if (div == 0) {
			printf("Error: div == 0, quit\n");
			printf("\tb_d_h=%lx b_d_l=%lx a_d_h=%lx a_d_l=%lx s_len=%d\n",
					b_data[b_len-1], b_data[b_len-2], 
					a_data[a_len-1], a_data[a_len-2], s_len);
			exit(-1);
		}
#endif
		len = bi_mul1(b_len, b_data, div, data);
		// A = A - (B * div) << (s_len*32)
		/*if (DEBUG_N == 15) {
		  printf ("s_len=%d\n", s_len);
		  bi_print ("a_data", a_len, a_data, 16);
		  bi_print ("data", len, data, 16);
		  }*/
		a_len = bi_sftsub (a_len, a_data, len, data, s_len);
#ifdef COUNT_MUL
		num2++;
#endif
	}
}

// return bit-len of data
int bi_blen (int len, unsigned long *data)
{
  int i, blen;
  for(i=31; i >= 0; i--)
    if(data[len-1] & (1 << i)) break;
  blen = i + 1 + (len - 1) * 32;
  return blen;
}

// return 32-bit-word-len of data
int bi_len (int len, unsigned long *data)
{
  int i;
  for(i=len-1; i >= 0; i--)
    if(data[i]) break;
  return i+1;
}

/* 蒙哥马利算法求：C = A^K mod B
   A is decrypted (encrypted) code, K is publie or private key, 
   C is encrypted (decrypted) code, B is N,
   return C's len
*/
int bi_mon (int a_len, unsigned long *a_data, int k_len, unsigned long *k_data, 
	    int b_len, unsigned long *b_data, unsigned long *c_data)
{
  unsigned long aa_data0[BI_MAXLEN], aa_data1[BI_MAXLEN], 
    ao_data0[BI_MAXLEN], *ao_data1/*[BI_MAXLEN]*/;
  unsigned long *tmp_data, *c_aa_data, *l_aa_data, *c_ao_data, *l_ao_data;
  int i, odd, k_blen, c_aa_len, l_aa_len, c_ao_len, l_ao_len;

  // get K length in bit
  k_blen = bi_blen(k_len, k_data);
  //printf("===============%s %d %d\n",__FILE__,__LINE__,k_blen);
  //printf("A: len=%d\n", a_len);
  //printf("K: len=%d\n", k_len);
  //printf("B: len=%d\n", b_len);

  // initial static
#ifndef SCROM_RSA
  struct GLOBAL_VAR * global = &global_var;
#endif
  global->n_w0 = b_data[b_len-1];
  global->n_w1 = b_data[b_len-2];
#ifdef MIPS_INSN
  __asm__ __volatile__ ("clz\t%0,%1":"=r"(global->lz_nw0):"r"(global->n_w0));
#else
  for(i=31; i >= 0; i--) if(global->n_w0 & (1 << i)) break;
  global->lz_nw0 = 32 - (i + 1);
#endif

  global->sig_nw0 = 32 - global->lz_nw0;
  unsigned long n_2fw, n_w2 = b_data[b_len-3];
  global->n_ffw = (global->lz_nw0 == 0) ? 
    global->n_w0 : 
    (global->n_w0 << global->lz_nw0) | (global->n_w1 >> global->sig_nw0);
  n_2fw = (global->lz_nw0 == 0) ? 
    global->n_w1 :
    (global->n_w1 << global->lz_nw0) | (n_w2 >> global->sig_nw0);
  global->n_2fw_c = ~n_2fw;
  for (i=0; i<8; i++) {
    global->n_offbit_c[i]
      = (global->n_ffw << (32 - (i + 8))) | (n_2fw >> (i + 8));
    global->n_offbit_c[i] = ~global->n_offbit_c[i];
  }
  global->n_ffw_p1 = global->n_ffw + 1;
  global->n_hw1_p1 = (global->n_ffw >> 16) + 1;
  // -1 for hw2
  //n_hw2_c14 = ((1<<16) - 1 - n_ffw & ((1<<16) - 1)) >> 2;
  global->n_hw2_c15 = ((1<<16) - 1 - (global->n_ffw & ((1<<16) - 1)));
  global->n_hw2_c16_b0 = global->n_hw2_c15 & 1;
  global->n_hw2_c15 >>= 1;
  global->div_nhw1p1 = (BIT_BASE-1) / global->n_hw1_p1;
  global->rem_nhw1p1_p1 = (BIT_BASE-1) % global->n_hw1_p1 + 1;

  // initial
  c_aa_len = a_len;
  c_aa_data = aa_data0;
  for (i=0; i<a_len; i++) c_aa_data[i] = a_data[i];
  l_aa_data = aa_data1;

  // to save 1 BI space, use c_data as ao_data1
  ao_data1 = c_data;
  c_ao_len = 1;
  c_ao_data = ao_data0;
  c_ao_data[0] = 1;
  l_ao_data = ao_data1; l_ao_len = 0;
  //  error_exit(ERR_CHECK_DATA_FAILED);
  for(i = 0; i < k_blen; i++) {
    odd = k_data[i >> 5] & (1 << (i & 0x1F));     
    //printf("%s %d %d %d %d\n",__FILE__,__LINE__,i, odd, k_blen);
   
    if (odd) {
      //if (i == 108) printf("odd i=%d\n", i);
      l_ao_len = bi_mul (c_aa_len, c_aa_data, c_ao_len, c_ao_data, l_ao_data);
      //if (i == 108) printf("MUL: i=%d, len=%d\n", i, l_ao_len);
      //if (i == 108) {DEBUG=1; DEBUG_N=0;}
      l_ao_len = bi_mod (l_ao_len, l_ao_data, b_len, b_data);
     
      //if (i == 108) printf("MOD: i=%d, len=%d\n", i, l_ao_len);
      if (i == (k_blen - 1)) break;
      c_ao_len = l_ao_len;
      tmp_data = l_ao_data;
      l_ao_data = c_ao_data;
      c_ao_data = tmp_data;
      //exit(-1);
    }

    //if (i == 108) printf("i=%d\n", i);
    l_aa_len = bi_mul (c_aa_len, c_aa_data, c_aa_len, c_aa_data, l_aa_data);

    //if (i == 108) printf("MUL: i=%d, len=%d\n", i, l_aa_len);
    l_aa_len = bi_mod (l_aa_len, l_aa_data, b_len, b_data);
    //if (i == 108) printf("MOD: i=%d, len=%d\n", i, l_aa_len);
    c_aa_len = l_aa_len;
    tmp_data = l_aa_data;
    l_aa_data = c_aa_data;
    c_aa_data = tmp_data;
  }  
 
#ifndef SCROM_RSA
  if (i != (k_blen - 1)) {
    printf("Error: unexpected end of the loop, i=%d, k_blen=%d, quit\n",
	   i, k_blen);
    exit (-1);
  }
#endif

  if (c_data != l_ao_data)
    for (i=0; i<l_ao_len; i++) c_data[i] = l_ao_data[i];

  return l_ao_len;
}   

// compare if A = B
int bi_eq (int a_len, unsigned long *a_data, int b_len, unsigned long *b_data)
{
  int i;
  if (a_len != b_len) return 0;
  for (i=a_len-1; i>=0; i--)
    if (a_data[i] != b_data[i]) return 0;

  return 1;
}

#ifndef SCROM_RSA
int nn_len;
unsigned long nn[BI_MAXLEN/2];
int ku_len;
unsigned long ku[BI_MAXLEN/2];
#endif

/* RSA crypt SRC by using nn(N) and ku(public key) and compare it with DST
   return value:
   0  - success and compare PASS
   1  - success and compare FAIL
   -1 - invalid nn or ku, if nn-length-bit < 512 or ku-length-bit < 17
        or *-length-word > BI_MAXLEN / 2 or nn+ku-length-word > (2048/32 - 1)
	or ku >= nn
   -2 - invalid src data, if src-length-bit < 64
        or src-length-word > BI_MAXLEN / 2 or src > nn
   -3 - invalid dst data, if dst-length-bit < 64
        or dst-length-word > BI_MAXLEN / 2 or dst > nn
*/
// return 1 if N or Ku is invalid
int invalid_nk (int nn_len, unsigned long *nn, int ku_len, unsigned long *ku)
{
  int invalid, len, nn_blen, ku_blen;
  invalid = (nn_len <= 0 || 
	     nn_len > BI_MAXLEN / 2 ||
	     ku_len <= 0 ||
	     ku_len > BI_MAXLEN / 2 ||
	     (nn_len + ku_len) > OTP_MAXLEN);
  if (invalid) return 1;
  len = bi_len (nn_len, nn);
  invalid = len != nn_len;
  len = bi_len (ku_len, ku);
  invalid |= len != ku_len;
  nn_blen = bi_blen (nn_len, nn);
  ku_blen = bi_blen (ku_len, ku);
  invalid |= (nn_blen < MIN_N_BIT || ku_blen < MIN_K_BIT ||
	      !bi_lt(ku_len, ku, nn_len, nn));

  return invalid;
}

// return 1 if data is invalid
int invalid_d (int nn_len, unsigned long *nn, int d_len, unsigned long *data)
{
  int invalid, len, blen;
  invalid = (d_len <= 0 || d_len > BI_MAXLEN / 2);
  if (invalid) return 1;
  len = bi_len (d_len, data);
  blen = bi_blen (d_len, data);
  invalid = (len != d_len || !bi_lt(d_len, data, nn_len, nn));
  return invalid;
}

#ifndef SCROM_RSA
// RSA & EQ
int rsa_eq (int src_len, unsigned long *src, int dst_len, unsigned long *dst)
{
  int len, invalid;
  unsigned long data[BI_MAXLEN];

  // check valid of nn and ku
  invalid = invalid_nk (nn_len, nn, ku_len, ku);
  if (invalid) return -1;

  // check valid of src (assume the address is valid)
  invalid = invalid_d (nn_len, nn, src_len, src);
  if (invalid) return -2;

  // check valid of dst (assume the address is valid)
  invalid = invalid_d (nn_len, nn, dst_len, dst);
  if (invalid) return -3;

  // rsa
  len = bi_mon (src_len, src, ku_len, ku, nn_len, nn, data);
  // compare
  return bi_eq(dst_len, dst, len, data)? 0 : 1;
}

/**************************************************************/
/****** the following functions are not needed in BT-ROM ******/
/**************************************************************/

// add: A=A+b, return A's new len
int bi_add1 (int a_len, unsigned long *a_data, unsigned long b_d)
{
  unsigned long carry, sum;
  int i, len_min;

  if (a_len == 0) {
    a_data[0] = b_d;
    a_len = 1;
  } else {
    carry = b_d;
    for (i=0; i<a_len; i++) {
      sum = a_data[i] + carry;
      carry = sum < a_data[i];
      //printf("i=%d, sum=%lu, carry=%lu\n", i, sum, carry);
      a_data[i] = sum;
      if (carry==0) break;
    }
    if (carry) {
      a_data[a_len] = carry;
      a_len += carry;
    }
  }
  return a_len;
}

// string -> BI:A, return A's len, assume big endian
int bi_str2bi (char *str, int str_len, unsigned long *a_data, 
	       const unsigned int system)
{
  int i, a_len;   
  char ch;
  a_len = 0;
  for (i=0; i < str_len; i++) {  	   
    ch = str[i] - '0';
    if (system == 16 && ch > 9)
      ch = str[i] - 'a' + 10;

    if ((unsigned int)ch >= system) {
      printf("Error: unexpected char %c in %s, quit\n", str[i], str);
      exit (-1);
    }
    a_len = bi_add1(a_len, a_data, (unsigned long)ch);
    if (i != (str_len - 1))
      a_len = bi_mul1(a_len, a_data, (unsigned long)system, a_data);
  }

  return a_len;
}

// a = A % b, return a
unsigned long bi_mod1 (int a_len, unsigned long *a_data, unsigned long b_d)
{
  unsigned long long num;
  int i;

  num = 0;
  for (i=a_len-1; i>=0; i--) {
    num = (num << BIT_SHIFT) + a_data[i];
    num = num % b_d;
  }

  return (unsigned long)num;
}

// A = A / b, return A's new len
int bi_div1 (int a_len, unsigned long *a_data, unsigned long b_d)
{
  unsigned long long num;
  int i;

  num = 0;
  for (i=a_len-1; i>=0; i--) {
    num = ((unsigned long long)num << BIT_SHIFT) + a_data[i];
    a_data[i] = num / b_d;
    num = num % b_d;
  }

  for (i=a_len-1; i>=0; i--) {
    //printf("i=%d, data=%lu\n", i, a_data[i]);
    if (a_data[i] == 0) a_len--;
    else break;
  }

  return a_len;
}

// BI:A -> string, return the string length, little in DEC, big in HEX
int bi_bi2str (char *str, int a_len, unsigned long *a_data, 
	       const unsigned int system)
{
  unsigned long data[BI_MAXLEN];
  int i;
  char ch;

  if (system == 10) {
    for (i=0; i<a_len; i++) data[i] = a_data[i];
    i = 0;
    while(a_len > 0 && data[a_len-1] > 0) {
      ch = (char) bi_mod1(a_len, data, system) + 48;
      str[i] = ch;
      a_len = bi_div1(a_len, data, system);
      i++;
    }  
  } else if (system == 16) {
    int j, k;
    int start = 0;
    unsigned long d;
    i = 0;
    for (k=a_len-1; k>=0; k--) {
      for (j=7; j>=0; j--) {
	d = (a_data[k] >> (j*4)) & 0xf;
	if (!start && d != 0) start = 1;
	if (!start) continue;

	if (d < 10) ch = d + 48;
	else ch = d - 10 + 97;

	str[i] = ch;
	i++;
      }
    }
  } else {printf("Error: invalid system: %d, quit\n", system); exit(-1);}

  str[i] = 0;

  return i;   
}

// print A
void bi_print (char * name, int a_len, unsigned long *a_data, 
	       const unsigned int system)
{
  char result[5000];
  printf("%s: len=%d\n", name, a_len);
  bi_bi2str (result, a_len, a_data, system);
  printf("%s\n", result);
}
#if 0
int main()
{
	int n=0x4d7e8d;
	int pub=0x10001;
	int pri=0x391721;

	int data = 0x88;

	int out[32] = {0,};
	int out2[32] = {0,};

	int x = invalid_nk (1, &n,1, &pub);
	int y = invalid_d (1, &n, 1, &data);
	int z = invalid_nk (1, &n,1, &pri);

	printf("x,y,z=%d,%d,%d\n",x,y,z);

	int r = bi_mon (1, &data, 1, &pri, 1, &n, out);
	int i=0;
	for(i=0;i<r;i++)
		printf("%x\n",out[i]);

	x = bi_mon (r, out, 1, &pub, 1, &n, out2);
	for(i=0;i<x;i++)
		printf("%x\n",out2[i]);
}
#else
int do_rsa(int *n, int *key, int klen, int *input, int inlen, int *output)
{
	int i=0;

	int r = bi_mon (inlen, input, klen, key, klen, n, output);

	return 0;
}
#endif

#endif
