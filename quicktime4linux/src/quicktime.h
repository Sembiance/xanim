#ifndef QUICKTIME_H
#define QUICKTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "funcprotos.h"
#include "sizes.h"

// This is the reference for all your library entry points.
// The compression formats are the formats supported natively by Quicktime 4 Linux.

// ===================== compression formats =================== //

// All codecs have a compression format here consisting of the 4 byte 
// compressor tag.

// These tags are used by str routines as well as binary.
// RGB uncompressed
#define QUICKTIME_RAW  "raw "

// Jpeg Photo
#define QUICKTIME_JPEG "jpeg"

// YUV 4:2:2
#define QUICKTIME_YUV2 "yuv2"

// YUV 4:2:0
#define QUICKTIME_YUV4 "yuv4"

// EXPERIMENT USE ONLY.  DO NOT USE.
#define QUICKTIME_WMX1 "WMX1"

// Audio formats

// Unsigned 8 bit
#ifndef QUICKTIME_RAW
#define QUICKTIME_RAW "raw "
#endif

// IMA4
#define QUICKTIME_IMA4 "ima4"

// Twos compliment 8, 16, 24
#define QUICKTIME_TWOS "twos"

// ulaw
#define QUICKTIME_ULAW "ulaw"

// =========================== public interface ========================= //

// return 1 if the file is a quicktime file
int quicktime_check_sig(char *path);

// make the quicktime file streamable
int quicktime_make_streamable(char *path);

// Set various options in the file.
int quicktime_set_copyright(quicktime_t *file, char *string);
int quicktime_set_name(quicktime_t *file, char *string);
int quicktime_set_info(quicktime_t *file, char *string);
char* quicktime_get_copyright(quicktime_t *file);
char* quicktime_get_name(quicktime_t *file);
char* quicktime_get_info(quicktime_t *file);

// call this first to open the file and create all the objects
quicktime_t* quicktime_open(char *filename, int rd, int wr);

// Read all the information about the file.
// Requires a MOOV atom be present in the file.
// If no MOOV atom exists return 1 else return 0.
int quicktime_read_info(quicktime_t *file);

// set up tracks in a new file after opening and before writing
// returns the number of quicktime tracks allocated
// audio is stored two channels per quicktime track
int quicktime_set_audio(quicktime_t *file, int channels, long sample_rate, int bits, char *compressor);
// Samplerate can be set after file is created
int quicktime_set_framerate(quicktime_t *file, float framerate);

// video is stored one layer per quicktime track
int quicktime_set_video(quicktime_t *file, int tracks, int frame_w, int frame_h, float frame_rate, char *compressor);

// routines for setting various video parameters
// should be called after set_video
int quicktime_set_jpeg(quicktime_t *file, int quality, int use_float);

// close the file and delete all the objects
int quicktime_close(quicktime_t *file);

// get length information
// channel numbers start on 1 for audio and video
long quicktime_audio_length(quicktime_t *file, int track);
long quicktime_video_length(quicktime_t *file, int track);

// get position information
long quicktime_audio_position(quicktime_t *file, int track);
long quicktime_video_position(quicktime_t *file, int track);

// get file information
int quicktime_video_tracks(quicktime_t *file);
int quicktime_audio_tracks(quicktime_t *file);

int quicktime_has_audio(quicktime_t *file);
long quicktime_sample_rate(quicktime_t *file, int track);
int quicktime_audio_bits(quicktime_t *file, int track);
int quicktime_track_channels(quicktime_t *file, int track);
char* quicktime_audio_compressor(quicktime_t *file, int track);

int quicktime_has_video(quicktime_t *file);
int quicktime_video_width(quicktime_t *file, int track);
int quicktime_video_height(quicktime_t *file, int track);
float quicktime_frame_rate(quicktime_t *file, int track);
char* quicktime_video_compressor(quicktime_t *file, int track);

// number of bytes of raw data in this frame
long quicktime_frame_size(quicktime_t *file, long frame, int track);

// get the quicktime track and channel that the audio channel belongs to
// channels and tracks start on 0
int quicktime_channel_location(quicktime_t *file, int *quicktime_track, int *quicktime_channel, int channel);

// file positioning
int quicktime_seek_end(quicktime_t *file);
int quicktime_seek_start(quicktime_t *file);

// set position of file descriptor relative to a track
int quicktime_set_audio_position(quicktime_t *file, long sample, int track);
int quicktime_set_video_position(quicktime_t *file, long frame, int track);

// ========================== Access to raw data follows.
// write data for one quicktime track
// the user must handle conversion to the channels in this track
int quicktime_write_audio(quicktime_t *file, char *audio_buffer, long samples, int track);
int quicktime_write_frame(quicktime_t *file, unsigned char *video_buffer, long bytes, int track);

// for writing a frame using a library that needs a file descriptor
int quicktime_write_frame_init(quicktime_t *file, int track); // call before fwrite
FILE* quicktime_get_fd(quicktime_t *file);     // return a file descriptor
int quicktime_write_frame_end(quicktime_t *file, int track); // call after fwrite

// For reading and writing audio to a file descriptor.
int quicktime_write_frame_init(quicktime_t *file, int track); // call before fwrite
int quicktime_write_audio_end(quicktime_t *file, int track, long samples); // call after fwrite

// Read an entire chunk.
// read the number of bytes starting at the byte_start in the specified chunk
// You must provide enough space to store the chunk.
int quicktime_read_chunk(quicktime_t *file, char *output, int track, long chunk, long byte_start, long byte_len);

// read raw data
long quicktime_read_audio(quicktime_t *file, char *audio_buffer, long samples, int track);
long quicktime_read_frame(quicktime_t *file, unsigned char *video_buffer, int track);

// for reading frame using a library that needs a file descriptor
// Frame caching doesn't work here.
int quicktime_read_frame_init(quicktime_t *file, int track);
int quicktime_read_frame_end(quicktime_t *file, int track);

// Attempt to cache all frames for this track in RAM.
// For video tracks only.
// Returns 1 on failure.
int quicktime_cache_frames(quicktime_t *file, int track);

// ===================== Access to built in codecs follows.

// If the codec for this track is supported in the library return 1.
int quicktime_supported_video(quicktime_t *file, int track);
int quicktime_supported_audio(quicktime_t *file, int track);


// Decode or encode the frame into a frame buffer.
// All the frame buffers passed to these functions are unsigned char
// rows with 3 bytes per pixel.  The byte order per 3 byte pixel is
// RGB.
int quicktime_decode_video(quicktime_t *file, unsigned char **row_pointers, int track);
int quicktime_encode_video(quicktime_t *file, unsigned char **row_pointers, int track);

// Decode or encode audio for a single channel into the buffer.
// Pass a buffer for the _i or the _f argument if you want int16 or float data.
// Notice that encoding requires an array of pointers to each channel.
int quicktime_decode_audio(quicktime_t *file, QUICKTIME_INT16 *output_i, float *output_f, long samples, int channel);
int quicktime_encode_audio(quicktime_t *file, QUICKTIME_INT16 **input_i, float **input_f, long samples);

// Dump the file structures for the currently opened file.
int quicktime_dump(quicktime_t *file);

#ifdef __cplusplus
}
#endif

#endif
