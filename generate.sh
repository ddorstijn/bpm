#!/bin/bash
make release

tests=(128 512 1024 2048 8192 48000)
for i in "${tests[@]}"
do
	echo $i
	./bin/bpm data/sine_beat.wav "$i"
	echo "Finished generating fft for audio fragment"
	python tools/plot.py "$i"
	echo "Finished generating plot from data"
done
