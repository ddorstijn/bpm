#!/usr/bin/python 

import math
import wave
import struct

sample_rate = 44100.0

def generate_beeps(n=5):
    """ Generate n beeps at a bpm beats per minute at freq frequency in Hz """
    audio = []
    for x in range(n * int(sample_rate)):
        freq = x / sample_rate * 440
        bpm = sample_rate / 1.2
        vol = 0.5
        val = vol * min(math.asin(math.sin(freq)), 0) * math.sin(bpm)
        audio.append(val)
    
    return audio


def save_wav(file_name, audio):
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


audio = generate_beeps()
save_wav("data/sine_beat.wav", audio)
