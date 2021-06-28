
#include <stdio.h>

#include "xa_export.h"

void ppm_encode_video(const char *output_path, int w, int h)
{
	char path[BUFSIZ];
	FILE *output;

	sprintf(path, output_path, export_data.frame, "ppm");
	output = fopen(path, "wb");
	if ( output ) {
		fprintf(output, "P6\n");
		fprintf(output, "%d %d\n", w, h);
		fprintf(output, "255\n");
		if ( !fwrite(export_data.raw_data, (w*3*h), 1, output) ) {
			fprintf(stderr, "Warning: Truncated file: %s\n", path);
		}
		fclose(output);
	}
}
