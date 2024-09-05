CXX = g++
CXXFLAGS = -std=c++11 -Iinclude

SRC = src/http_downloader.cpp src/multi_threaded_downloader.cpp src/md5.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = downloader

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) temp_*.tmp