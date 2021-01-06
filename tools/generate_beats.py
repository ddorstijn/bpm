#!/usr/bin/python 

import math
import wave
import struct

audio = []
sample_rate = 44100.0

def append_silence(duration=950):
    """ Add zeros audio array for duration amount of time in ms """
    num_samples = duration * (sample_rate / 1000.0)

    for x in range(int(num_samples)): 
        audio.append(0.0)


def append_sinewave(freq=440.0, duration=50, volume=1.0):
    """ Add sinewave to array at (freq) Hz for (duration) amount of time in ms at (volume) """ 
    global audio 
    num_samples = duration * (sample_rate / 1000.0)

    for x in range(int(num_samples)):
        audio.append(volume * math.sin(2 * math.pi * freq * ( x / sample_rate )))


def generate_beeps(freq=444.0, bpm=60, n=5):
    """ Generate n beeps at a bpm beats per minute at freq frequency in Hz """
    for i in range(n):
        append_sinewave(freq)
        append_silence()


def save_wav(file_name):
    """ Write audio array to wav file in format """
    wav_file=wave.open(file_name,"w")

    # Number of channels
    nchannels = 1
    # Number of bytes per sample (short is 2 bytes
    sampwidth = 2
    # 44100 is the industry standard sample rate - CD quality
    nframes = len(audio)
    # No compression
    comptype = "NONE"
    compname = "not compressed"
    # Apply params
    wav_file.setparams((nchannels, sampwidth, sample_rate, nframes, comptype, compname))

    for sample in audio:
        wav_file.writeframes(struct.pack('h', int(sample * 32767.0)))

    wav_file.close()


generate_beeps()
save_wav("data/sine_beat.wav")
