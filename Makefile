OBJS = src/main.o src/player.o src/platforms.o src/rect.o
LIBS=$(shell pkg-config gl sdl2 --libs)
CFLAGS=-Wall -Werror -std=c11 $(shell pkg-config gl sdl2 --cflags)

nothing: $(OBJS)
	cc $(CFLAGS) $(OBJS) -o nothing $(LIBS)

%.o: %.c
	cc $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf nothing $(OBJS)
