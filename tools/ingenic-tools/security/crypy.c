/*
 * crypy.c
 *
 * Copyright (C) 2014 ingenic Electronics
 * author: cli  <cli@ingenic.cn>
 * baseon: spl.c  hpwang<hpwang@ingenic.cn>
 * crypy part author(rsa aes sha1): jzhang <jzhang@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG
static char optstring[] = "+hVo:i:r:a:l:m:s:t:";
static struct option long_options[] = {
	{ "help", 0, 0, 'h' },
	{ "Version", 0, 0, 'V' },
	{ "output", 1, 0, 'o' },
	{ "input", 1, 0, 'i' },
	{ "aes-key", 1, 0, 'a' },
	{ "rsa-Key", 1, 0, 'r'},
	{ "rsa-len", 1, 0, 'l'},			//in bit
	{ "crypt-mode", 1, 0, 'm'},
	{ "skip-sz", 1, 0, 's'},			//in k
	{ "title-sz", 1, 0, 't'},			//in 512byte
	{0, 0, 0, 0}
};

static void print_usage(void)
{
	printf("-h --help\t\tprint help message\n");
	printf("-V --Version\t\tprint version  message\n");
	printf("-i --input\t\tinput file\n");
	printf("-o --output\t\toutput file\n");
	printf("-a --aes-key\t\taes key file\n");
	printf("-r --rsa-key\t\trsa key file\n");
	printf("-l --rsa-len\t\trsa N length in bit\n");
	printf("-m --crypt-mode\t\tcrypy mode bit 0: 0:file no crypt 1:file crypt\n");
	printf("\t\t\t\t\tbit 1:  0:key no crypt 1:key crypt\n");
	printf("\t\t\t\t\tbit 2:  0:use chip key 1:use user key\n");
	printf("\t\t\t\t\tbit 3:  0:no spl 1:is spl\n");
	printf("-s --skip-sz\t\tinput file skip size from head to crypy in KB\n");
	printf("-t --title-sz\t\tinput title size of head to reserve in 512Byte\n");
}

static int str2hex(const char *p, unsigned int *hex)
{
	int i = 0;
	int ret = 0;
	for (; i < 8; i++) {
	//	printf("p[%d] = %c\n", i, p[i]);
		*hex = *hex * 16;
		if (p[i] >= '0' && p[i] <= '9')
			*hex += (p[i] - '0');
		else if (p[i] >= 'a' && p[i] <= 'f')
			*hex += (p[i] - 'a' + 10);
		else if (p[i] >= 'A' && p[i] <= 'F')
			*hex += (p[i] - 'A' + 10);
		else
			ret = -1;
	//	printf("result = %x\n", *hex);
	}
	return ret;
}


#define LINE_MAX 1024
int main(int argc, char *argv[]) {


	char in_file[128];
	char out_file[128];
	char aesk_file[128];
	char rsak_file[128];
	char line[LINE_MAX];
	int filesha[5] = {0,};
	int filesharsa[68];
	int padding[128];		//512 byte padding
	FILE *infp = NULL;
	FILE *outfp = NULL;
	FILE *aesk_fp = NULL;
	FILE *rsak_fp = NULL;
	struct stat in_stat;
	unsigned crypt_mode = 0;
	unsigned infile_len = 0;
	unsigned file_crypt_mode = 0;	//ck = 1, uk = 2, nocrypt = 0
	unsigned rsa_len = 0;		//length in byte
	unsigned rsa_bit_len = 0;	//length in bit
	unsigned key_crypt_mode = 0;	//ck = 1, uk = 2, nocrypt = 0
	unsigned skip_size = 0;
	unsigned title_size = 0;
	unsigned is_spl = 0;		//is spl or not
	unsigned int *tmp_buf = NULL;
	unsigned int *rsa_key = NULL;
	unsigned int *rsa_nkey = NULL;
	unsigned int *rsa_kukey = NULL;
	unsigned int *rsa_krkey = NULL;
	unsigned int aes_key[4];
	int ret = 0;
	int c = -1;
	int i = 0;
	int endi = 0;

	memset(in_file, 0, sizeof(in_file));
	memset(out_file, 0, sizeof(out_file));
	memset(aesk_file, 0, sizeof(aesk_file));
	memset(rsak_file, 0, sizeof(rsak_file));
	memset(padding , 0, 512);

	while(1) {
		c = getopt_long(argc, argv, optstring, long_options, NULL);
		if (c == -1)
			break;
		switch(c) {
			case 'o':
				if (strlen(optarg) > 127) {
					printf("file name is too long max is 127: %s\n", optarg);
					return 127;
				}
				strncpy(out_file, optarg, strlen(optarg));
				break;
			case 'i':
				if (strlen(optarg) > 127) {
					printf("file name is too long max is 127: %s\n", optarg);
					return 127;
				}
				strncpy(in_file, optarg, strlen(optarg));
				break;
			case 'a':
				if (strlen(optarg) > 127) {
					printf("file name is too long max is 127: %s\n", optarg);
					return 127;
				}
				strncpy(aesk_file, optarg, strlen(optarg));
				break;
			case 'r':
				if (strlen(optarg) > 127) {
					printf("file name is too long max is 127: %s\n", optarg);
					return 127;
				}
				strncpy(rsak_file, optarg, strlen(optarg));
				break;
			case 'l':
				rsa_bit_len = atoi(optarg);
				rsa_len = rsa_bit_len/8;
				break;
			case 's':
				skip_size = atoi(optarg);
				skip_size = skip_size * 1024;
				break;
			case 't':
				title_size = atoi(optarg);
				title_size = title_size * 512;
				break;
			case 'm':
#define CRYPY	0x1
#define	USE_CK	0x2
#define NO_SPL	0x3	//for future FIXME
				crypt_mode = atoi(optarg);
				printf("crypy mode %x\n", crypt_mode);
				if (crypt_mode & NO_SPL)
					is_spl = 0;
				else
					is_spl = 1;
				if (crypt_mode & CRYPY) {
					if (crypt_mode & USE_CK)
						file_crypt_mode = 1;
					else
						file_crypt_mode = 2;
				}
				if (crypt_mode & CRYPY) {
					if (crypt_mode & USE_CK)
						key_crypt_mode = 1;
					else
						key_crypt_mode = 2;
				}
				break;
			default:
			case '?':
				printf("Unkown params %s\n", optarg);
				break;
			case 'h':
				goto print_usage;
				break;
		}
	}

	if (stat(in_file, &in_stat)) {
		printf("stat input file %s, error is %s\n",
				in_file ,strerror(errno));
		return -1;
	}
	infile_len = in_stat.st_size;
	if (infile_len <= skip_size) {
		printf("skip size %d is greater than infile length %d",
				skip_size, infile_len);
		return -1;
	}
	infile_len -= skip_size;

#define DEF_SIGNATURE_SZ	512
	if (skip_size && !title_size)	title_size = DEF_SIGNATURE_SZ;

	if (!(infp = fopen(in_file, "r"))) {
		printf("fopen in file %s fail,error is %s\n", in_file, strerror(errno));
		return -1;
	}
	fseek(infp, 0, SEEK_SET);

	if (!(rsak_fp = fopen(rsak_file, "r"))) {
		printf("fopen rsa file %s fail,error is %s\n", rsak_file, strerror(errno));
		fclose(infp);
		return -1;
	}
	fseek(rsak_fp, 0, SEEK_SET);

#define DEF_OUT_FILE	"dstfile"
	if (!(outfp = fopen(out_file, "w+")) && !(outfp = fopen(DEF_OUT_FILE, "w+"))) {
		printf("fopen out file %s fail,error is %s\n", DEF_OUT_FILE, strerror(errno));
	}
	fseek(outfp, 0, SEEK_SET);

	if (!(aesk_fp = fopen(aesk_file, "r"))) {
		file_crypt_mode = 0;
		key_crypt_mode = 0;
	} else {
		fseek(aesk_fp, 0, SEEK_SET);
	}

	if (!rsa_len) {
#define DEF_RSA_KEY_SZ 124
		rsa_len = DEF_RSA_KEY_SZ;
		rsa_bit_len = 8*(DEF_RSA_KEY_SZ);
	}

	/*READ RSA KEY*/
	rsa_key = (unsigned *)malloc(3 *rsa_len);
	rsa_nkey = rsa_key;
	rsa_kukey = rsa_nkey + rsa_len/sizeof(unsigned);
	rsa_krkey = rsa_kukey + rsa_len/sizeof(unsigned);
	while (fgets(line, LINE_MAX, rsak_fp) != NULL) {
		char *p = line;
		if (!strncmp(line, "rsan", 4)) {
			tmp_buf = rsa_nkey;
			endi = rsa_len/sizeof(unsigned);
			i = 0;
		}
		if (!strncmp(line, "rsaku", 5)) {
			tmp_buf = rsa_kukey;
			endi = rsa_len/sizeof(unsigned);
			i = 0;
		}
		if (!strncmp(line, "rsakr", 5)) {
			tmp_buf =  rsa_krkey;
			endi = rsa_len/sizeof(unsigned);
			i = 0;
		}
		if (i == endi)
			continue;
		while ((p = strstr(p, "0x"))) {
			p += 2;
			if (str2hex(p, (tmp_buf + i))) {
				printf("error read rsa file %s\n", p);
				return -1;
			}
			p += 8;
			i++;
		}
	}

	/*READ AES KEY*/
	if (aesk_fp) {
		i = 0;
		while (fgets(line, LINE_MAX, aesk_fp) != NULL || i < 4) {
			char *p = line;
			while ((p = strstr(p, "0x")) || i < 4) {
				p += 2;
				if (str2hex(p, (aes_key + i))) {
					printf("error read rsa file %s\n", p);
					return -1;
				}
				p += 8;
				i++;
			}
		}
#ifdef DEBUG
		printf("aes key is :");
		for (i = 0; i < 4; i++) {
			if (i%4 == 0)
				printf("\n");
			printf("0x%08x\t",aes_key[i]);
		}
		printf("\n");
#endif
	}

