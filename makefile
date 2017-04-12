CXX = cc

build:
	$(CXX) -c mini_filesystem.h mini_filesystem.c
	ar -cvq lib_mini_filesystem.a mini_filesystem.o
	$(CXX) -o test test.c lib_mini_filesystem.a

clean:
	rm -f test
	rm -f mini_filesystem.h.gch
	rm -f lib_mini_filesystem.a
	rm -f mini_filesystem.o
	rm -f filename
