CFLAGS = -Wall -Wextra -Werror -std=c99 -Wpedantic -D_POSIX_C_SOURCE=200809L

TARGET = mu-http

SRCS = $(wildcard src/*.c) src/config.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

.PHONY = all clean remake

all: build $(TARGET)

build:
	mkdir -p build

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

src/config.c: mkconfig.m4 config
	m4 $^ > $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -r build mu-http src/config.c

remake: clean all

debug: CFLAGS += -g3 -DDEBUG
debug: remake
