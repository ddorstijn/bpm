#!/usr/bin/python

from sys import argv
from numpy import genfromtxt
from matplotlib import pyplot as plt

if len(argv) != 2:
    print("Please provide your chunk_size size")
    exit()

chunk_size = argv[1]
input_wav = 'output/{}/wav.csv'.format(chunk_size)
input_fft = "output/{}/fft.csv".format(chunk_size)
output_png = 'output/{}/fig.png'.format(chunk_size)

# Plot waveform
data = genfromtxt(input_wav, delimiter=',')
x, y = data.T

plt.figure(1)
plt.subplot(211)
plt.plot(x, y, "b-", markersize=1)
plt.yscale("log")
plt.xlabel("Time (s)")
plt.ylabel("Amplitude (v)")
plt.xlim(0, .05)

# Plot fft
data = genfromtxt(input_fft, delimiter=',')
x, y = data.T

plt.subplot(212)
plt.plot(x, y, "ro", markersize=1)
plt.yscale("log")
plt.xlabel("Freq (Hz)")
plt.ylabel("Abs FFT audio (v)")
plt.xlim(0, 500)

#plt.savefig(output_png)
plt.show()
