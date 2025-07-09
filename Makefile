CFLAGS = -Wall -Wextra -Werror -std=c99

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
