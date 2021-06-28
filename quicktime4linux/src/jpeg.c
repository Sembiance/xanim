#include "quicktime.h"
#include "jpeglib.h"

int quicktime_init_codec_jpeg(quicktime_video_map_t *vtrack)
{
	vtrack->codecs.jpeg_codec.quality = 100;
	vtrack->codecs.jpeg_codec.use_float = 0;
}

int quicktime_delete_codec_jpeg(quicktime_video_map_t *vtrack)
{
}

int quicktime_set_jpeg(quicktime_t *file, int quality, int use_float)
{
	int i;
	char *compressor;

	for(i = 0; i < file->total_vtracks; i++)
	{
		if(quicktime_match_32(quicktime_video_compressor(file, i), QUICKTIME_JPEG))
		{
			quicktime_jpeg_codec_t *codec = &(file->vtracks[i].codecs.jpeg_codec);
			codec->quality = quality;
			codec->use_float = use_float;
		}
	}
}

int quicktime_decode_jpeg(quicktime_t *file, unsigned char **row_pointers, int track)
{
	int result = 0;
	struct jpeg_decompress_struct jpeg_decompress;
	struct jpeg_error_mgr jpeg_error;
	long i, color_channels, bytes;
	quicktime_trak_t *trak = file->vtracks[track].track;

	jpeg_decompress.err = jpeg_std_error(&jpeg_error);
	jpeg_create_decompress(&jpeg_decompress);

	if(!file->vtracks[track].frames_cached)
	{
		quicktime_set_video_position(file, file->vtracks[track].current_position, track);
		jpeg_stdio_src(&jpeg_decompress, quicktime_get_fd(file));
	}
	else
	{
		jpeg_buffer_src(&jpeg_decompress, 
			file->vtracks[track].frame_cache[file->vtracks[track].current_position],
			quicktime_frame_size(file, file->vtracks[track].current_position, track));
	}

	jpeg_read_header(&jpeg_decompress, TRUE);
	jpeg_start_decompress(&jpeg_decompress);
	color_channels = jpeg_decompress.jpeg_color_space;
	row_pointers = (JSAMPROW*)row_pointers;

// read the image
	while(jpeg_decompress.output_scanline < jpeg_decompress.output_height)
	{
		jpeg_read_scanlines(&jpeg_decompress, 
			(JSAMPROW*)&row_pointers[jpeg_decompress.output_scanline], 
			jpeg_decompress.output_height - jpeg_decompress.output_scanline);
	}
	jpeg_finish_decompress(&jpeg_decompress);
	jpeg_destroy((j_common_ptr)&jpeg_decompress);

	return result;
}

int quicktime_encode_jpeg(quicktime_t *file, unsigned char **row_pointers, int track)
{
	long offset = quicktime_position(file);
	int result = 0;
	int i;
	quicktime_trak_t *trak = file->vtracks[track].track;
	int height = trak->tkhd.track_height;
	int width = trak->tkhd.track_width;
	struct jpeg_compress_struct jpeg_compress;
	struct jpeg_error_mgr jpeg_error;
	JSAMPROW row_pointer[1];
	long bytes;

	jpeg_compress.err = jpeg_std_error(&jpeg_error);
	jpeg_create_compress(&jpeg_compress);
	jpeg_stdio_dest(&jpeg_compress, quicktime_get_fd(file));
	jpeg_compress.image_width = width;
	jpeg_compress.image_height = height;
	jpeg_compress.input_components = 3;
	jpeg_compress.in_color_space = JCS_RGB;
	jpeg_set_defaults(&jpeg_compress);
	jpeg_set_quality(&jpeg_compress, file->vtracks[track].codecs.jpeg_codec.quality, 0);
	if(file->vtracks[track].codecs.jpeg_codec.use_float)
		jpeg_compress.dct_method = JDCT_FLOAT;

	jpeg_start_compress(&jpeg_compress, TRUE);
	while(jpeg_compress.next_scanline < jpeg_compress.image_height)
	{
		row_pointer[0] = row_pointers[jpeg_compress.next_scanline];
		jpeg_write_scanlines(&jpeg_compress, row_pointer, 1);
	}
	jpeg_finish_compress(&jpeg_compress);
	jpeg_destroy((j_common_ptr)&jpeg_compress);

	bytes = quicktime_position(file) - offset;
	quicktime_update_tables(file->vtracks[track].track,
						offset,
						file->vtracks[track].current_chunk,
						file->vtracks[track].current_position,
						1,
						bytes);

	file->vtracks[track].current_chunk++;
	return result;
}
