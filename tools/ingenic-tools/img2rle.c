/*
 * File:    jpeg2rle.c
 * Author:  Li XianJing <xianjimli@hotmail.com>
 * Brief:   convert jpeg to rle.
 *
 * Copyright (c) 2009  Li XianJing <xianjimli@hotmail.com>
 *
 * Licensed under the Academic Free License version 2.1
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

/*
 * History:
 * ================================================================
 * 2009-08-16 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <stdio.h>
#include <jpeglib.h>
//#include <setjmp.h>
#define PNG_SKIP_SETJMP_CHECK
#include <png.h>

#define to565(r,g,b)                                            \
		((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	(*cinfo->err->output_message) (cinfo);

	longjmp(myerr->setjmp_buffer, 1);
}

void put_scanline_to_file(FILE * outfile, char *scanline, int width,
			  int row_stride)
{
	int i = 0;
	char *pixels = scanline;
	char *start = scanline;

	while (i < width) {
		unsigned short n = 0;
		for (; i < width; i++, n++) {
			if (memcmp(pixels, pixels + 3 * n, 3) != 0) {
				break;
			}
		}
		unsigned char r = pixels[0];
		unsigned char g = pixels[1];
		unsigned char b = pixels[2];
		unsigned short rle565 =
		    ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

		fwrite(&n, 2, 1, outfile);
		fwrite(&rle565, 2, 1, outfile);
		pixels += 3 * n;
	}

	return;
}

int jpeg2rle(char *filename, char *outfilename)
{
	struct jpeg_decompress_struct cinfo;
	int i = 0;
	struct my_error_mgr jerr;
	FILE *infile;
	FILE *outfile;
	JSAMPARRAY buffer;
	int row_stride;

	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return 0;
	}

	if ((outfile = fopen(outfilename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", outfilename);
		fclose(infile);
		return 0;
	}

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		fclose(outfile);
		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);

	(void)jpeg_read_header(&cinfo, TRUE);

	(void)jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
	    ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);
	//printf("size: %dx%d\n", cinfo.output_width, cinfo.output_height);
	fwrite(&cinfo.output_width, 2, 1, outfile);	//�洢ͼƬ���
	fwrite(&cinfo.output_height, 2, 1, outfile);	//�洢ͼƬ�߶�
	int outsize = 0x0;
	fwrite(&outsize, 4, 1, outfile);	//�洢ͼƬ�߶�
	while (cinfo.output_scanline < cinfo.output_height) {
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);
		put_scanline_to_file(outfile, buffer[0], cinfo.output_width,
				     row_stride);
	}

	(void)jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	outsize = fseek(outfile, 0, SEEK_END);
	outsize = ftell(outfile);
	outsize = outsize - 8;
//      printf("outsize = %d\n", outsize);
	fseek(outfile, 4, SEEK_SET);
	fwrite(&outsize, 4, 1, outfile);	//�洢ͼƬ�߶�

	fclose(infile);
	fclose(outfile);

	return 1;
}

int png2rle(char *infilename, char *outfilename)
{
	FILE *infile = NULL;
	FILE *outfile = NULL;
	if ((infile = fopen(infilename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", infilename);
		return 0;
	}

	if ((outfile = fopen(outfilename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", outfilename);
		fclose(infile);
		return 0;
	}

	png_structp png_ptr =
	    png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr));

	png_init_io(png_ptr, infile);

	// read the png file
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);

	fwrite(&width, 2, 1, outfile);	//�洢ͼƬ���
	fwrite(&height, 2, 1, outfile);	//�洢ͼƬ�߶�

	int color_type = png_get_color_type(png_ptr, info_ptr);

	//printf("PNG Width=%d Height=%d ColorType=%d \n",width, height, color_type);

	int size = height * width * 4;

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

	int i = 0;
	int j = 0;
	unsigned short last, color, count;
	unsigned total = 0;
	count = 0;
	for (; i < height; i++) {
		for (j = 0; j < (4 * width); j += 4) {
			//printf("R=%d G=%d B=%d Position=%d\n",row_pointers[i][j],row_pointers[i][j + 1],row_pointers[i][j + 2],pos);
			color =
			    to565(row_pointers[i][j], row_pointers[i][j + 1],
				  row_pointers[i][j + 2]);
			if (count) {
				if ((color == last) && (count != 65535)) {
					count++;
					continue;
				} else {
					fwrite(&count, 2, 1, outfile);
					fwrite(&last, 2, 1, outfile);
					total += count;
				}
			}
			last = color;
			count = 1;
		}
	}
	if (count) {
		fwrite(&count, 2, 1, outfile);
		fwrite(&last, 2, 1, outfile);
		total += count;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	fclose(infile);
	fclose(outfile);
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("*******************************************\n");
		printf("usage: %s <jpeg/png>file <rle>file\n", argv[0]);
		printf("Powered by figofuture\n");
		printf("*******************************************\n");
	} else {
		char *extname = strrchr(argv[1], '.');
		if (!strcasecmp(extname + 1, "jpg")
		    || !strcasecmp(extname + 1, "jpeg"))
			jpeg2rle(argv[1], argv[2]);
		else if (!strcasecmp(extname + 1, "png"))
			png2rle(argv[1], argv[2]);
		else
			printf("Unsupported file format!\n");
	}

	return 0;
}
