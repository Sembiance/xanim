#ifndef CODECS_H
#define CODECS_H

// =================================== codecs
// All new codecs store their variables in a quicktime_...._codec structure.
// The structure is statically allocated here even if the codec isn't used.

#include "ima4.h"
#include "jpeg.h"
#include "raw.h"
#include "ulaw.h"
#include "yuv4.h"
#include "yuv2.h"
#include "wmx1.h"

typedef struct
{
	quicktime_jpeg_codec_t jpeg_codec;
	quicktime_yuv4_codec_t yuv4_codec;
	quicktime_yuv2_codec_t yuv2_codec;
	quicktime_raw_codec_t raw_codec;
	quicktime_ima4_codec_t ima4_codec;
	quicktime_ulaw_codec_t ulaw_codec;
	quicktime_wmx1_codec_t wmx1_codec;
} quicktime_codecs_t;


#endif
