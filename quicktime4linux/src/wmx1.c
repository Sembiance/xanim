#include "quicktime.h"
#include "jpeglib.h"

#define ABS(x) ((x) < 0 ? -(x) : (x)) 

int quicktime_init_codec_wmx1(quicktime_video_map_t *vtrack)
{
	int i;
	quicktime_wmx1_codec_t *codec = &(vtrack->codecs.wmx1_codec);

	for(i = 0; i < WMX_CHUNK_FRAMES; i++)
		codec->frame_cache[i] = 0;

	codec->key_frame = 0;
	codec->keyframe_position = 0;
	codec->quality = 100;
	codec->use_float = 0;
	codec->frames_per_chunk = 0;
	codec->threshold = 5;
	return 0;
}

int quicktime_delete_codec_wmx1(quicktime_video_map_t *vtrack)
{
	int i;
	quicktime_wmx1_codec_t *codec = &(vtrack->codecs.wmx1_codec);

	for(i = 0; i < WMX_CHUNK_FRAMES; i++)
		if(codec->frame_cache[i]) free(codec->frame_cache[i]);

	if(codec->key_frame) free(codec->key_frame);
	return 0;
}

int wmx1_write_cache(quicktime_t *file, int track)
{
	long offset = quicktime_position(file);
	quicktime_video_map_t *vtrack = &(file->vtracks[track]);
	quicktime_wmx1_codec_t *codec = &(vtrack->codecs.wmx1_codec);
	int width = vtrack->track->tkhd.track_width;
	int height = vtrack->track->tkhd.track_height;
	int result = 0;
	int frame, i;
//	long bytes_per_row = width * 3 * codec->frames_per_chunk;
	long bytes_per_row = width * 3;
//	int step = 3 * codec->frames_per_chunk - 3;
	int step = codec->frames_per_chunk * bytes_per_row;
//	long bytes = height * bytes_per_row;
	long bytes = height * codec->frames_per_chunk * bytes_per_row;
	long output_bytes;

printf("wmx1_write_cache 1\n");
	if(!codec->key_frame)
	{
		codec->key_frame = malloc(bytes);
		if(!codec->key_frame) result = 1;
	}
printf("wmx1_write_cache 2\n");

// Interlace cached frames
// Try horizontally.
// 	for(frame = 0; frame < codec->frames_per_chunk; frame++)
// 	{
// 		unsigned char *input_frame = codec->frame_cache[frame];
// 		unsigned char *output_frame = codec->key_frame + frame * 3;
// 
// 		for(i = 0; i < height; i++)
// 		{
// 			unsigned char *input_row = input_frame + i * width * 3;
// 			unsigned char *output_row = output_frame + i * bytes_per_row;
// 			unsigned char *input_end = input_row + width * 3;
// 
// 			while(input_row < input_end)
// 			{
// 				*output_row++ = *input_row++;
// 				*output_row++ = *input_row++;
// 				*output_row++ = *input_row++;
// 				output_row += step;
// 			}
// 		}
// 	}
printf("wmx1_write_cache 3\n");

// Try vertically
	for(frame = 0; frame < codec->frames_per_chunk; frame++)
	{
		unsigned char *input_frame = codec->frame_cache[frame];
		unsigned char *output_frame = codec->key_frame + frame * bytes_per_row;

		for(i = 0; i < height; i++)
		{
			unsigned char *input_row = input_frame + i * width * 3;
			unsigned char *output_row = output_frame + i * step;
			unsigned char *input_end = input_row + width * 3;
			
			while(input_row < input_end)
			{
				*output_row++ = *input_row++;
			}
		}
	}

// Write as a jpeg
	{
		struct jpeg_compress_struct jpeg_compress;
		struct jpeg_error_mgr jpeg_error;
		JSAMPROW row_pointer[1];
		jpeg_compress.err = jpeg_std_error(&jpeg_error);
		jpeg_create_compress(&jpeg_compress);
		jpeg_stdio_dest(&jpeg_compress, quicktime_get_fd(file));
//		jpeg_compress.image_width = width * codec->frames_per_chunk;
		jpeg_compress.image_width = width;
//		jpeg_compress.image_height = height;
		jpeg_compress.image_height = height * codec->frames_per_chunk;
		jpeg_compress.input_components = 3;
		jpeg_compress.in_color_space = JCS_RGB;
		jpeg_set_defaults(&jpeg_compress);
		jpeg_set_quality(&jpeg_compress, codec->quality, 0);
		if(codec->use_float) jpeg_compress.dct_method = JDCT_FLOAT;
		jpeg_start_compress(&jpeg_compress, TRUE);
		while(jpeg_compress.next_scanline < jpeg_compress.image_height)
		{
			row_pointer[0] = codec->key_frame + jpeg_compress.next_scanline * bytes_per_row;
			jpeg_write_scanlines(&jpeg_compress, row_pointer, 1);
		}
		jpeg_finish_compress(&jpeg_compress);
		jpeg_destroy((j_common_ptr)&jpeg_compress);
	}
printf("wmx1_write_cache 4\n");

	output_bytes = quicktime_position(file) - offset;
	quicktime_update_tables(vtrack->track,
						offset,
						vtrack->current_chunk,
						vtrack->current_position,
						codec->frames_per_chunk,
						output_bytes);
printf("wmx1_write_cache 5\n");

	codec->frames_per_chunk = 0;
	vtrack->current_chunk++;
	return result;
}

