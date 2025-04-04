CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -Iinclude
LDFLAGS = -lpcap -lpthread

SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = ipk25-chat

all: $(TARGET)

# Link object files to create the final executable
# Clean up object files after linking
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	rm -f $(OBJS)

# Compile source files into object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
