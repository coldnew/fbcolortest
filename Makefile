HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

TARGET = fbcolortest

CFLAGS	= -std=c99

.PHONY: default all clean

all: $(TARGET) $(TARGET_TEST)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -static -o $@

clean:
	$(RM) -rf *.o
	$(RM) -rf $(TARGET)
