#ifndef CODECBASE_H
#define CODECBASE_H

// Back end for all codecs.
// Eventually to become the plugin front end.

// Structure owned by all plugins
typedef struct
{
	quicktime_t *file;
	int track;
} quicktime_plugin_t;

// Allocate the codec structure somewhere else
void* quicktime_create_codec(quicktime_codec_t *plugin, int size);

// Retrieve the codec structure for the plugin
void* quicktime_get_codec(quicktime_codec_t *plugin, int track);


#endif
