CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

FLEX = flex
BISON = bison

TARGET = lisp
SRC_DIR = src
BUILD_DIR = build

SRCS = $(BUILD_DIR)/lexer.cpp $(BUILD_DIR)/parser.cpp \
       $(SRC_DIR)/eval.cpp $(SRC_DIR)/main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/parser.cpp $(BUILD_DIR)/parser.hpp: $(SRC_DIR)/parser.y $(SRC_DIR)/ast.hpp
	$(BISON) -d -o $(BUILD_DIR)/parser.cpp $<

$(BUILD_DIR)/lexer.cpp: $(SRC_DIR)/lexer.l $(BUILD_DIR)/parser.hpp
	$(FLEX) -o $@ $<

$(BUILD_DIR)/lexer.o: $(BUILD_DIR)/lexer.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c -o $@ $<

$(BUILD_DIR)/parser.o: $(BUILD_DIR)/parser.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c -o $@ $<

$(SRC_DIR)/eval.o: $(SRC_DIR)/eval.cpp $(SRC_DIR)/eval.hpp $(SRC_DIR)/ast.hpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c -o $@ $<

$(SRC_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/eval.hpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(SRC_DIR)/*.o