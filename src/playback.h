#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <soundio/soundio.h>

struct metadata 
{
	int sample_rate;
	int samples_played;
	int n_samples;
    int n_channels;
	float** data;
};

struct metadata* init_audio(const char* audio_file);
void free_audio(struct metadata* md);

int play(struct metadata* metadata);
void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
void underflow_callback(struct SoundIoOutStream *outstream);

#endif
