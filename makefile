# Compiler
CC = gcc

# Compilation flags
CFLAGS = -lgd

# Target executable
TARGET = old-photo-parallel-B

# Source files
SRCS = old-photo-parallel-B.c func.c

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)

