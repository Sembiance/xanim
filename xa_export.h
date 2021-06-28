/*-------------------------
Export code by
Adam Williams
broadcast@earthling.net
---------------------------*/





#ifndef XA_EXPORT_H
#define XA_EXPORT_H

#include "quicktime.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xa_export_ppm.h"
#include "xa_export_jpg.h"

typedef struct {
	int export;           // 1 - export mode  0 - normal
// Video export
#ifdef XA_EXPORT_QT
	quicktime_t *file;    // file for quicktime export
#endif
	char vid_compressor[4];   // compression method
	char aud_compressor[4];
	char path[1024];
	int jpeg_quality;
	unsigned char *raw_data;
	unsigned char **row_pointers;
	int depth, bits_per_pixel; // Depth and bits_per_pixel are different
	int frame;            // the current exported frame number
// Audio export
	int audio_fd;
} xa_exportable;

extern xa_exportable export_data;

int xa_export_init();
int xa_export_end();
int xa_export_set_path(char *path);
int xa_export_set_vid_compressor(char *compressor);
int xa_export_set_aud_compressor(char *compressor);
int xa_export_frame(unsigned char *data, int w, int h);
int xa_export_ximage(XImage * image);
int xa_export_16to24_row(unsigned short *input, unsigned char *output, int w);
int xa_export_32to24_row(unsigned char *input, unsigned char *output, int w);
int xa_export_match_vid_compressor(char *test);
char* xa_export_int_to_codec(int t);

#endif
