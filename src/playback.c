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
int 
play(struct metadata* md) 
{
    struct SoundIo *soundio = soundio_create();
    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    int err = soundio_connect(soundio);
    if (err) {
        fprintf(stderr, "Unable to connect to backend: %s\n", soundio_strerror(err));
        return 1;
    }

    soundio_flush_events(soundio);

    int device_id = soundio_default_output_device_index(soundio);
    if (device_id < 0) {
        fprintf(stderr, "Output device not found\n");
        return 1;
    }

	struct SoundIoDevice *device = soundio_get_output_device(soundio, device_id); 
	if (!device) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    fprintf(stderr, "Output device: %s\n", device->name);

    if (device->probe_error) {
        fprintf(stderr, "Cannot probe device: %s\n", soundio_strerror(device->probe_error));
        return 1;
    }

    struct SoundIoOutStream *outstream = soundio_outstream_create(device);
    outstream->write_callback = write_callback;
    outstream->underflow_callback = underflow_callback;
	outstream->sample_rate = md->sample_rate;
	outstream->userdata = &md;

    if (soundio_device_supports_format(device, SoundIoFormatFloat32NE)) {
        outstream->format = SoundIoFormatFloat32NE;
    } else {
        fprintf(stderr, "No suitable device format available.\n");
        return 1;
    }

    if ((err = soundio_outstream_open(outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return 1;
    }

    fprintf(stderr, "Software latency: %f\n", outstream->software_latency);

    if (outstream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
        return 1;
    }

    for (;;) {
		sleep(1);

		float percent = (float)md->samples_played / (float)md->n_samples * 100.0;
		printf("%d / %d (%d%%)\n", md->samples_played, md->n_samples, (int)percent);
		fflush(stdout);
		if (md->samples_played >= md->n_samples) {
			break;
		}
    }

    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);

	return 0;
}

void 
write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) 
{
	struct metadata* md = outstream->userdata;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    int err;

    for (;;) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
            exit(1);
        }

        if (!frame_count) {
			break;
		}

        for (int frame = 0; frame < frame_count; frame++) {
            for (int channel = 0; channel < md->n_channels; channel += 1) {
				float* buf = (float*)areas[channel].ptr;
				*buf = md->data[channel][md->samples_played + frame];

				areas[channel].ptr += areas[channel].step;
            }

			md->samples_played++;
        }

        if ((err = soundio_outstream_end_write(outstream))) {
            if (err == SoundIoErrorUnderflow)
                return;
            fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
        if (frames_left <= 0)
            break;
    }
}

void 
underflow_callback(struct SoundIoOutStream *outstream) 
{
    static int count = 0;
    fprintf(stderr, "underflow %d\n", count++);
}

