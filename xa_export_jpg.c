
#include <stdio.h>
#include <jpeglib.h>

#include "xa_export.h"

void jpg_encode_video(const char *output_path, int w, int h)
{
	char path[BUFSIZ];
	FILE *output;

	sprintf(path, output_path, export_data.frame, "jpg");
	output = fopen(path, "wb");
	if ( output ) {
		struct jpeg_error_mgr jerr;
		struct jpeg_compress_struct cinfo;

		/* Create the JPEG compression structure */
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);
		jpeg_stdio_dest(&cinfo, output);

		/* Set 24-bit RGB output with the requested quality */
		cinfo.image_width = w;
		cinfo.image_height = h;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, export_data.jpeg_quality, TRUE);

		/* Do it all! */
		jpeg_start_compress(&cinfo, TRUE);
		jpeg_write_scanlines(&cinfo, export_data.row_pointers, h);
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		fclose(output);
	}
}
