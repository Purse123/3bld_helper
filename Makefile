CXX = clang++
CXXFLAGS = -Wall
CXXLIBS = $(shell pkg-config --cflags --libs raylib) -lm

main: main.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) $(CXXLIBS)

clean:
	rm main *~
