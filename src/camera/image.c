/***************************************************************************
 *   Copyright (C) 2012 by Tobias Müller                                   *
 *   Tobias_Mueller@twam.info                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <jpeglib.h>

#include "camera.h"

/**
	Convert from YUV420 format to YUV444.

	\param width width of image
	\param height height of image
	\param src source
	\param dst destination
*/
void YUV420toYUV444(int width, int height, unsigned char* src, unsigned char* dst) {
	int line;
	int column;
	// NOLINTNEXTLINE(readability-isolate-declaration)
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = dst;

	// In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	// Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	unsigned char *base_py = src;
	unsigned char *base_pu = src + (height * width);
	unsigned char *base_pv = src + (height * width) + (height * width) / 4;

	for (line = 0; line < height; line++) {
		for (column = 0; column < width; column++) {
			py = base_py + (line * width) + column;
			pu = base_pu + (line / 2 * width / 2) + column / 2;
			pv = base_pv + (line / 2 * width / 2) + column / 2;

			*tmp++ = *py;
			*tmp++ = *pu;
			*tmp++ = *pv;
		}
	}
}

/**
	Write image to jpeg file.

	\param img image to write
*/
void jpegWrite(uint8_t *img, camOpt co) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	FILE *outfile = fopen(co.jpegFilename, "wb");

	// try to open file for saving
	if (!outfile) {
		perror("jpeg");
	}

	// create jpeg data
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	// set image parameters
	cinfo.image_width = co.width;
	cinfo.image_height = co.height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;

	// set jpeg compression parameters to default
	jpeg_set_defaults(&cinfo);
	// and then adjust quality setting
	jpeg_set_quality(&cinfo, co.jpegQuality, TRUE);

	// start compress
	jpeg_start_compress(&cinfo, TRUE);

	// feed data
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &img[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// finish compression
	jpeg_finish_compress(&cinfo);

	// destroy jpeg data
	jpeg_destroy_compress(&cinfo);

	// close output file
	fclose(outfile);
}
