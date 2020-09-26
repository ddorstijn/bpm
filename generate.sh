#!/bin/bash
make release

tests=(128 512 1024 2048 8192 48000)
for i in "${tests[@]}"
do
	echo $i
	./bpm songs/sine.wav "$i"
	python plot.py "$i"
done
