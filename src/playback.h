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

#endif
