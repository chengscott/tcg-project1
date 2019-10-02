all:
	g++ -std=c++11 -O3 -o threes threes.cpp

format:
	clang-format -i *.cpp *.h

clean:
	rm -rf threes agent board episode *.dSYM