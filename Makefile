CXX = g++
CXXFLAGS = -std=c++17
LIBS = -lstdc++

lb: $(wildcard *.cpp)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f lb
