CC = mpicc
CFLAGS = -Wall -O3 -fopenmp -march=native -g -Iinclude
LDFLAGS = -lm

TARGET = main.x

SRC_DIR = src
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(TARGET)

