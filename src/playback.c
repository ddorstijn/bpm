#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "playback.h"

struct metadata* 
init_audio(const char* audio_file)
{
	struct metadata* md = malloc(sizeof *md);
	drwav wav;

	if (!drwav_init_file(&wav, audio_file, NULL)) { 
		return NULL;
    }

    float* data = malloc(wav.totalPCMFrameCount * wav.channels * sizeof *data);
    drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, data);

	unsigned int nearest_power = wav.totalPCMFrameCount;
	nearest_power--;
	nearest_power |= nearest_power >> 1;
	nearest_power |= nearest_power >> 2;
	nearest_power |= nearest_power >> 4;
	nearest_power |= nearest_power >> 8;
	nearest_power |= nearest_power >> 16;
	nearest_power++;

	md->n_channels = wav.channels;
	md->n_samples = nearest_power;
	md->sample_rate = wav.sampleRate;
	md->samples_played = 0;
	md->data = malloc(wav.channels * sizeof *md->data);
	for (int c = 0; c < wav.channels; c++) {
		md->data[c] = calloc(md->n_samples, sizeof *md->data[c]);
		for (int s = 0; s < wav.totalPCMFrameCount; s++) { 
			md->data[c][s] = data[s * wav.channels];
		}
	}

	free(data);
    drwav_uninit(&wav);		

	return md;
}

void
free_audio(struct metadata* md)
{
	for (int c = 0; c < md->n_channels; c++) {
		free(md->data[c]); 
	}

	free(md->data);
	free(md);
}
