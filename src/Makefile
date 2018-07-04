.PHONY:all

TARGETS=raytracer

all: $(TARGETS)

clean:
	@find . -name "*.o" -a -not -name "*GLM*" -delete
	@find . -name "*.d" -a -not -name "*GLM*" -a -not -name "*lodepng.d" -delete
	@rm -f $(TARGETS)

CC=g++
CFLAGS=-Wall -std=c++11 -g -I./glm-0.9.8.4/glm/ -fopenmp -I./lodepng-master/ -O3

sources=./Scene/Camera/Camera.cpp ./Scene/Scene.cpp ./Scene/Object/Object.cpp ./Ray/Ray.cpp ./lodepng-master/lodepng.cpp

# compile
%.o: %.cpp
	$(CC) -c $(CFLAGS) $*.cpp -o $*.o

%.d: %.cpp        
	@echo “make $@“
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

raytracer: main.o Scene/Scene.o Scene/Object/Object.o Scene/Camera/Camera.o
	$(CC) $(CFLAGS) $^ -o $@

include $(sources:.cpp=.d)

