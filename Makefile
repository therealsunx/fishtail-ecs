CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC = main.cpp
OUT = ecstest
BDIR = build

$(OUT): $(SRC)
	mkdir -p $(BDIR)
	$(CXX) $(CXXFLAGS) -o $(BDIR)/$(OUT) $(SRC)

clean:
	rm -f $(BDIR)/$(OUT)
