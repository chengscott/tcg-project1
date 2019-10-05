.PHONY: run judge format clean
all: threes

threes:
	g++ -std=c++11 -O3 -o threes threes.cpp

run: threes
	./threes --save=stat.txt

judge: run
	/tcg/files/pj-1-judge --judge=stat.txt

format:
	clang-format -i *.cpp *.h

clean:
	rm -rf threes action agent board episode statistic *.dSYM