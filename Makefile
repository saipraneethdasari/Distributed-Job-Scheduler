CXX = g++
CXXFLAGS = -std=c++17 -pthread

all: scheduler worker

scheduler:
	mkdir -p build
	$(CXX) $(CXXFLAGS) scheduler/scheduler.cpp scheduler/server.cpp -o build/scheduler

worker:
	mkdir -p build
	$(CXX) $(CXXFLAGS) worker/worker.cpp -o build/worker

clean:
	rm -rf build