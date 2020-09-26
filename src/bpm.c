#include <errno.h>
#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "playback.h"

#define REAL 0
#define IMAG 1

static void
normalize(float* data, int n_samples, long chunk_size)
{
	/* Normalize data */
	for (int i = 0; i < n_samples / chunk_size; i++) {
		float max = 0;
		for (int j = 0; j < chunk_size; j++) {
			float val = fabs(data[i + j]);
			if (val > max) {
				max = fabs(data[i + j]);
			}
		}
		
		if (max != 0) {
			for (int j = 0; i < chunk_size; i++) {
				data[i + j] = data[i + j] / max;
			}	
		}
	}
}

static void
output_wav(float* data, int n_samples, int sample_rate, long chunk_size)
{
	char output_dir[16];
	sprintf(output_dir, "output/%ld", chunk_size); 
	struct stat st = {0};

	if (stat(output_dir, &st) == -1) {
		mkdir(output_dir, 0700);
	}

	char output[32];
	sprintf(output, "%s/wav.csv", output_dir); 
	FILE* wav = fopen(output, "w");
	for (int i = 0; i < n_samples; i += 2) {
		float x = (float)i/sample_rate; 
		float y = data[i];
		fprintf(wav, "%f,%f \n", x, y);
	}

	fclose(wav);
}

fftwf_complex*
generate_fft(float* wav, int n_samples, int n_channels)
{
	fftwf_complex* data;
	fftwf_plan plan;

	data = fftwf_malloc(n_samples * sizeof *data);
	plan = fftwf_plan_dft_r2c_1d(n_samples, wav, data, FFTW_ESTIMATE);

    fftwf_execute(plan);
    fftwf_destroy_plan(plan);

	return data;
}

static void
output_fft(fftwf_complex* data, int n_samples, int sample_rate, long chunk_size)
{
	char output_dir[16];
	sprintf(output_dir, "output/%ld", chunk_size); 
	struct stat st = {0};

	if (stat(output_dir, &st) == -1) {
		mkdir(output_dir, 0700);
	}

	char output[32];
	sprintf(output, "%s/fft.csv", output_dir); 
	FILE* temp = fopen(output, "w");
	for (int i = 0; i < n_samples; i++) {
		float x = (float)i * (sample_rate / (float)n_samples);
		float y = sqrt(data[i][REAL] * data[i][REAL] + data[i][IMAG] * data[i][IMAG]);
		if (x < 0.5) {
			y = 0;
		}

		fprintf(temp, "%f,%f \n", x, y);
	}

	fclose(temp);
}

int 
main(int argc, char* argv[]) 
{
	if (argc != 3) {
		printf("Usage: bpm path_to_audio chunk_size\n");
		return 1;
	}

	/* Get audio data */
	const char* audio_file = argv[1];
	long chunk_size = strtol(argv[2], NULL, 10);
	if (errno != 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	if (chunk_size > 10E25) {
		fprintf(stderr, "Chunk size cannot be bigger then %ld\n", (long)10E25);
		return 1;
	}

	struct metadata* md = init_audio(audio_file);	
	if (md == NULL) {
		fprintf(stderr, "Error opening audio file\n");
		return 1;
	}

	normalize(md->data[0], md->n_samples, chunk_size);
	output_wav(md->data[0], md->n_samples, md->sample_rate, chunk_size);

	fftwf_complex* fft = generate_fft(md->data[0], md->n_samples, md->n_channels);
	output_fft(fft, md->n_samples, md->sample_rate, chunk_size);

	/* Clean up */
    fftwf_free(fft);	
	free_audio(md);
	
    return 0;
}
