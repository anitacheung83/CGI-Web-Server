all: simple die large server

simple: simple.o cgi.o
	gcc -Wall -g -o simple simple.o cgi.o

die: die.o
	gcc -Wall -g -o die die.o

large: large.o cgi.o
	gcc -Wall -g -o large large.o cgi.o

server: server.o
	gcc -Wall -g -o server server.o

%.o: %.c
	gcc -Wall -g -c $<

#Tests
tests: test1 test2 test3 test4
	killall(server)
	rm* .o
	rm output*.txt

test1: die server
	./server < testfiles/input1 > output1
	diff output1 testfiles/die.expected

test2: large server
	./server < testfiles/input2 >output2
	diff output2 testfiles/large.expected

test3: simple server
	./server < testfiles/input3> output3
	diff output3 testfiles/simple.expected

test4: server
	./server < testfiles/input4 > output4
	diff output4 testfiles/404.expected
