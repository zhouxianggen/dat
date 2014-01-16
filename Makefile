
VPATH = ../com
CXXFLAGS = -I../com

test : test.o utils.o dat.o
	g++ -pg -o $@ $^
	rm *.o