#ifdef DEBUG
	printf("rsa n is :");
	for (i = 0; i < rsa_len/sizeof(unsigned); i++) {
		if (i%4 == 0)
			printf("\n");
		printf("0x%08x\t",rsa_nkey[i]);
	}
	printf("\n");
	printf("rsa ku is :");
	for (i = 0; i < rsa_len/sizeof(unsigned); i++) {
		if (i%4 == 0)
			printf("\n");
		printf("0x%08x\t",rsa_kukey[i]);
	}
	printf("\n");
	printf("rsa kr is :");
	for (i = 0; i < rsa_len/sizeof(unsigned); i++) {
		if (i%4 == 0)
			printf("\n");
		printf("0x%08x\t",rsa_krkey[i]);
	}
	printf("\n");
#endif

	/*PART1: FILL title like signature*/
	if (title_size) {
		tmp_buf = malloc(title_size);
		if (tmp_buf == NULL) {
			printf("malloc %d byte is failed\n", title_size);
			return -1;
		}
		fread(tmp_buf, title_size , 1, infp);
#ifdef DEBUG
		printf("signature is :");
		for (i = 0; i < title_size/4; i++) {
			if (i%8 == 0)
				printf("\n");
			printf("0x%08x\t", tmp_buf[i]);
		}
		printf("\n");
#endif
		fwrite(tmp_buf, title_size, 1, outfp);
		free(tmp_buf);
	}

	/*PART2: Fill SPL params*/
	fwrite(&infile_len, 4, 1, outfp); //spl bin len
	fwrite(&file_crypt_mode, 4, 1, outfp); //spl bin crypt flag
	fwrite(&rsa_bit_len, 4, 1, outfp); //rsa bit num
	fwrite(&rsa_bit_len, 4, 1, outfp);  //ku bit num
	fwrite(&key_crypt_mode, 4, 1, outfp); //spl key crypt flag
	fwrite(padding , 4, 59 , outfp); //pending to 2048 bit

	/*PART3: Fill rsa n, ku KEY*/
	switch (key_crypt_mode) {
	case 1:
	case 2:{
		unsigned group = 0;
		group = rsa_len/16;
		group = rsa_len%16 ? group + 1 : group;
		tmp_buf = (unsigned *)malloc(16 * group);
		memset((void *)tmp_buf, 0, group* 16);
		for (i = 0; i < rsa_len/sizeof(unsigned); i++) {
			tmp_buf[i] = rsa_nkey[i];
		}
		do_aes(aes_key, tmp_buf , group*16);
		fwrite(tmp_buf, sizeof(unsigned),
				(group *16)/sizeof(unsigned),
				outfp);
		fwrite(padding, 4,
				64 -((group *16)/sizeof(unsigned)),
				outfp);			//pending to 2048 bit

		memset((void *)tmp_buf, 0, group*16);
		for (i = 0; i < rsa_len/sizeof(unsigned); i++) {
			tmp_buf[i] = rsa_kukey[i];
		}
		do_aes(aes_key, tmp_buf, group*16);
		fwrite(tmp_buf, sizeof(unsigned),
				(group *16)/sizeof(unsigned),
				outfp);

		fwrite(padding, 4,
				64 -((group *16)/sizeof(unsigned)),
				outfp);			//pending to 2048 bit
		free(tmp_buf);
	       }
	       break;
	case 0:
		fwrite(rsa_nkey, 4, rsa_len/sizeof(unsigned) , outfp);
		fwrite(padding, 4, 64 - rsa_len/sizeof(unsigned) , outfp);
		fwrite(rsa_kukey, 4, rsa_len/sizeof(unsigned) , outfp);
		fwrite(padding, 4, 64 - rsa_len/sizeof(unsigned) , outfp);
		break;
	}

	/*PART4: Fill sha1*/
	if (skip_size) {
		fseek(infp, skip_size, SEEK_SET);
	}
	tmp_buf = malloc(infile_len);
	memset(tmp_buf, 0 , infile_len);
	fread(tmp_buf , infile_len, 1, infp);
