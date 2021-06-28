#include "quicktime.h"



int quicktime_stsc_init(quicktime_stsc_t *stsc)
{
	stsc->version = 0;
	stsc->flags = 0;
	stsc->total_entries = 0;
	stsc->entries_allocated = 0;
}

int quicktime_stsc_init_table(quicktime_t *file, quicktime_stsc_t *stsc)
{
	if(!stsc->total_entries)
	{
		stsc->total_entries = 1;
		stsc->entries_allocated = 1;
		stsc->table = (quicktime_stsc_table_t*)malloc(sizeof(quicktime_stsc_table_t) * stsc->entries_allocated);
	}
}

int quicktime_stsc_init_video(quicktime_t *file, quicktime_stsc_t *stsc)
{
	quicktime_stsc_table_t *table;
	quicktime_stsc_init_table(file, stsc);
	table = &(stsc->table[0]);
	table->chunk = 1;
	table->samples = 1;
	table->id = 1;
}

int quicktime_stsc_init_audio(quicktime_t *file, quicktime_stsc_t *stsc, int sample_rate)
{
	quicktime_stsc_table_t *table;
	quicktime_stsc_init_table(file, stsc);
	table = &(stsc->table[0]);
	table->chunk = 1;
	table->samples = 0;         // set this after completion or after every audio chunk is written
	table->id = 1;
}

int quicktime_stsc_delete(quicktime_stsc_t *stsc)
{
	if(stsc->total_entries) free(stsc->table);
	stsc->total_entries = 0;
}

int quicktime_stsc_dump(quicktime_stsc_t *stsc)
{
	int i;
	printf("     sample to chunk\n");
	printf("      version %d\n", stsc->version);
	printf("      flags %d\n", stsc->flags);
	printf("      total_entries %d\n", stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		printf("       chunk %d samples %d id %d\n", 
			stsc->table[i].chunk, stsc->table[i].samples, stsc->table[i].id);
	}
}

int quicktime_read_stsc(quicktime_t *file, quicktime_stsc_t *stsc)
{
	int i;
	stsc->version = quicktime_read_char(file);
	stsc->flags = quicktime_read_int24(file);
	stsc->total_entries = quicktime_read_int32(file);
	
	stsc->entries_allocated = stsc->total_entries;
	stsc->table = (quicktime_stsc_table_t*)malloc(sizeof(quicktime_stsc_table_t) * stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		stsc->table[i].chunk = quicktime_read_int32(file);
		stsc->table[i].samples = quicktime_read_int32(file);
		stsc->table[i].id = quicktime_read_int32(file);
	}
}


int quicktime_write_stsc(quicktime_t *file, quicktime_stsc_t *stsc)
{
	int i;
	quicktime_atom_t atom;
	quicktime_atom_write_header(file, &atom, "stsc");

	quicktime_write_char(file, stsc->version);
	quicktime_write_int24(file, stsc->flags);
	quicktime_write_int32(file, stsc->total_entries);
	for(i = 0; i < stsc->total_entries; i++)
	{
		quicktime_write_int32(file, stsc->table[i].chunk);
		quicktime_write_int32(file, stsc->table[i].samples);
		quicktime_write_int32(file, stsc->table[i].id);
	}

	quicktime_atom_write_footer(file, &atom);
}

int quicktime_update_stsc(quicktime_stsc_t *stsc, long chunk, long samples)
{
	quicktime_stsc_table_t *table = stsc->table;
	quicktime_stsc_table_t *new_table;
	long i, j, k;

	i = stsc->total_entries;
	do
	{
		i--;
		if(i >= 0)
		{
			if(table[i].samples != samples)
			{
				if(table[i].chunk == chunk)
				{
					table[i].samples = samples;
					return 0;
				}
				else
				if(table[i].chunk < chunk)
				{
					if(stsc->total_entries + 1 > stsc->entries_allocated)
					{
						stsc->entries_allocated *= 2;
						new_table = (quicktime_stsc_table_t*)malloc(sizeof(quicktime_stsc_table_t) * stsc->entries_allocated);
						for(j = 0; j < stsc->total_entries; j++)
							new_table[j] = table[j];

						free(stsc->table);
						stsc->table = new_table;
						table = new_table;
					}

					for(j = stsc->total_entries, k = j - 1; k > i; j--, k--)
						table[j] = table[k];

					table[i + 1].samples = samples;
					table[i + 1].chunk = chunk;
					table[i + 1].id = table[0].id;
					stsc->total_entries++;
					i = -1;
				}
			}
		}
	}while(i >= 0 && table[i].chunk > chunk);
}
