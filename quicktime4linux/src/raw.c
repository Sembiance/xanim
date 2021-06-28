#include "quicktime.h"

int quicktime_init_codec_raw(quicktime_video_map_t *vtrack)
{
}

int quicktime_delete_codec_raw(quicktime_video_map_t *vtrack)
{
}

int quicktime_decode_raw(quicktime_t *file, unsigned char **row_pointers, int track)
{
	int i, j, height, width, consecutive_rows;
	int result = 0;
	quicktime_trak_t *trak = file->vtracks[track].track;

	height = trak->tkhd.track_height;
	width = trak->tkhd.track_width;

	if(!file->vtracks[track].frames_cached)
	{
// read a frame from disk
		quicktime_set_video_position(file, file->vtracks[track].current_position, track);

		if(quicktime_raw_rows_consecutive(row_pointers, width, height))
		{
			long bytes = quicktime_frame_size(file, file->vtracks[track].current_position, track);
			result = quicktime_read_data(file, row_pointers[0], bytes);
			if(result) result = 0; else result = 1;
		}
		else
		for(i = 0; i < height && !result; i++)
		{
			result = quicktime_read_data(file, row_pointers[i], width * 3);
			if(result) result = 0; else result = 1;
		}
	}
	else
	{
// copy a frame from cache
		unsigned char *buffer = file->vtracks[track].frame_cache[file->vtracks[track].current_position];

		for(i = 0; i < height && !result; i++)
		{
			for(j = 0; j < width * 3; j++)
			{
				row_pointers[i][j] = *buffer++;
			}
		}
	}

	return result;
}

int quicktime_encode_raw(quicktime_t *file, unsigned char **row_pointers, int track)
{
	long offset = quicktime_position(file);
	int result = 0;
	int i;
	quicktime_trak_t *trak = file->vtracks[track].track;
	int height = trak->tkhd.track_height;
	int width = trak->tkhd.track_width;
	long bytes = height * width * 3;

	if(quicktime_raw_rows_consecutive(row_pointers, width, height))
	{
		result = quicktime_write_data(file, row_pointers[0], bytes);
		if(result) result = 0; else result = 1;
	}
	else
	for(i = 0; i < height && !result; i++)
	{
		result = quicktime_write_data(file, row_pointers[i], width * 3);
		if(result) result = 0; else result = 1;
	}

	quicktime_update_tables(file->vtracks[track].track,
						offset,
						file->vtracks[track].current_chunk,
						file->vtracks[track].current_position,
						1,
						bytes);

	file->vtracks[track].current_chunk++;
	return result;
}

int quicktime_raw_rows_consecutive(unsigned char **row_pointers, int w, int h)
{
	int i, result;
// see if row_pointers are consecutive
	for(i = 1, result = 1; i < h; i++)
	{
		if(row_pointers[i] - row_pointers[i - 1] != w * 3) result = 0;
	}
	return result;
}