int wmx1_store_in_cache(unsigned char *cache, unsigned char **row_pointers, int w, int h)
{
	int i;
	unsigned char *row_pointer1, *row_pointer2, *endpoint;

	for(i = 0; i < h; i++)
	{
		row_pointer1 = row_pointers[i];
		row_pointer2 = cache + i * w * 3;
		endpoint = row_pointer1 + w * 3;

		while(row_pointer1 < endpoint)
		{
			*row_pointer2++ = *row_pointer1++;
		}
	}
	return 0;
}

int quicktime_encode_wmx1(quicktime_t *file, unsigned char **row_pointers, int track)
{
	int result = 0;
	int written_cache = 0;
	quicktime_video_map_t *vtrack = &(file->vtracks[track]);
	quicktime_wmx1_codec_t *codec = &(vtrack->codecs.wmx1_codec);
	int width = vtrack->track->tkhd.track_width;
	int height = vtrack->track->tkhd.track_height;
	int bytes = width * height * 3;
	long frame_difference = 0;
	int i;

printf("quicktime_encode_wmx1 1\n");
// Compare the new frame against the last cached frame.
	if(codec->frames_per_chunk)
	{
		unsigned char *last_frame = codec->frame_cache[codec->frames_per_chunk - 1];
		unsigned char *row_pointer1, *row_pointer2, *endpoint;

		for(i = 0; i < height; i++)
		{
			row_pointer1 = row_pointers[i];
			row_pointer2 = last_frame + i * width * 3;
			endpoint = row_pointer1 + width * 3;
			
			while(row_pointer1 < endpoint)
			{
				if(ABS(*row_pointer1++ - *row_pointer2++) > codec->threshold) frame_difference++;
			}
		}
	}
printf("quicktime_encode_wmx1 2\n");

// Frames similar
	if(frame_difference < bytes / 4 || !codec->frames_per_chunk)
	{
		unsigned char *frame_cache;
		unsigned char *row_pointer1, *row_pointer2, *endpoint;
		if(!codec->frame_cache[codec->frames_per_chunk])
			codec->frame_cache[codec->frames_per_chunk] = malloc(bytes);
		frame_cache = codec->frame_cache[codec->frames_per_chunk];

// Copy to cache
		wmx1_store_in_cache(frame_cache, row_pointers, width, height);
		codec->frames_per_chunk++;
	}
	else
	{
// Frames different
// Write cache and store in new cache.
		unsigned char *frame_cache;

		result = wmx1_write_cache(file, track);
		written_cache = 1;

// Copy to cache
		if(!codec->frame_cache[codec->frames_per_chunk])
			codec->frame_cache[codec->frames_per_chunk] = malloc(bytes);
		frame_cache = codec->frame_cache[codec->frames_per_chunk];
		wmx1_store_in_cache(codec->frame_cache[codec->frames_per_chunk], row_pointers, width, height);
		codec->frames_per_chunk++;
	}
printf("quicktime_encode_wmx1 3\n");

// Cache full
	if(codec->frames_per_chunk == WMX_CHUNK_FRAMES)
	{
		result = wmx1_write_cache(file, track);
		written_cache = 1;
	}
printf("quicktime_encode_wmx1 4\n");

// 	if(!written_cache)
// 		quicktime_update_tables(vtrack->track,
// 						offset,
// 						vtrack->current_chunk,
// 						vtrack->current_position,
// 						codec->frames_per_chunk,
// 						output_bytes);
// 
	return result;
}

int quicktime_decode_wmx1(quicktime_t *file, unsigned char **row_pointers, int track)
{
	int result = 0;
	return result;
}
