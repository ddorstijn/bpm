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


struct fft {
	int n_samples;
	fftwf_complex* data;
};

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

static struct fft
generate_fft(float* wav, int n_samples, int n_channels)
{
	struct fft fft = {0};
	fft.n_samples = ((n_samples / 2) + 1);
	fft.data = fftwf_malloc(fft.n_samples * sizeof *fft.data);

	fftwf_plan p = fftwf_plan_dft_r2c_1d(n_samples, wav, fft.data, FFTW_ESTIMATE);
    fftwf_execute(p);
    fftwf_destroy_plan(p);

	return fft;
}

static void
output_fft(struct fft fft, int sample_rate, long chunk_size)
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
	for (int i = 0; i < fft.n_samples; i++) {
		float x = (float)i * (sample_rate / (float)fft.n_samples);
		float y = sqrt(fft.data[i][REAL] * fft.data[i][REAL] + fft.data[i][IMAG] * fft.data[i][IMAG]);
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

	struct fft fft = generate_fft(md->data[0], md->n_samples, md->n_channels);
	output_fft(fft, md->sample_rate, chunk_size);

	/* Clean up */
    fftwf_free(fft.data);	
	free_audio(md);
	
    return 0;
}
