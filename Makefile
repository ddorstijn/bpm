output_dir = bin
program_name = bpm
src = $(wildcard src/*.c)
obj = $(src:.c=.o)

LDFLAGS =  -lsoundio -lfftw3f -lm

all: release

debug: CFLAGS += -g -Wall
debug: $(program_name)

release: CFLAGS += -O3
release: $(program_name)

$(program_name): $(obj)
	$(CC) -o $(output_dir)/$@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(output_dir)/$(program_name)

