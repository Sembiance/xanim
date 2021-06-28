#include <time.h>
#include "quicktime.h"


int quicktime_read_data(quicktime_t *file, char *data, int size)
{
	return fread(data, size, 1, file->stream);
}

int quicktime_write_data(quicktime_t *file, char *data, int size)
{
	return fwrite(data, size, 1, file->stream);
}

int quicktime_read_pascal(quicktime_t *file, char *data)
{
	char len = fgetc(file->stream);
	quicktime_read_data(file, data, len);
	data[len] = 0;
}

int quicktime_write_pascal(quicktime_t *file, char *data)
{
	char len = strlen(data);
	fputc(len, file->stream);
	quicktime_write_data(file, data, len);
}

float quicktime_read_fixed32(quicktime_t *file)
{
	unsigned long a, b, c, d;
	unsigned char data[4];
	
	fread(data, 4, 1, file->stream);
	a = data[0];
	b = data[1];
	c = data[2];
	d = data[3];
	
	a = (a << 8) + b;
	b = (c << 8) + d;

	return (float)a + (float)b / 65536;
}

int quicktime_write_fixed32(quicktime_t *file, float number)
{
	unsigned char data[4];
	int a, b;

	a = number;
	b = (number - a) * 65536;
	data[0] = a >> 8;
	data[1] = a & 0xff;
	data[2] = b >> 8;
	data[3] = b & 0xff;
	
	return fwrite(data, 4, 1, file->stream);
}

int quicktime_write_int32(quicktime_t *file, long value)
{
	unsigned char data[4];

	data[0] = (value & 0xff000000) >> 24;
	data[1] = (value & 0xff0000) >> 16;
	data[2] = (value & 0xff00) >> 8;
	data[3] = value & 0xff;

	return fwrite(data, 4, 1, file->stream);
}

int quicktime_write_char32(quicktime_t *file, char *string)
{
	return fwrite(string, 4, 1, file->stream);
}


float quicktime_read_fixed16(quicktime_t *file)
{
	unsigned char data[2];
	
	fread(data, 2, 1, file->stream);
	return (float)data[0] + (float)data[1] / 256;
}

int quicktime_write_fixed16(quicktime_t *file, float number)
{
	unsigned char data[2];
	int a, b;

	a = number;
	b = (number - a) * 256;
	data[0] = a;
	data[1] = b;
	
	return fwrite(data, 2, 1, file->stream);
}

long quicktime_read_int32(quicktime_t *file)
{
	unsigned long result;
	unsigned long a, b, c, d;
	char data[4];
	
	fread(data, 4, 1, file->stream);
	a = (unsigned char)data[0];
	b = (unsigned char)data[1];
	c = (unsigned char)data[2];
	d = (unsigned char)data[3];

	result = (a<<24) | (b<<16) | (c<<8) | d;
	return (long)result;
}


long quicktime_read_int24(quicktime_t *file)
{
	unsigned long result;
	unsigned long a, b, c;
	char data[4];
	
	fread(data, 3, 1, file->stream);
	a = (unsigned char)data[0];
	b = (unsigned char)data[1];
	c = (unsigned char)data[2];

	result = (a<<16) | (b<<8) | c;
	return (long)result;
}

int quicktime_write_int24(quicktime_t *file, long number)
{
	unsigned char data[3];
	data[0] = (number & 0xff0000) >> 16;
	data[1] = (number & 0xff00) >> 8;
	data[2] = (number & 0xff);
	
	return fwrite(data, 3, 1, file->stream);
}

int quicktime_read_int16(quicktime_t *file)
{
	unsigned long result;
	unsigned long a, b;
	char data[2];
	
	fread(data, 2, 1, file->stream);
	a = (unsigned char)data[0];
	b = (unsigned char)data[1];

	result = (a<<8) | b;
	return (int)result;
}

int quicktime_write_int16(quicktime_t *file, int number)
{
	unsigned char data[2];
	data[0] = (number & 0xff00) >> 8;
	data[1] = (number & 0xff);
	
	return fwrite(data, 2, 1, file->stream);
}

int quicktime_read_char(quicktime_t *file)
{
	return fgetc(file->stream);
}

int quicktime_write_char(quicktime_t *file, char x)
{
	return fputc(x, file->stream);
}

int quicktime_read_char32(quicktime_t *file, char *string)
{
	fread(string, 4, 1, file->stream);
}

long quicktime_position(quicktime_t *file) { return ftell(file->stream); }

int quicktime_set_position(quicktime_t *file, long position) { fseek(file->stream, position, SEEK_SET); }

int quicktime_copy_char32(char *output, char *input)
{
	*output++ = *input++;
	*output++ = *input++;
	*output++ = *input++;
	*output = *input;
}


int quicktime_print_chars(char *desc, char *input, int len)
{
	int i;
	printf("%s", desc);
	for(i = 0; i < len; i++) printf("%c", input[i]);
	printf("\n");
}

unsigned long quicktime_current_time()
{
	time_t t;
	time (&t);
	return (t+(66*31536000)+1468800);
}

int quicktime_match_32(char *input, char *output)
{
	if(input[0] == output[0] &&
		input[1] == output[1] &&
		input[2] == output[2] &&
		input[3] == output[3])
		return 1;
	else 
		return 0;
}