#ifdef DEBUG
	printf("tmp_buf (%p)%x len = %d\n", tmp_buf, *tmp_buf, infile_len);
#endif
	do_sha1_1(tmp_buf, infile_len);
	do_sha1_2(filesha);
#ifdef DEBUG
	for (i = 0; i < 5; i++)
		printf("splsha1 0x%08x\n", filesha[i]);
#endif
	memset(filesharsa, 0, 68*4);
	do_rsa(rsa_nkey, rsa_krkey, rsa_len/sizeof(unsigned), filesha, 5, filesharsa);
	for (i = 0; i < rsa_len; i++){
		printf("rsa 0x%08x\n", filesharsa[i]);
	}
	fwrite(filesharsa, sizeof(unsigned), rsa_len/sizeof(unsigned), outfp); //spl src sha1 kr enc
	fwrite(padding, sizeof(unsigned), 64-rsa_len/sizeof(unsigned), outfp); //pending to 2048 bit
	fwrite(padding, sizeof(unsigned), 128, outfp); //pending for last 512 byte


	if (file_crypt_mode) {
		do_aes(aes_key, tmp_buf, infile_len);
	}
	printf("tmp_buf (%p)%x len = %d\n", tmp_buf, *tmp_buf, infile_len);
	fwrite(tmp_buf, sizeof(unsigned), infile_len/sizeof(unsigned), outfp);

	free(tmp_buf);
	free(rsa_key);
	fclose(infp);
	fclose(outfp);
	fclose(aesk_fp);
	fclose(rsak_fp);
	return 0;
print_usage:
	print_usage();
	return  -1;
}
