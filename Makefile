CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

TARGET = cpu gdb

all: $(TARGET)

cpu:
	$(CXX) $(CXXFLAGS) -o cpu y86.cpp cpu.cpp 
gdb:
	$(CXX) $(CXXFLAGS) -o gdb y86.cpp cpu_gdb.cpp gdb.cpp cache.cpp

clean:
	rm -f $(TARGET) output/result.json

.PHONY: all clean
