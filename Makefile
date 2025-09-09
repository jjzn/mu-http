CFLAGS = -Wall -Wextra -Werror -std=c99 -Wpedantic -D_POSIX_C_SOURCE

TARGET = mu-http

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

.PHONY = all clean remake

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p build
	$(CC) $(LDFLAGS) $^ -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -r build/* mu-http

remake: clean all

debug: CFLAGS += -g3 -DDEBUG
debug: remake
