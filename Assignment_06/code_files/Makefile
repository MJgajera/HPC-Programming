CXX = g++
CFLAGS_SERIAL = -O3
CFLAGS_PARALLEL = -O3 -fopenmp

all: pic_serial pic_parallel

pic_serial: main_serial.cpp init.cpp utils_serial.cpp
	$(CXX) $(CFLAGS_SERIAL) -o pic_serial main_serial.cpp init.cpp utils_serial.cpp

pic_parallel: main_parallel.cpp init.cpp utils_parallel.cpp
	$(CXX) $(CFLAGS_PARALLEL) -o pic_parallel main_parallel.cpp init.cpp utils_parallel.cpp

clean:
	rm -f pic_serial pic_parallel Mesh_serial.out Mesh_parallel.out
