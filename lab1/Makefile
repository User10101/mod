all: lab1 tests
lab1: lab1.cpp model.cpp
	g++ -g -std=c++11 lab1.cpp model.cpp -o lab1 -lm
tests: tests.cpp model.cpp
	g++ -g -std=c++11 tests.cpp model.cpp -o tests -lm
clean:
	rm *.o lab1 tests
