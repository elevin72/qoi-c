

default:
	gcc -g -o qoi src/*.c


tests:
	gcc test/*.c -lcriterion && ./a.out
