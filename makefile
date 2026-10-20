CXX = g++
CXXFLAGS = -Wall
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

build/main: $(OBJ)
	mkdir -p build
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) build/main
