src = $(wildcard src/*.c)
obj = $(src:.c=.o)

LDFLAGS =  -lsoundio -lfftw3f -lm

all: bpm

debug: CFLAGS += -g -Wall
debug: bpm

release: CFLAGS += -O3
release: bpm

bpm: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) bpm

