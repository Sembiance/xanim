/*-------------------------
Export code by
Adam Williams
broadcast@earthling.net
---------------------------*/





#include <stdio.h>
#include <signal.h>
#include <jpeglib.h>
#include "quicktime.h"
#include "xa_export.h"

xa_exportable export_data;

int xa_export_init()
{
	export_data.export = 0;
	export_data.raw_data = 0;
	export_data.jpeg_quality = 100;
	export_data.row_pointers = 0;
#ifdef XA_EXPORT_QT
	export_data.file = 0;
	xa_export_set_path("exported.mov");
#else
	xa_export_set_path("xanimf%08d.%s");
#endif
	xa_export_set_vid_compressor(QUICKTIME_RAW);
	xa_export_set_aud_compressor(QUICKTIME_TWOS);
}

int xa_export_end()
{
#ifdef XA_EXPORT_QT
	if(export_data.file) quicktime_close(export_data.file);
#endif
	if(export_data.raw_data) free(export_data.raw_data);
	if(export_data.row_pointers) free(export_data.row_pointers);
}

int xa_export_set_path(char *path)
{
	strcpy(export_data.path, path);
}

int xa_export_set_vid_compressor(char *compressor)
{
	export_data.vid_compressor[0] = compressor[0];
	export_data.vid_compressor[1] = compressor[1];
	export_data.vid_compressor[2] = compressor[2];
	export_data.vid_compressor[3] = compressor[3];
}

int xa_export_match_vid_compressor(char *test)
{
	if(test[0] == export_data.vid_compressor[0] &&
       test[1] == export_data.vid_compressor[1] &&
       test[2] == export_data.vid_compressor[2] &&
       test[3] == export_data.vid_compressor[3])
		return 1;
	else
		return 0;
}

char* xa_export_int_to_codec(int t)
{
	switch(t)
	{
		case 0:
			return QUICKTIME_RAW;
			break;
		case 1:
			return QUICKTIME_JPEG;
			break;
		case 2:
			return QUICKTIME_YUV2;
			break;
		case 3:
			return QUICKTIME_YUV4;
			break;
		default:
			return QUICKTIME_RAW;
			break;
	}
}

int xa_export_set_aud_compressor(char *compressor)
{
	export_data.aud_compressor[0] = compressor[0];
	export_data.aud_compressor[1] = compressor[1];
	export_data.aud_compressor[2] = compressor[2];
	export_data.aud_compressor[3] = compressor[3];
}

int xa_export_ximage(XImage * image)
{
	char path[BUFSIZ];
	FILE *fp;
	unsigned long red_mask = image->red_mask;
	unsigned long green_mask = image->green_mask;
	unsigned long blue_mask = image->blue_mask;

	sprintf(path, export_data.path, export_data.frame, "ppm");
	fp = fopen(path, "wb");
	if(fp)
	{
		fprintf(fp, "P6\n%d %d\n255\n", image->width, image->height);
		for (int y = 0; y < image->height; y++)
		{
			for (int x = 0; x < image->width ; x++)
			{
				unsigned char color[3];
				unsigned long pixel = XGetPixel(image, x, y);
				unsigned char blue = pixel & blue_mask;
				unsigned char green = (pixel & green_mask) >> 8;
				unsigned char red = (pixel & red_mask) >> 16;

				color[0] = red;
				color[1] = green;
				color[2] = blue;
				fwrite(color, 1, 3, fp);
			}
		}
		fclose(fp);
	}
	
	++export_data.frame;
	return 0;
}

int xa_export_frame(unsigned char *data, int w, int h)
{
	int i, doexport;

/* get row pointers for the output buffer */
	if(!export_data.raw_data)
		export_data.raw_data = malloc(w * 3 * h);

	if(!export_data.row_pointers)
	{
		export_data.row_pointers = malloc(sizeof(unsigned char*) * h);
		for(i = 0; i < h; i++)
		{
			export_data.row_pointers[i] = &(export_data.raw_data[i * w * 3]);
		}
	}

	if(export_data.depth == 16)
	{
/* 16 to 24 */
		for(i = 0; i < h; i++)
		{
			xa_export_16to24_row((unsigned short*)(&data[i * w * 2]), export_data.row_pointers[i], w);
		}
		doexport = 1;
	}
	else
	if(export_data.depth == 24 && export_data.bits_per_pixel == 32)
	{
/* 32 to 24 */
		for(i = 0; i < h; i++)
		{
			xa_export_32to24_row(&data[i * w * 4], export_data.row_pointers[i], w);
		}
		doexport = 1;
	}
	else
	{
		printf("xa_export: unsupported color depth. depth [%d] and bits_per_pixel [%d]\n", export_data.depth, export_data.bits_per_pixel);
		doexport = 0;
	}
	if ( doexport ) {
#ifdef XA_EXPORT_QT
		quicktime_encode_video(export_data.file, export_data.row_pointers, 0);
#else
        if ( xa_export_match_vid_compressor(QUICKTIME_RAW) ) {
		    ppm_encode_video(export_data.path, w, h);
        }
        if ( xa_export_match_vid_compressor(QUICKTIME_JPEG) ) {
		    jpg_encode_video(export_data.path, w, h);
        }
#endif
	}
	++export_data.frame;
}

int xa_export_16to24_row(unsigned short *input, unsigned char *output, int w)
{
	unsigned short *input_end = input + w;

	while(input < input_end)
	{
		*output++ = (*input & 0xf800) >> 8;
		*output++ = (*input & 0x7e0) >> 3;
		*output++ = (*input & 0x1f) << 3;
		input++;
	}
}

int xa_export_32to24_row(unsigned char *input, unsigned char *output, int w)
{
	unsigned char *input_end = input + w * 4;

	while(input < input_end)
	{
// Big endian input
		input++;
		*output++ = *input++;
		*output++ = *input++;
		*output++ = *input++;
	}
}
