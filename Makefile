
test : test.o datrie.h common.h
	g++ -o $@ $^
	rm *.o
