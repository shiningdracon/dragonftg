GCC=g++
CFLAGS=-Wall -Werror -m64 -std=c++0x -g -I/usr/include/libxml2 #-DDEBUG
#CFLAGS=-m64 -g

EXTRA_SYSLIBS = -lSDL -lSDL_image -lxml2

SOURCE = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

TARGET = run

$(TARGET): $(OBJS)
	$(GCC) $(CFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_SYSLIBS)

$(OBJS): %.o: %.cpp
	$(GCC) -c $(CFLAGS) $< -o $@

all: $(TARGET)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

