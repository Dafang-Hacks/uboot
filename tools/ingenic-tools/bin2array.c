#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

enum {
	ONE_FILE_INPUT,
	MULT_FILE_INPUT
};


#define TEST       0
#define bufferLen  1024
char *data_string = "__attribute__ ((section(\".data\")));";
void help()
{
	printf("binary ----> array\n\n");
	printf
	    ("Interprets any file as plain binary data and dumps to a raw C array.\n");
	printf("usage: bin2array --one  <in-file> <out-file> [--arrayname <name-string> ] \n");
	printf("       bin2array --mult <in-file1> <in-file2> <in-file3> ...<in-fileN>  <out-file> [--arrayname <name-string>] \n\n");
}

int get_array_size(int argc, char *argv[])
{
	int file_size=0, fd_in, fsize;
	int i=0;
	for(i = 2; i < argc; i++ ){
		fd_in = open(argv[i], O_RDONLY);
		if (fd_in == -1) {
			printf("open input file error\n");
			return -1;
		}
		fsize = lseek(fd_in, 0, SEEK_END);
		lseek(fd_in, 0, SEEK_SET);
		close(fd_in);
		if(file_size < fsize)
			file_size = fsize;
	}
	return file_size;
}

int main(int argc, char *argv[])
{
	int fd_in, fd_out, in_fsize;
	char out_buffer[1024] = { 0 };
	int restart = 0;
	int i, j;
	unsigned char in_buffer[1024] = { 0 };
	int chunk;
	int mode = -1;
	int argc_count=0;
	char *name_str = NULL;
	/*printf("bin2array %s %s\n", argv[1], argv[2]);*/
	if (argc < 4) {
		help();
		return -1;
	}
	
	if (!strcmp(argv[1], "--one")){
		mode = ONE_FILE_INPUT;
	}else if (!strcmp(argv[1], "--mult")){
		mode = MULT_FILE_INPUT;
	}else{
		help();
		return -1;
	}

	if (!strcmp(argv[argc-2], "--arrayname")){
		argc_count = argc - 2;
		name_str = argv[argc-1];
	}
	else{
		argc_count = argc;
	}


	if(mode  == ONE_FILE_INPUT){
		if(name_str  == NULL){
			name_str = "rle_default_logo_addr";
		}
		
		fd_in = open(argv[2], O_RDONLY);
		if (fd_in == -1) {
			printf("open input file error\n");
			return -1;
		}
		in_fsize = lseek(fd_in, 0, SEEK_END);
		/*printf("----------------infilesize = %d\n", in_fsize);*/
		fd_out = open(argv[3], O_CREAT | O_RDWR | O_TRUNC,
				S_IRGRP | S_IROTH | S_IRUSR | S_IWUSR);
		if (fd_out == -1) {
			printf("please use the absolute file path, open ouput file error\n");
			return -1;
		}
		sprintf(out_buffer, "unsigned char %s [ %d ] %s\n", name_str,
				in_fsize, data_string);
		if (write(fd_out, out_buffer, strlen(out_buffer)) != strlen(out_buffer)) {
			printf("write outfile error\n");
			return -1;
		}
		sprintf(out_buffer, "unsigned char %s [ %d ] = {\n\t", name_str, in_fsize);
		if (write(fd_out, out_buffer, strlen(out_buffer)) != strlen(out_buffer)) {
			printf("write outfile error\n");
			return -1;
		}
		lseek(fd_in, 0, SEEK_SET);

#if TEST			//for debug
		int test;
		read(fd_in, &test, 1);
		printf("test  = %.2hx\n", test);
		close(fd_in);
		close(fd_out);
		return 0;

#endif /*  */
		for (i = 0; i < in_fsize;) {
			chunk = i + bufferLen < in_fsize ? bufferLen : in_fsize - i;
			read(fd_in, in_buffer, chunk);
			for (j = 0; j < chunk; ++j) {
				char outbuf[128];
				sprintf(outbuf, "0x%.2hX,", in_buffer[j]);
				if (write(fd_out, outbuf, strlen(outbuf)) != strlen(outbuf)) {
					printf("write data error\n");
					return -1;
				}
				++restart;
				if (restart > 0xf) {
					sprintf(outbuf, "\n\t");
					write(fd_out, outbuf, strlen(outbuf));
					restart = 0;
				}
			}
			i += chunk;
		}
		sprintf(out_buffer, "\n};\n");
		write(fd_out, out_buffer, strlen(out_buffer));
		chmod(argv[2], 0644);
		close(fd_in);
		close(fd_out);
	}
	else if (mode  == MULT_FILE_INPUT){
		if(name_str  == NULL){
			name_str = "rle_charge_logo_addr";
		}
		char *out_file = argv[argc_count-1];
		int k;
		//printf(" out_file  === %s\n", out_file);
	
		fd_out =
		open(out_file, O_CREAT | O_RDWR | O_TRUNC,
			 S_IRGRP | S_IROTH | S_IRUSR | S_IWUSR);
		if (fd_out == -1) {
			printf("please use the absolute file path, open ouput file error\n");
			return -1;
		}

		in_fsize = get_array_size(argc_count, argv);
		fd_in = open(argv[2], O_RDONLY);
		if (fd_in == -1) {
			printf("open input file error\n");
			return -1;
		}

		sprintf(out_buffer, "unsigned char %s [ %d ] [ %d ] %s\n", name_str,
			argc_count-3	, in_fsize, data_string);
		if (write(fd_out, out_buffer, strlen(out_buffer)) != strlen(out_buffer)) {
			printf("write outfile error\n");
			return -1;
		}

		sprintf(out_buffer, "unsigned char %s [ %d ] [ %d ] = {\n\t", name_str,
			argc_count-3, in_fsize);
		if (write(fd_out, out_buffer, strlen(out_buffer)) != strlen(out_buffer)) {
			printf("write outfile error\n");
			return -1;
		}

		sprintf(out_buffer, "{\n\t");
		write(fd_out, out_buffer, strlen(out_buffer));
		for(k=2; k < argc_count-1; ){
			lseek(fd_in, 0, SEEK_SET);

			for (i = 0; i < in_fsize;) {
				chunk = i + bufferLen < in_fsize ? bufferLen : in_fsize - i;
				read(fd_in, in_buffer, chunk);
				for (j = 0; j < chunk; ++j) {
					char outbuf[128];
					sprintf(outbuf, "0x%.2hX,", in_buffer[j]);
					if (write(fd_out, outbuf, strlen(outbuf)) != strlen(outbuf)) {
						printf("write data error\n");
						return -1;
					}
					++restart;
					if (restart > 0xf) {
						sprintf(outbuf, "\n\t");
						write(fd_out, outbuf, strlen(outbuf));
						restart = 0;
					}
				}
				i += chunk;
			}
			close(fd_in);
			k++;
			if(k < argc_count-1){
				fd_in = open(argv[k], O_RDONLY);
				if (fd_in == -1) {
					printf("open input file error\n");
					return -1;
				}
				restart = 0;
				sprintf(out_buffer, "\n\t},\n\t{\n\t");
				write(fd_out, out_buffer, strlen(out_buffer));
				restart = 0;
			}else{
				sprintf(out_buffer, "\n\t}\n");
				write(fd_out, out_buffer, strlen(out_buffer));
			}
	
	
		}
		sprintf(out_buffer, "\n};\n");
		write(fd_out, out_buffer, strlen(out_buffer));
		chmod(argv[2], 0644);
		close(fd_out);
		
	}	


	return 0;
}
