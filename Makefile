
test : test.o utils.o dat.o
	g++ -pg -o $@ $^
	rm *.o
