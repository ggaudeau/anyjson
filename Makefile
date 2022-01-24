
TARGET = test_anyjson

SRCS = $(shell find . -name '*.cpp')
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS := -Wall -Wextra -pedantic -std=c++17

all: release

release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

debug: CXXFLAGS += -g -DWITH_CHRONO
debug: $(TARGET)

$(TARGET): $(OBJS)
	g++ -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *~

fclean: clean
	rm -f $(TARGET)